# uiohook-napi

[![](https://img.shields.io/npm/v/uiohook-napi/latest?color=CC3534&label=uiohook-napi&logo=npm&labelColor=212121)](https://www.npmjs.com/package/uiohook-napi)

N-API C-bindings for [libuiohook](https://github.com/kwhat/libuiohook).


### Usage example

```typescript
import { uIOhook, UiohookKey } from 'uiohook-napi'

uIOhook.on('keydown', (e) => {
  if (e.keycode === UiohookKey.Q) {
    console.log('Hello!')
  }

  if (e.keycode === UiohookKey.Escape) {
    process.exit(0)
  }
})

uIOhook.start()
```

### API

```typescript
interface UiohookNapi {
  on(event: 'input', listener: (e: UiohookKeyboardEvent | UiohookMouseEvent | UiohookWheelEvent) => void): this

  on(event: 'keydown', listener: (e: UiohookKeyboardEvent) => void): this
  on(event: 'keyup', listener: (e: UiohookKeyboardEvent) => void): this

  on(event: 'mousedown', listener: (e: UiohookMouseEvent) => void): this
  on(event: 'mouseup', listener: (e: UiohookMouseEvent) => void): this
  on(event: 'mousemove', listener: (e: UiohookMouseEvent) => void): this
  on(event: 'click', listener: (e: UiohookMouseEvent) => void): this

  on(event: 'wheel', listener: (e: UiohookWheelEvent) => void): this

  registerSuppress(shortcuts: UiohookKeyboardSuppressShortcut[]): number
  unregisterSuppress(registrationId: number)
  toggleSuppress(registrationId: number, enabled: boolean)
  keyTap(key: keycode, modifiers?: keycode[])
  keyToggle(key: keycode, toggle: 'down' | 'up')
}

export interface UiohookKeyboardEvent {
  altKey: boolean
  ctrlKey: boolean
  metaKey: boolean
  shiftKey: boolean
  keycode: number
}

export interface UiohookKeyboardSuppressShortcut {
  keycode: number
  altKey?: boolean
  ctrlKey?: boolean
  metaKey?: boolean
  shiftKey?: boolean
}

export interface UiohookMouseEvent {
  altKey: boolean
  ctrlKey: boolean
  metaKey: boolean
  shiftKey: boolean
  x: number
  y: number
  button: unknown
  clicks: number
}

export interface UiohookWheelEvent {
  altKey: boolean
  ctrlKey: boolean
  metaKey: boolean
  shiftKey: boolean
  x: number
  y: number
  clicks: number
  amount: number
  direction: WheelDirection
  rotation: number
}
```

### Shortcut suppression

Register shortcuts once, then toggle or unregister them later:

```typescript
import { uIOhook, UiohookKey } from 'uiohook-napi'

const suppressId = uIOhook.registerSuppress([
  { keycode: UiohookKey.Meta },
  { keycode: UiohookKey.R, metaKey: true },
  { keycode: UiohookKey.Tab, altKey: true },
  { keycode: UiohookKey.F4, altKey: true }
])

uIOhook.start()

uIOhook.toggleSuppress(suppressId, false)
uIOhook.toggleSuppress(suppressId, true)
uIOhook.unregisterSuppress(suppressId)
```

The shortcut match is exact for `ctrlKey`, `altKey`, `shiftKey`, and `metaKey`. For modifier-only shortcuts like `Meta`, `Ctrl`, `Shift`, or `Alt`, passing the base keycode matches either left or right side. Use `MetaRight`, `AltRight`, `CtrlRight`, or `ShiftRight` if you need the right-side key specifically.
