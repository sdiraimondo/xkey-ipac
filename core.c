#include "xkey.h"

static const char *evval[] = {"RELEASE", "PRESS", "REPEAT"};

int handle_workflow()
{
        struct input_event ev;
        int n;

        if (stop) {
                return -1;
        }

        n = read(keyboard_fd, &ev, sizeof(ev));

        if (n == -1) {
                if (errno == EINTR || errno == EAGAIN)
                        return -1;
                perror("read");
        }

        if (n != sizeof(ev)) {
                fprintf(stderr, "Partial read\n");
                return -1;
        }

        if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                printf("--------SYN REPORT--------\n");
                fflush(stdout);
                return 1;
        }

        if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2) {
                printf("%s  code=0x%04x (%d)\n", evval[ev.value], ev.code,
                       ev.code);

                if (toggled == TRUE) {
                        write_event(ev.code, ev.value);
                }
        }

        return 1;
}
