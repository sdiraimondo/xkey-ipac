#include "xkey.h"

static int w_pressed;
static int a_pressed;
static int s_pressed;
static int d_pressed;
static int prev_x;
static int prev_y;

#define FULL 32767
#define DIAGONAL 23170

static void update_stick()
{
        int x = 0;
        int y = 0;

        if (a_pressed && !d_pressed)
                x = -FULL;
        else if (d_pressed && !a_pressed)
                x = FULL;
        else
                x = 0;

        if (w_pressed && !s_pressed)
                y = -FULL;
        else if (s_pressed && !w_pressed)
                y = FULL;
        else
                y = 0;

        if (x != 0 && y != 0) {
                x = (x < 0) ? -DIAGONAL : DIAGONAL;
                y = (y < 0) ? -DIAGONAL : DIAGONAL;
        }

        if (x != prev_x || y != prev_y) {
                emit(EV_ABS, ABS_X, x);
                emit(EV_ABS, ABS_Y, y);
                syn();
                prev_x = x;
                prev_y = y;
        }
}

static inline void SET_ABS(int axis, int min, int max, int fuzz, int flat)
{
        struct uinput_abs_setup abs_setup;
        memset(&abs_setup, 0, sizeof(abs_setup));
        abs_setup.code = axis;
        abs_setup.absinfo.minimum = min;
        abs_setup.absinfo.maximum = max;
        abs_setup.absinfo.fuzz = fuzz;
        abs_setup.absinfo.flat = flat;
        ioctl(xkey_fd, UI_ABS_SETUP, &abs_setup);
}

void setup_virtual_device()
{
        struct uinput_setup usetup;

        memset(&usetup, 0, sizeof(usetup));

        snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "Microsoft X-Box 360 pad");

        usetup.id.bustype = BUS_USB;
        usetup.id.vendor = 0x45e;case KEY_SPACE:
        printf("P1 bouton 3 -> North (Y)\n");
        emit_north(val);
        break;
        usetup.id.product = 0x28e;
        usetup.id.version = 0x110;
        usetup.ff_effects_max = 16;

        ioctl(xkey_fd, UI_SET_EVBIT, EV_KEY);
        ioctl(xkey_fd, UI_SET_EVBIT, EV_ABS);
        ioctl(xkey_fd, UI_SET_EVBIT, EV_SYN);
        ioctl(xkey_fd, UI_SET_EVBIT, EV_FF);

        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_SOUTH);
        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_EAST);
        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_NORTH);
        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_WEST);

        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_TL);
        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_TR);

        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_THUMBL);
        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_THUMBR);

        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_SELECT);
        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_START);
        ioctl(xkey_fd, UI_SET_KEYBIT, BTN_MODE);

        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_X);
        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_Y);
        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_RX);
        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_RY);
        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_Z);
        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_RZ);
        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_HAT0X);
        ioctl(xkey_fd, UI_SET_ABSBIT, ABS_HAT0Y);

        int stick_min = -32768;
        int stick_max = 32767;

        int trigger_min = 0;
        int trigger_max = 255;

        int hat_min = -1;
        int hat_max = 1;

        SET_ABS(ABS_X, stick_min, stick_max, 16, 128);
        SET_ABS(ABS_Y, stick_min, stick_max, 16, 128);
        SET_ABS(ABS_RX, stick_min, stick_max, 16, 128);
        SET_ABS(ABS_RY, stick_min, stick_max, 16, 128);

        SET_ABS(ABS_Z, trigger_min, trigger_max, 0, 0);
        SET_ABS(ABS_RZ, trigger_min, trigger_max, 0, 0);

        SET_ABS(ABS_HAT0X, hat_min, hat_max, 0, 0);
        SET_ABS(ABS_HAT0Y, hat_min, hat_max, 0, 0);

        ioctl(xkey_fd, UI_SET_FFBIT, FF_RUMBLE);
        ioctl(xkey_fd, UI_SET_FFBIT, FF_PERIODIC);
        ioctl(xkey_fd, UI_SET_FFBIT, FF_SQUARE);
        ioctl(xkey_fd, UI_SET_FFBIT, FF_TRIANGLE);
        ioctl(xkey_fd, UI_SET_FFBIT, FF_SINE);
        ioctl(xkey_fd, UI_SET_FFBIT, FF_GAIN);
        ioctl(xkey_fd, UI_DEV_SETUP, &usetup);
        ioctl(xkey_fd, UI_DEV_CREATE);

        sleep(1);
}

void emit(int type, int code, int val)
{
        struct input_event ie;
        ie.type = type;
        ie.code = code;
        ie.value = val;
        ie.time.tv_sec = 0;
        ie.time.tv_usec = 0;
        write(xkey_fd, &ie, sizeof(ie));
}

void emit_north(int val) { emit(EV_KEY, BTN_NORTH, val); }
void emit_south(int val) { emit(EV_KEY, BTN_SOUTH, val); }
void emit_east(int val) { emit(EV_KEY, BTN_EAST, val); }
void emit_west(int val) { emit(EV_KEY, BTN_WEST, val); }
void emit_tl(int val) { emit(EV_KEY, BTN_TL, val); }
void emit_tr(int val) { emit(EV_KEY, BTN_TR, val); }
void emit_hat0x(int val) { emit(EV_ABS, ABS_HAT0X, val); }
void emit_hat0y(int val) { emit(EV_ABS, ABS_HAT0Y, val); }
void emit_lz(int val) { emit(EV_ABS, ABS_Z, val); }
void emit_rz(int val) { emit(EV_ABS, ABS_RZ, val); }
void emit_start(int val) { emit(EV_KEY, BTN_START, val); }
void emit_select(int val) { emit(EV_KEY, BTN_SELECT, val); }

void syn() { emit(EV_SYN, SYN_REPORT, 0); }

void write_event(int event, int val)
{
        if (val < 0 || val > 2)
                return;

        switch (event) {
        case KEY_W:
                w_pressed = (val > 0);
                update_stick();
                return;
        case KEY_A:
                a_pressed = (val > 0);
                update_stick();
                return;
        case KEY_S:
                s_pressed = (val > 0);
                update_stick();
                return;
        case KEY_D:
                d_pressed = (val > 0);
                update_stick();
                return;
        case KEY_UP:
                printf("P1 up -> hat0y = -1\n");
                emit_hat0y(val ? -1 : 0);
                break;
        case KEY_DOWN:
                printf("P1 down -> hat0y = 1\n");
                emit_hat0y(val ? 1 : 0);
                return;
        case KEY_LEFT:
                printf("P1 left -> hat0x = -1\n");
                emit_hat0x(val ? -1 : 0);
                return;
        case KEY_RIGHT:
                printf("P1 right -> hat0x = 1\n");
                emit_hat0x(val ? 1 : 0);
                return;
        case KEY_LEFTCTRL:
                printf("P1 button 1 -> South (A)\n");
                emit_south(val);
                return;
        case KEY_LEFTALT:
                printf("P1 button 2 -> East (B)\n");
                emit_east(val);
                return;
        case KEY_SPACE:
                printf("P1 bouton 3 -> North (Y)\n");
                emit_north(val);
                return;
        case KEY_1:
                printf("P1 Start -> Start\n");
                emit_start(val);
                return;
        default:
                return;
        }
        syn();
}
