#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uiohook.h>
#include <uv.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif

#include "uiohook_worker.h"

// Thread and mutex variables.
static uv_thread_t hook_thread;
static int hook_thread_status;
static uv_mutex_t hook_running_mutex;
static uv_mutex_t hook_control_mutex;
static uv_cond_t hook_control_cond;
static uv_mutex_t suppress_shortcuts_mutex;
static uv_once_t suppress_shortcuts_once = UV_ONCE_INIT;

static dispatcher_t user_dispatcher = NULL;
static suppress_shortcut_t* suppress_shortcuts = NULL;
static size_t suppress_shortcut_count = 0;

static void init_suppress_shortcuts_mutex(void) {
  uv_mutex_init(&suppress_shortcuts_mutex);
}

static inline uint16_t get_primary_modifier_mask(uint16_t keycode) {
  switch (keycode) {
  case VC_SHIFT_L:
  case VC_SHIFT_R:
    return MASK_SHIFT;
  case VC_CONTROL_L:
  case VC_CONTROL_R:
    return MASK_CTRL;
  case VC_ALT_L:
  case VC_ALT_R:
    return MASK_ALT;
  case VC_META_L:
  case VC_META_R:
    return MASK_META;
  default:
    return 0;
  }
}

static inline bool keycode_matches_shortcut(uint16_t shortcut_keycode, uint16_t event_keycode) {
  switch (shortcut_keycode) {
  case VC_SHIFT_L:
    return event_keycode == VC_SHIFT_L || event_keycode == VC_SHIFT_R;
  case VC_CONTROL_L:
    return event_keycode == VC_CONTROL_L || event_keycode == VC_CONTROL_R;
  case VC_ALT_L:
    return event_keycode == VC_ALT_L || event_keycode == VC_ALT_R;
  case VC_META_L:
    return event_keycode == VC_META_L || event_keycode == VC_META_R;
  default:
    return shortcut_keycode == event_keycode;
  }
}

static inline uint16_t to_generic_modifier_mask(uint16_t mask) {
  uint16_t generic_mask = 0;

  if (mask & (MASK_SHIFT)) {
    generic_mask |= MASK_SHIFT;
  }
  if (mask & (MASK_CTRL)) {
    generic_mask |= MASK_CTRL;
  }
  if (mask & (MASK_ALT)) {
    generic_mask |= MASK_ALT;
  }
  if (mask & (MASK_META)) {
    generic_mask |= MASK_META;
  }

  return generic_mask;
}

static inline void normalize_keyboard_event(uiohook_event* const event) {
  if (event->type != EVENT_KEY_PRESSED && event->type != EVENT_KEY_RELEASED) {
    return;
  }

  event->mask = to_generic_modifier_mask(event->mask);
  event->mask |= get_primary_modifier_mask(event->data.keyboard.keycode);
}

static bool matches_suppress_shortcut(uiohook_event* const event) {
  bool matches = false;

  if (event->type != EVENT_KEY_PRESSED && event->type != EVENT_KEY_RELEASED) {
    return false;
  }

  const uint16_t keycode = event->data.keyboard.keycode;
  uint16_t mask = event->mask;

  uv_once(&suppress_shortcuts_once, init_suppress_shortcuts_mutex);
  uv_mutex_lock(&suppress_shortcuts_mutex);

  for (size_t i = 0; i < suppress_shortcut_count; i++) {
    if (suppress_shortcuts[i].any_keycode) {
      if ((mask & suppress_shortcuts[i].mask) == suppress_shortcuts[i].mask) {
        matches = true;
        break;
      }
      continue;
    }

    if (!keycode_matches_shortcut(suppress_shortcuts[i].keycode, keycode)) {
      continue;
    }

    if (suppress_shortcuts[i].mask == mask) {
      matches = true;
      break;
    }
  }

  uv_mutex_unlock(&suppress_shortcuts_mutex);
  return matches;
}

static bool should_consume_event(uiohook_event* const event) {
  return matches_suppress_shortcut(event);
}

bool logger_proc(unsigned int level, const char* format, ...) {
  bool status = false;

  va_list args;
  switch (level) {
  case LOG_LEVEL_WARN:
  case LOG_LEVEL_ERROR:
    va_start(args, format);
    status = vfprintf(stderr, format, args) >= 0;
    va_end(args);
    break;
  }

  return status;
}

// NOTE: The following callback executes on the same thread that hook_run() is called 
// from.  This is important because hook_run() attaches to the operating systems
// event dispatcher and may delay event delivery to the target application.
// Furthermore, some operating systems may choose to disable your hook if it 
// takes to long to process.  If you need to do any extended processing, please 
// do so by copying the event to your own queued dispatch thread.
void worker_dispatch_proc(uiohook_event* const event) {
  normalize_keyboard_event(event);

  if (should_consume_event(event)) {
    event->reserved = 0x01;
  }

  switch (event->type) {
  case EVENT_HOOK_ENABLED:
    // Lock the running mutex so we know if the hook is enabled.
    uv_mutex_lock(&hook_running_mutex);

    // Signal control cond so hook_enable() can continue.
    uv_mutex_lock(&hook_control_mutex);
    uv_cond_signal(&hook_control_cond);
    uv_mutex_unlock(&hook_control_mutex);
    break;

  case EVENT_HOOK_DISABLED:
    // Lock the control mutex until we exit.
    uv_mutex_lock(&hook_control_mutex);

    // Unlock the running mutex so we know if the hook is disabled.
    uv_mutex_unlock(&hook_running_mutex);
    break;

  case EVENT_KEY_PRESSED:
  case EVENT_KEY_RELEASED:
  // case EVENT_KEY_TYPED:
  case EVENT_MOUSE_CLICKED:
  case EVENT_MOUSE_PRESSED:
  case EVENT_MOUSE_RELEASED:
  case EVENT_MOUSE_MOVED:
  case EVENT_MOUSE_DRAGGED:
  case EVENT_MOUSE_WHEEL: {
    user_dispatcher(event);
    break;
  }

  default:
    break;
  }
}

