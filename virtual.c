#include "xkey.h"

static int w_pressed, a_pressed, s_pressed, d_pressed;
static int prev_x, prev_y;
#define FULL 32767
#define DIAGONAL 23170

static void update_stick(void)
{
    int x = (a_pressed && !d_pressed) ? -FULL : (d_pressed && !a_pressed) ? FULL : 0;
    int y = (w_pressed && !s_pressed) ? -FULL : (s_pressed && !w_pressed) ? FULL : 0;
    if (x && y) { x = x < 0 ? -DIAGONAL : DIAGONAL; y = y < 0 ? -DIAGONAL : DIAGONAL; }
    if (x != prev_x || y != prev_y) {
        emit(EV_ABS, ABS_X, x); emit(EV_ABS, ABS_Y, y); syn(); prev_x = x; prev_y = y;
    }
}

static void set_abs(int axis, int min, int max, int fuzz, int flat)
{
    struct uinput_abs_setup s;
    memset(&s, 0, sizeof(s)); s.code = axis; s.absinfo.minimum = min; s.absinfo.maximum = max;
    s.absinfo.fuzz = fuzz; s.absinfo.flat = flat; (void)ioctl(xkey_fd, UI_ABS_SETUP, &s);
}

void setup_virtual_device(void)
{
    struct uinput_setup u;
    memset(&u, 0, sizeof(u));
    snprintf(u.name, UINPUT_MAX_NAME_SIZE, "xkey virtual gamepad");
    u.id.bustype = BUS_USB; u.id.vendor = 0x45e; u.id.product = 0x28e; u.id.version = 0x110;
    (void)ioctl(xkey_fd, UI_SET_EVBIT, EV_KEY); (void)ioctl(xkey_fd, UI_SET_EVBIT, EV_ABS);
    (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_SOUTH); (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_EAST);
    (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_NORTH); (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_WEST);
    (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_TL); (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_TR);
    (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_SELECT); (void)ioctl(xkey_fd, UI_SET_KEYBIT, BTN_START);
    (void)ioctl(xkey_fd, UI_SET_ABSBIT, ABS_X); (void)ioctl(xkey_fd, UI_SET_ABSBIT, ABS_Y);
    (void)ioctl(xkey_fd, UI_SET_ABSBIT, ABS_Z); (void)ioctl(xkey_fd, UI_SET_ABSBIT, ABS_RZ);
    (void)ioctl(xkey_fd, UI_SET_ABSBIT, ABS_HAT0X); (void)ioctl(xkey_fd, UI_SET_ABSBIT, ABS_HAT0Y);
    set_abs(ABS_X, -32768, 32767, 16, 128); set_abs(ABS_Y, -32768, 32767, 16, 128);
    set_abs(ABS_Z, 0, 255, 0, 0); set_abs(ABS_RZ, 0, 255, 0, 0);
    set_abs(ABS_HAT0X, -1, 1, 0, 0); set_abs(ABS_HAT0Y, -1, 1, 0, 0);
    (void)ioctl(xkey_fd, UI_DEV_SETUP, &u); (void)ioctl(xkey_fd, UI_DEV_CREATE); sleep(1);
}

void emit(int type, int code, int val)
{
    struct input_event e;
    memset(&e, 0, sizeof(e)); e.type = type; e.code = code; e.value = val;
    (void)write(xkey_fd, &e, sizeof(e));
}
void syn(void) { emit(EV_SYN, SYN_REPORT, 0); }
void emit_north(int v) { emit(EV_KEY, BTN_NORTH, v); }
void emit_south(int v) { emit(EV_KEY, BTN_SOUTH, v); }
void emit_east(int v) { emit(EV_KEY, BTN_EAST, v); }
void emit_west(int v) { emit(EV_KEY, BTN_WEST, v); }
void emit_tl(int v) { emit(EV_KEY, BTN_TL, v); }
void emit_tr(int v) { emit(EV_KEY, BTN_TR, v); }
void emit_hat0x(int v) { emit(EV_ABS, ABS_HAT0X, v); }
void emit_hat0y(int v) { emit(EV_ABS, ABS_HAT0Y, v); }
void emit_lz(int v) { emit(EV_ABS, ABS_Z, v); }
void emit_rz(int v) { emit(EV_ABS, ABS_RZ, v); }
void emit_start(int v) { emit(EV_KEY, BTN_START, v); }
void emit_select(int v) { emit(EV_KEY, BTN_SELECT, v); }

void write_event(int event, int val)
{
    if (val < 0 || val > 2) return;
    switch (event) {
    case KEY_W: w_pressed = val > 0; update_stick(); return;
    case KEY_A: a_pressed = val > 0; update_stick(); return;
    case KEY_S: s_pressed = val > 0; update_stick(); return;
    case KEY_D: d_pressed = val > 0; update_stick(); return;
    case KEY_UP: emit_hat0y(val ? -1 : 0); break;
    case KEY_DOWN: emit_hat0y(val ? 1 : 0); break;
    case KEY_LEFT: emit_hat0x(val ? -1 : 0); break;
    case KEY_RIGHT: emit_hat0x(val ? 1 : 0); break;
    case KEY_LEFTCTRL: emit_south(val); break;
    case KEY_LEFTALT: emit_east(val); break;
    case KEY_SPACE: emit_north(val); printf("P1 bouton 3 -> North (Y)\n"); break;
    case KEY_1: emit_start(val); break;
    default: return;
    }
    syn();
}
