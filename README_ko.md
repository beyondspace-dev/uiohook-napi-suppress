# @fainthit/uiohook-napi-suppress

[한국어](./README_ko.md) | [English](./README.md)

[![](https://img.shields.io/npm/v/%40fainthit%2Fuiohook-napi-suppress?color=CC3534&label=%40fainthit%2Fuiohook-napi-suppress&logo=npm&labelColor=212121)](https://www.npmjs.com/package/@fainthit/uiohook-napi-suppress)

설정 가능한 키보드 단축키 차단 기능을 추가한 [SnosMe/uiohook-napi](https://github.com/SnosMe/uiohook-napi)의 포크입니다.

이 패키지는 원래 `uiohook-napi` API를 유지하면서 아래 차단 헬퍼를 추가합니다.

- `registerSuppress(shortcuts)`
- `unregisterSuppress(registrationId)`
- `toggleSuppress(registrationId, enabled)`

## 설치

```bash
npm install @fainthit/uiohook-napi-suppress
```

## 사용법

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

## 단축키 차단

단축키를 한 번 등록한 뒤, 나중에 켜거나 끄거나 해제할 수 있습니다.

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

`keycode`가 있는 경우에는 `ctrlKey`, `altKey`, `shiftKey`, `metaKey` 조합이 정확히 일치해야 매칭됩니다.  
`Meta`, `Ctrl`, `Shift`, `Alt` 같은 modifier 키 자체를 등록할 때는 기본 키코드를 넘기면 왼쪽/오른쪽 키 모두 매칭됩니다. 오른쪽 키만 따로 지정하고 싶다면 `MetaRight`, `AltRight`, `CtrlRight`, `ShiftRight`를 사용하면 됩니다.

`keycode`를 생략하고 하나 이상의 modifier만 지정하면, 해당 modifier가 눌린 상태에서 들어오는 모든 키를 차단합니다. 예를 들어 `{ metaKey: true }`는 `Meta` 자체, `Meta+R`, `Meta+Tab`, `Meta+Shift+R`를 모두 차단합니다.

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

## 포크 정보

- 원본 저장소: [SnosMe/uiohook-napi](https://github.com/SnosMe/uiohook-napi)
- 포크 저장소: [hinaple/uiohook-napi-suppress](https://github.com/hinaple/uiohook-napi-suppress)
- 이 포크는 `@fainthit/uiohook-napi-suppress` 이름으로 npm 배포하는 용도입니다.
