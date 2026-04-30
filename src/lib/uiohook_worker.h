#ifndef ADDON_SRC_UIOHOOK_WORKER_H_
#define ADDON_SRC_UIOHOOK_WORKER_H_

#include <uiohook.h>
#include <stddef.h>

#define UIOHOOK_ERROR_THREAD_CREATE				0x10

typedef struct {
  bool any_keycode;
  uint16_t keycode;
  uint16_t mask;
} suppress_shortcut_t;

int uiohook_worker_start(dispatcher_t dispatch_proc);

int uiohook_worker_stop();

void uiohook_worker_set_suppress_shortcuts(const suppress_shortcut_t* shortcuts, size_t count);

#endif // !ADDON_SRC_UIOHOOK_WORKER_H_
