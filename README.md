# @fainthit/uiohook-napi-suppress

[English](./README.md) | [한국어](./README_ko.md)

[![](https://img.shields.io/npm/v/%40fainthit%2Fuiohook-napi-suppress?color=CC3534&label=%40fainthit%2Fuiohook-napi-suppress&logo=npm&labelColor=212121)](https://www.npmjs.com/package/@fainthit/uiohook-napi-suppress)

Fork of [SnosMe/uiohook-napi](https://github.com/SnosMe/uiohook-napi) with configurable keyboard shortcut suppression support.

This package keeps the original `uiohook-napi` API and adds suppression helpers:

- `registerSuppress(shortcuts)`
- `unregisterSuppress(registrationId)`
- `toggleSuppress(registrationId, enabled)`

## Install

```bash
npm install @fainthit/uiohook-napi-suppress
```

When working from this repository, run `npm run prepare-libuiohook` before local native builds if the `libuiohook` submodule is clean or freshly initialized. That command initializes the submodule when needed and applies `src/libuiohook.patch`.

## Usage

```typescript
import { uIOhook, UiohookKey } from "@fainthit/uiohook-napi-suppress";

uIOhook.on("keydown", (e) => {
  if (e.keycode === UiohookKey.Q) {
    console.log("Hello!");
  }

  if (e.keycode === UiohookKey.Escape) {
    process.exit(0);
  }
});

uIOhook.start();
```

## Shortcut suppression

Register shortcuts once, then toggle or unregister them later:

```typescript
import { uIOhook, UiohookKey } from "@fainthit/uiohook-napi-suppress";

const suppressId = uIOhook.registerSuppress([
  { metaKey: true },
  { keycode: UiohookKey.F4, altKey: true },
]);

uIOhook.start();

uIOhook.toggleSuppress(suppressId, false);
uIOhook.toggleSuppress(suppressId, true);
uIOhook.unregisterSuppress(suppressId);
```

The shortcut match is exact for `ctrlKey`, `altKey`, `shiftKey`, and `metaKey` when `keycode` is present. For modifier-only shortcuts like `Meta`, `Ctrl`, `Shift`, or `Alt`, passing the base keycode matches either left or right side. Use `MetaRight`, `AltRight`, `CtrlRight`, or `ShiftRight` if you need the right-side key specifically.

If you omit `keycode` and provide one or more modifiers, that rule suppresses every key pressed while those modifiers are active. For example, `{ metaKey: true }` suppresses `Meta` itself, `Meta+R`, `Meta+Tab`, and `Meta+Shift+R`.

## API

```typescript
interface UiohookNapi {
  on(
    event: "input",
    listener: (
      e: UiohookKeyboardEvent | UiohookMouseEvent | UiohookWheelEvent,
    ) => void,
  ): this;

  on(event: "keydown", listener: (e: UiohookKeyboardEvent) => void): this;
  on(event: "keyup", listener: (e: UiohookKeyboardEvent) => void): this;
  on(event: "mousedown", listener: (e: UiohookMouseEvent) => void): this;
  on(event: "mouseup", listener: (e: UiohookMouseEvent) => void): this;
  on(event: "mousemove", listener: (e: UiohookMouseEvent) => void): this;
  on(event: "click", listener: (e: UiohookMouseEvent) => void): this;

  on(event: "wheel", listener: (e: UiohookWheelEvent) => void): this;
  registerSuppress(shortcuts: UiohookKeyboardSuppressShortcut[]): number;
  unregisterSuppress(registrationId: number);
  toggleSuppress(registrationId: number, enabled: boolean);
  keyTap(key: keycode, modifiers?: keycode[]);
  keyToggle(key: keycode, toggle: "down" | "up");
}

export interface UiohookKeyboardEvent {
  altKey: boolean;
  ctrlKey: boolean;
  metaKey: boolean;
  shiftKey: boolean;
  keycode: number;
}

export interface UiohookKeyboardSuppressShortcut {
  keycode?: number;
  altKey?: boolean;
  ctrlKey?: boolean;
  metaKey?: boolean;
  shiftKey?: boolean;
}

export interface UiohookMouseEvent {
  altKey: boolean;
  ctrlKey: boolean;
  metaKey: boolean;
  shiftKey: boolean;
  x: number;
  y: number;
  button: unknown;
  clicks: number;
}

export interface UiohookWheelEvent {
  altKey: boolean;
  ctrlKey: boolean;
  metaKey: boolean;
  shiftKey: boolean;
  x: number;
  y: number;
  clicks: number;
  amount: number;
  direction: WheelDirection;
  rotation: number;
}
```

## Fork note

- Upstream repository: [SnosMe/uiohook-napi](https://github.com/SnosMe/uiohook-napi)
- Fork repository: [beyondspace-dev/uiohook-napi-suppress](https://github.com/beyondspace-dev/uiohook-napi-suppress)
- This fork is intended for npm distribution under the scoped package name `@fainthit/uiohook-napi-suppress`
