#include "xkey.h"

static const char *evval[] = {"RELEASE", "PRESS", "REPEAT"};
static int has_scan_code = 0; /* set to 1 when an EV_MSC/MSC_SCAN was seen
                                  since the last SYN_REPORT */

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

        if (ev.type == EV_MSC && ev.code == MSC_SCAN) {
                has_scan_code = 1;
                return 1;
        }

        if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                printf("--------SYN REPORT--------\n");
                fflush(stdout);
                has_scan_code = 0;
                return 1;
        }

        if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2) {
                printf("%s  code=0x%04x (%d) scan=%d\n", evval[ev.value],
                       ev.code, ev.code, has_scan_code);

                /* Observed I-PAC noise: spurious KEY_5 (coin) events with
                 * no preceding MSC_SCAN. Drop them before they even reach
                 * the debounce layer in virtual.c. */
                if (ev.code == KEY_5 && !has_scan_code) {
                        printf("  -> ignored (no MSC_SCAN, likely noise)\n");
                        fflush(stdout);
                        return 1;
                }

                if (toggled == TRUE) {
                        write_event(ev.code, ev.value);
                }
        }

        return 1;
}
