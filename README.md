# Xkey-IPAC

`xkey-ipac` is a small Linux input-remapping tool that reads a keyboard from an `evdev` input device and exposes two virtual Xbox 360-style gamepad through `uinput`. It allows a keyboard (or any evdev-compatible device, such as an
I-PAC arcade controller board) to act as Xbox 360 controllers for applications and emulators that support X-Input or generic gamepads.

## Features

- Reads raw keyboard/input events via `evdev` (`/dev/input/eventN`)
- Emulates 2 virtual Xbox 360 controllers via `uinput`
- Automatic input device detection and persistence (no need to reselect the device on every run)
- Designed to run as a background service/daemon at boot (systemd-ready)

---

## Mapping (Player 1)

- `KEY_W`, `KEY_A`, `KEY_S`, `KEY_D`: left analog stick
- `UP`, `DOWN`, `LEFT`, `RIGHT`: D-pad
- `LEFTCTRL`, `LEFTALT`, `SPACE`: Xbox A, B, X respectively
- `KEY_1`: Start

The ESC toggle block has been removed intentionally. The keyboard is grabbed
immediately in `main.c` after opening the selected device.

---

## Device selection & configuration

On first run, `xkey` lists all available input devices under
`/dev/input/event*` and asks you to pick the event number corresponding to
your keyboard or controller board:

```sh
sudo ./xkey
Available input devices:
  /dev/input/event0: Sleep Button
  /dev/input/event1: Power Button
  /dev/input/event4: Ultimarc I-PAC Ultimarc I-PAC
  ...

Enter the event number for your keyboard: 4
```

The selected device's name is automatically saved to /etc/xkey.conf. On
subsequent runs, xkey reads this file and reconnects to the matching device
by name (event numbers can change across reboots, so matching is done by
device name, not by path):
```
sudo ./xkey
Using configured device: Ultimarc I-PAC Ultimarc I-PAC (/dev/input/event4)
```

If the configured device is not found (e.g. unplugged), xkey falls back to manual selection.
To reset the configuration and pick a different device, simply delete or edit /etc/xkey.conf:

```
sudo rm /etc/xkey.conf
Build
make
```

---

## Requirements / Permissions
Running xkey requires access to:

- /dev/uinput (to create the virtual gamepad)
 The selected /dev/input/eventN device (to read input events)
- Write access to /etc/xkey.conf (to persist the selected device)

This typically requires root privileges, or appropriate udev rules granting access to these devices for a non-root user/group.

---

## Running at startup (daemon)
Since the device is now automatically detected from /etc/xkey.conf, xkey
no longer blocks on an interactive prompt, making it suitable to run as a
systemd service at boot. (Service unit setup — coming soon.)