void hook_thread_proc(void* arg) {
  #ifdef _WIN32
  // Attempt to set the thread priority to time critical.
  HANDLE this_thread = GetCurrentThread();
  if (SetThreadPriority(this_thread, THREAD_PRIORITY_TIME_CRITICAL) == FALSE) {
    logger_proc(LOG_LEVEL_WARN, "%s [%u]: Could not set thread priority %li for thread %#p! (%#lX)\n",
      __FUNCTION__, __LINE__, (long)THREAD_PRIORITY_TIME_CRITICAL,
      this_thread, (unsigned long)GetLastError());
  }
  #else
  // Raise the thread priority
  pthread_t this_thread = pthread_self();
  struct sched_param params = {
    .sched_priority = (sched_get_priority_max(SCHED_RR) / 2)
  };
  if (pthread_setschedparam(this_thread, SCHED_RR, &params) != 0) {
    logger_proc(LOG_LEVEL_WARN, "%s [%u]: Could not set thread priority %i for thread 0x%lX!\n",
      __FUNCTION__, __LINE__, params.sched_priority, (unsigned long)this_thread);
  }
  #endif

  // Set the hook status.
  hook_thread_status = hook_run();

  // Make sure we signal that we have passed any exception throwing code for
  // the waiting hook_enable().
  uv_cond_signal(&hook_control_cond);
  uv_mutex_unlock(&hook_control_mutex);
}

int hook_enable() {
  // Lock the thread control mutex.  This will be unlocked when the
  // thread has finished starting, or when it has fully stopped.
  uv_mutex_lock(&hook_control_mutex);

  // Set the initial status.
  int status = UIOHOOK_FAILURE;

  if (uv_thread_create(&hook_thread, hook_thread_proc, NULL) == 0) {
    // Wait for the thread to indicate that it has passed the 
    // initialization portion by blocking until either a EVENT_HOOK_ENABLED 
    // event is received or the thread terminates.
    uv_cond_wait(&hook_control_cond, &hook_control_mutex);

    if (uv_mutex_trylock(&hook_running_mutex) == 0) {
      // Lock Successful; The hook is not running but the hook_control_cond 
      // was signaled!  This indicates that there was a startup problem!

      // Get the status back from the thread.
      uv_thread_join(&hook_thread);
      status = hook_thread_status;
      uv_mutex_unlock(&hook_running_mutex);
    }
    else {
      // Lock Failure; The hook is currently running and wait was signaled
      // indicating that we have passed all possible start checks.  We can 
      // always assume a successful startup at this point.
      status = UIOHOOK_SUCCESS;
    }

    logger_proc(LOG_LEVEL_DEBUG, "%s [%u]: Thread Result: (%#X).\n",
      __FUNCTION__, __LINE__, status);
  }
  else {
    status = UIOHOOK_ERROR_THREAD_CREATE;
  }

  // Make sure the control mutex is unlocked.
  uv_mutex_unlock(&hook_control_mutex);

  return status;
}


int uiohook_worker_start(dispatcher_t dispatch_proc) {
  // Lock the thread control mutex.  This will be unlocked when the
  // thread has finished starting, or when it has fully stopped.

  // Create event handles for the thread hook.
  uv_mutex_init(&hook_running_mutex);
  uv_mutex_init(&hook_control_mutex);
  uv_cond_init(&hook_control_cond);

  // Set the logger callback for library output.
  hook_set_logger_proc(logger_proc);

  // Set the event callback for uiohook events.
  hook_set_dispatch_proc(worker_dispatch_proc);

  user_dispatcher = dispatch_proc;

  // Start the hook and block.
  // NOTE If EVENT_HOOK_ENABLED was delivered, the status will always succeed.
  int status = hook_enable();
  if (status != UIOHOOK_SUCCESS) {
    // Close event handles for the thread hook.
    uv_mutex_destroy(&hook_running_mutex);
    uv_mutex_destroy(&hook_control_mutex);
    uv_cond_destroy(&hook_control_cond);
  }

  return status;
}

int uiohook_worker_stop() {
  int status = hook_stop();

  if (status == UIOHOOK_SUCCESS) {
    uv_thread_join(&hook_thread);

    // Close event handles for the thread hook.
    uv_mutex_destroy(&hook_running_mutex);
    uv_mutex_destroy(&hook_control_mutex);
    uv_cond_destroy(&hook_control_cond);
  }

  return status;
}

void uiohook_worker_set_suppress_shortcuts(const suppress_shortcut_t* shortcuts, size_t count) {
  suppress_shortcut_t* copied_shortcuts = NULL;
  uv_once(&suppress_shortcuts_once, init_suppress_shortcuts_mutex);

  if (count > 0) {
    copied_shortcuts = malloc(sizeof(suppress_shortcut_t) * count);
    if (copied_shortcuts == NULL) {
      return;
    }

    memcpy(copied_shortcuts, shortcuts, sizeof(suppress_shortcut_t) * count);
  }

  uv_mutex_lock(&suppress_shortcuts_mutex);
  free(suppress_shortcuts);
  suppress_shortcuts = copied_shortcuts;
  suppress_shortcut_count = count;
  uv_mutex_unlock(&suppress_shortcuts_mutex);
}
