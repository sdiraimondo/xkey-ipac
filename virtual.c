#include "xkey.h"

#include <sys/stat.h>

#define FULL 32767
#define DIAGONAL 23170
#define IS_PRESSED 1
#define IS_RELEASED 0

/* xkey_fd is the original (P1) uinput descriptor owned by main.c. */
static int p2_fd = -1;
static int escape_fd = -1;

struct stick_state {
	int up;
	int down;
	int left;
	int right;
	int prev_x;
	int prev_y;
};

static struct stick_state p1_stick;
static struct stick_state p2_stick;
static int p1_start_pressed;
static int p2_start_pressed;
static int escape_down;

static void emit_to(int fd, int type, int code, int value)
{
	struct input_event event = {0};

	if (fd < 0)
		return;
	event.type = type;
	event.code = code;
	event.value = value;
	if (write(fd, &event, sizeof(event)) != (ssize_t)sizeof(event))
		perror("write uinput event");
}

static void syn_to(int fd)
{
	emit_to(fd, EV_SYN, SYN_REPORT, 0);
}

void emit(int type, int code, int val)
{
	emit_to(xkey_fd, type, code, val);
}

void syn(void)
{
	syn_to(xkey_fd);
}

static void emit_p2(int type, int code, int val)
{
	emit_to(p2_fd, type, code, val);
}

static void syn_p2(void)
{
	syn_to(p2_fd);
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

static void emit_p2_north(int val) { emit_p2(EV_KEY, BTN_NORTH, val); }
static void emit_p2_south(int val) { emit_p2(EV_KEY, BTN_SOUTH, val); }
static void emit_p2_east(int val) { emit_p2(EV_KEY, BTN_EAST, val); }
static void emit_p2_start(int val) { emit_p2(EV_KEY, BTN_START, val); }

static void update_stick(int fd, struct stick_state *stick)
{
	int x = 0;
	int y = 0;

	if (stick->left && !stick->right)
		x = -FULL;
	else if (stick->right && !stick->left)
		x = FULL;

	if (stick->up && !stick->down)
		y = -FULL;
	else if (stick->down && !stick->up)
		y = FULL;

	if (x != 0 && y != 0) {
		x = x < 0 ? -DIAGONAL : DIAGONAL;
		y = y < 0 ? -DIAGONAL : DIAGONAL;
	}

	if (x == stick->prev_x && y == stick->prev_y)
		return;
	emit_to(fd, EV_ABS, ABS_X, x);
	emit_to(fd, EV_ABS, ABS_Y, y);
	syn_to(fd);
	stick->prev_x = x;
	stick->prev_y = y;
}

static int set_abs(int fd, int axis, int min, int max, int fuzz, int flat)
{
	struct uinput_abs_setup setup = {0};

	setup.code = axis;
	setup.absinfo.minimum = min;
	setup.absinfo.maximum = max;
	setup.absinfo.fuzz = fuzz;
	setup.absinfo.flat = flat;
	return ioctl(fd, UI_ABS_SETUP, &setup);
}

static int create_gamepad(int fd, const char *name)
{
	struct uinput_setup setup = {0};
	const int stick_min = -32768;
	const int stick_max = 32767;
	const int trigger_max = 255;
	const int hat_min = -1;
	const int hat_max = 1;
	const int key_bits[] = {
		BTN_SOUTH, BTN_EAST, BTN_NORTH, BTN_WEST, BTN_TL, BTN_TR,
		BTN_THUMBL, BTN_THUMBR, BTN_SELECT, BTN_START, BTN_MODE
	};
	const int abs_bits[] = {
		ABS_X, ABS_Y, ABS_RX, ABS_RY, ABS_Z, ABS_RZ, ABS_HAT0X, ABS_HAT0Y
	};
	size_t i;

	if (fd < 0)
		return -1;
	for (i = 0; i < sizeof(key_bits) / sizeof(key_bits[0]); ++i)
		ioctl(fd, UI_SET_KEYBIT, key_bits[i]);
	for (i = 0; i < sizeof(abs_bits) / sizeof(abs_bits[0]); ++i)
		ioctl(fd, UI_SET_ABSBIT, abs_bits[i]);
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_ABS);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);

	set_abs(fd, ABS_X, stick_min, stick_max, 16, 128);
	set_abs(fd, ABS_Y, stick_min, stick_max, 16, 128);
	set_abs(fd, ABS_RX, stick_min, stick_max, 16, 128);
	set_abs(fd, ABS_RY, stick_min, stick_max, 16, 128);
	set_abs(fd, ABS_Z, 0, trigger_max, 0, 0);
	set_abs(fd, ABS_RZ, 0, trigger_max, 0, 0);
	set_abs(fd, ABS_HAT0X, hat_min, hat_max, 0, 0);
	set_abs(fd, ABS_HAT0Y, hat_min, hat_max, 0, 0);

	(void)snprintf(setup.name, UINPUT_MAX_NAME_SIZE, "%s", name);
	setup.id.bustype = BUS_USB;
	setup.id.vendor = 0x45e;
	setup.id.product = 0x28e;
	setup.id.version = 0x110;
	if (ioctl(fd, UI_DEV_SETUP, &setup) < 0 ||
	    ioctl(fd, UI_DEV_CREATE) < 0)
		return -1;
	return 0;
}

