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
                        if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
                                snprintf(name, sizeof(name), "Unknown (%s)",
                                         strerror(errno));
                        }
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

int load_config_device_name(char *out_name, size_t out_size)
{
        FILE *f = fopen(XKEY_CONFIG_PATH, "r");
        char line[XKEY_MAX_NAME];

        if (!f) {
                return -1;
        }

        if (!fgets(line, sizeof(line), f)) {
                fclose(f);
                return -1;
        }
        fclose(f);

        /* strip trailing newline */
        line[strcspn(line, "\r\n")] = '\0';

        if (strlen(line) == 0) {
                return -1;
        }

        snprintf(out_name, out_size, "%s", line);
        return 0;
}

int save_config_device_name(const char *name)
{
        FILE *f = fopen(XKEY_CONFIG_PATH, "w");

        if (!f) {
                fprintf(stderr, "Cannot write %s: %s\n",
                        XKEY_CONFIG_PATH, strerror(errno));
                return -1;
        }

        fprintf(f, "%s\n", name);
        fclose(f);
        return 0;
}

int find_event_by_name(const char *name, char *out_path, size_t out_size)
{
        glob_t matches;
        size_t i;
        int result;

        result = glob("/dev/input/event*", 0, NULL, &matches);
        if (result != 0) {
                return -1;
        }

        for (i = 0; i < matches.gl_pathc; ++i) {
                int fd = open(matches.gl_pathv[i], O_RDONLY | O_NONBLOCK);
                char devname[XKEY_MAX_NAME] = "Unknown";

                if (fd < 0) {
                        continue;
                }

                if (ioctl(fd, EVIOCGNAME(sizeof(devname)), devname) < 0) {
                        close(fd);
                        continue;
                }
                close(fd);

                if (strcmp(devname, name) == 0) {
                        snprintf(out_path, out_size, "%s",
                                 matches.gl_pathv[i]);
                        globfree(&matches);
                        return 0;
                }
        }

        globfree(&matches);
        return -1;
}
