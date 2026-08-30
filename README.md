# xkey

`xkey` is a small Linux input-remapping tool that reads a keyboard from an
`evdev` input device and exposes a virtual Xbox 360-style gamepad through
`uinput`.

## P1 mapping

- `KEY_W`, `KEY_A`, `KEY_S`, `KEY_D`: left analog stick
- `UP`, `DOWN`, `LEFT`, `RIGHT`: D-pad
- `LEFTCTRL`, `LEFTALT`, `SPACE`: Xbox A, B, X respectively
- `KEY_1`: Start

The ESC toggle block has been removed intentionally. The keyboard is grabbed
immediately in `main.c` after opening the selected device.

## Build

```sh
make
```

Running generally requires access to `/dev/uinput` and the selected
`/dev/input/eventN` device, such as through appropriate permissions or root.
