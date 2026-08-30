#include "xkey.h"
#include <glob.h>

int list_devices(void)
{
        glob_t matches;
        size_t i;
        int result;

        result = glob("/dev/input/event*", 0, NULL, &matches);
        if (result == GLOB_NOMATCH) {
                fprintf(stderr, "No input event devices found.\n");
                return -1;
        }
        if (result != 0) {
                fprintf(stderr, "Unable to scan /dev/input/event*.\n");
                return -1;
        }

        printf("Available input devices:\n");
        for (i = 0; i < matches.gl_pathc; ++i) {
                int fd = open(matches.gl_pathv[i], O_RDONLY | O_NONBLOCK);
                char name[256] = "Unknown";
                if (fd >= 0) {
                        if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
                                snprintf(name, sizeof(name), "Unknown (%s)",
                                         strerror(errno));
                        close(fd);
                }
                printf("  %s: %s\n", matches.gl_pathv[i], name);
        }
        globfree(&matches);
        return 0;
}

void handle_signal(int sig)
{
        (void)sig;
        stop = 1;
}