static int create_escape_keyboard(void)
{
	struct uinput_setup setup = {0};

	ioctl(escape_fd, UI_SET_EVBIT, EV_KEY);
	ioctl(escape_fd, UI_SET_EVBIT, EV_SYN);
	ioctl(escape_fd, UI_SET_KEYBIT, KEY_ESC);
	(void)snprintf(setup.name, UINPUT_MAX_NAME_SIZE, "xkey combo keyboard");
	setup.id.bustype = BUS_USB;
	setup.id.vendor = 0x45e;
	setup.id.product = 0x28f;
	setup.id.version = 0x110;
	if (ioctl(escape_fd, UI_DEV_SETUP, &setup) < 0 ||
	    ioctl(escape_fd, UI_DEV_CREATE) < 0)
		return -1;
	return 0;
}

static void destroy_extra_devices(void)
{
	if (p2_fd >= 0) {
		ioctl(p2_fd, UI_DEV_DESTROY);
		close(p2_fd);
		p2_fd = -1;
	}
	if (escape_fd >= 0) {
		ioctl(escape_fd, UI_DEV_DESTROY);
		close(escape_fd);
		escape_fd = -1;
	}
}

void setup_virtual_device(void)
{
	if (create_gamepad(xkey_fd, "Microsoft X-Box 360 pad P1") < 0)
		perror("create P1 virtual gamepad");

	p2_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (p2_fd < 0 || create_gamepad(p2_fd, "Microsoft X-Box 360 pad P2") < 0) {
		perror("create P2 virtual gamepad");
		if (p2_fd >= 0) {
			close(p2_fd);
			p2_fd = -1;
		}
	}

	escape_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (escape_fd < 0 || create_escape_keyboard() < 0) {
		perror("create combo keyboard");
		if (escape_fd >= 0) {
			close(escape_fd);
			escape_fd = -1;
		}
	}
	atexit(destroy_extra_devices);
	sleep(1);
}

static void update_combo(void)
{
	int both = p1_start_pressed && p2_start_pressed;

	/* Edge-triggered press; release as soon as either Start is released. */
	if (both && !escape_down) {
		emit_to(escape_fd, EV_KEY, KEY_ESC, IS_PRESSED);
		syn_to(escape_fd);
		escape_down = TRUE;
	} else if (!both && escape_down) {
		emit_to(escape_fd, EV_KEY, KEY_ESC, IS_RELEASED);
		syn_to(escape_fd);
		escape_down = FALSE;
	}
}

void write_event(int event, int val)
{
	int pressed;

	if (val < 0 || val > 2)
		return;
	pressed = val > 0;

	/* P1: arrows are the left analog stick. */
	switch (event) {
	case KEY_UP: p1_stick.up = pressed; update_stick(xkey_fd, &p1_stick); return;
	case KEY_DOWN: p1_stick.down = pressed; update_stick(xkey_fd, &p1_stick); return;
	case KEY_LEFT: p1_stick.left = pressed; update_stick(xkey_fd, &p1_stick); return;
	case KEY_RIGHT: p1_stick.right = pressed; update_stick(xkey_fd, &p1_stick); return;
	case KEY_LEFTCTRL: emit_south(val); syn(); return;
	case KEY_LEFTALT: emit_east(val); syn(); return;
	case KEY_SPACE: emit_north(val); syn(); return;
	case KEY_1:
		p1_start_pressed = pressed;
		emit_start(val);
		syn();
		update_combo();
		return;
	default: break;
	}

	/* P2: R/F/D/G stick, A/S/Q face buttons, and 2 as Start. */
	switch (event) {
	case KEY_R: p2_stick.up = pressed; update_stick(p2_fd, &p2_stick); return;
	case KEY_F: p2_stick.down = pressed; update_stick(p2_fd, &p2_stick); return;
	case KEY_D: p2_stick.left = pressed; update_stick(p2_fd, &p2_stick); return;
	case KEY_G: p2_stick.right = pressed; update_stick(p2_fd, &p2_stick); return;
	case KEY_A: emit_p2_south(val); syn_p2(); return;
	case KEY_S: emit_p2_east(val); syn_p2(); return;
	case KEY_Q: emit_p2_north(val); syn_p2(); return;
	case KEY_2:
		p2_start_pressed = pressed;
		emit_p2_start(val);
		syn_p2();
		update_combo();
		return;
	default: return;
	}
}
