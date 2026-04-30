import { uIOhook, UiohookKey } from "./";

const keycodeMap = new Map(Object.entries(UiohookKey).map((_) => [_[1], _[0]]));

(function main() {
    const suppressId = uIOhook.registerSuppress([
        { keycode: UiohookKey.Meta },
        { keycode: UiohookKey.R, metaKey: true },
        { keycode: UiohookKey.Tab, metaKey: true },
        { keycode: UiohookKey.Tab, altKey: true },
        { keycode: UiohookKey.F4, altKey: true },
    ]);
    console.log("Suppress is registered for Meta, Meta+R, Meta+Tab, Alt+Tab, and Alt+F4. Press Escape to exit.");

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
    });

    uIOhook.start();
})();

function prettyModifier(name: string, state: boolean) {
    return state ? `[${name}]` : ` ${name} `;
}
