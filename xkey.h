#ifndef XKEY_H
#define XKEY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

extern int keyboard_fd;
extern int xkey_fd;
extern volatile sig_atomic_t stop;
extern int toggled;

int list_devices(void);
void handle_signal(int sig);
int handle_workflow(void);
void setup_virtual_device(void);
void write_event(int event, int val);
void emit(int type, int code, int val);
void syn(void);
void emit_north(int val);
void emit_south(int val);
void emit_east(int val);
void emit_west(int val);
void emit_tl(int val);
void emit_tr(int val);
void emit_hat0x(int val);
void emit_hat0y(int val);
void emit_lz(int val);
void emit_rz(int val);
void emit_start(int val);
void emit_select(int val);

#endif
