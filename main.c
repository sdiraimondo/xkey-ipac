#include "xkey.h"

int keyboard_fd;
int xkey_fd;
volatile sig_atomic_t stop;
int toggled;

int main()
{
        struct sigaction sa = {0};
        char dev[256];
        int event_num;

        stop = 0;
        toggled = TRUE;

        sa.sa_handler = handle_signal;
        sigaction(SIGINT, &sa, NULL);

        if (list_devices() < 0) {
                return EXIT_FAILURE;
        }

        printf("\nEnter the event number for your keyboard: ");
        if (scanf("%d", &event_num) != 1) {
                fprintf(stderr, "Invalid input\n");
                return EXIT_FAILURE;
        }

        snprintf(dev, sizeof(dev), "/dev/input/event%d", event_num);

        keyboard_fd = open(dev, O_RDONLY);
        xkey_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

        if (keyboard_fd < 0) {
                fprintf(stderr, "Cannot open %s: %s\n", dev, strerror(errno));
                return EXIT_FAILURE;
        }

        if (xkey_fd < 0) {
                fprintf(stderr, "CANNOT CREATE VIRTUAL DEVICE: %s",
                        strerror(errno));
                return EXIT_FAILURE;
        }

        sleep(2);
        ioctl(keyboard_fd, EVIOCGRAB, 1);

        setup_virtual_device();

        while (1) {
                int res = handle_workflow();
                if (res < 0) {
                        break;
                }
        }

        ioctl(keyboard_fd, EVIOCGRAB, FALSE);
        close(keyboard_fd);
        ioctl(xkey_fd, UI_DEV_DESTROY);
        close(xkey_fd);
        return EXIT_SUCCESS;
}
