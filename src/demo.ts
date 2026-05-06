import { uIOhook, UiohookKey } from "./";

const keycodeMap = new Map(Object.entries(UiohookKey).map((_) => [_[1], _[0]]));

(function main() {
    const suppressId = uIOhook.registerSuppress([
        { metaKey: true },
        { keycode: UiohookKey.F4, altKey: true },
        { keycode: UiohookKey.Escape, ctrlKey: true, shiftKey: true },
    ]);
    uIOhook.toggleSuppress(suppressId, true);
    console.log(
        "Suppress is enabled for Meta shortcuts, Alt+F4, and Ctrl+Shift+Esc. Press Enter to toggle, Escape to exit.",
    );

    let isSuppressing = true;

    uIOhook.on("keydown", (e) => {
        console.log(
            `${prettyModifier("ctrl", e.ctrlKey)}${prettyModifier("shift", e.shiftKey)}${prettyModifier("alt", e.altKey)}${prettyModifier("meta", e.metaKey)}`,
            e.keycode,
            keycodeMap.get(e.keycode as any),
        );

        if (e.keycode === UiohookKey.Escape) {
            uIOhook.unregisterSuppress(suppressId);
            uIOhook.stop();
            process.exit(0);
        }
        if (e.keycode === UiohookKey.Enter) {
            uIOhook.toggleSuppress(suppressId, !isSuppressing);
            isSuppressing = !isSuppressing;
            console.log(`Suppress ${isSuppressing ? "enabled" : "disabled"}.`);
        }
    });

    uIOhook.start();
})();

function prettyModifier(name: string, state: boolean) {
    return state ? `[${name}]` : ` ${name} `;
}
