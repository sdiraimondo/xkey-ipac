#include "xkey.h"
#include <dirent.h>
#include <ctype.h>

int load_config_device_name(char *out_name, size_t out_size)
{
    FILE *f = fopen(XKEY_CONFIG_PATH, "r");
    char line[XKEY_MAX_NAME];

    if (!f)
        return -1;

    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    /* Strip trailing newline. */
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }

    if (len == 0)
        return -1;

    snprintf(out_name, out_size, "%s", line);
    return 0;
}

int save_config_device_name(const char *name)
{
    FILE *f = fopen(XKEY_CONFIG_PATH, "w");

    if (!f)
        return -1;

    fprintf(f, "%s\n", name);
    fclose(f);
    return 0;
}

int find_event_by_name(const char *name, char *out_path, size_t out_size)
{
    DIR *dir = opendir("/dev/input");
    struct dirent *entry;
    int found = -1;

    if (!dir)
        return -1;

    while ((entry = readdir(dir)) != NULL) {
        char path[300];
        char devname[XKEY_MAX_NAME];
        int fd;

        if (strncmp(entry->d_name, "event", 5) != 0)
            continue;

        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);

        fd = open(path, O_RDONLY);
        if (fd < 0)
            continue;

        if (ioctl(fd, EVIOCGNAME(sizeof(devname)), devname) >= 0) {
            if (strcmp(devname, name) == 0) {
                snprintf(out_path, out_size, "%s", path);
                close(fd);
                found = 0;
                break;
            }
        }
        close(fd);
    }

    closedir(dir);
    return found;
}
