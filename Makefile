CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = xkey
SRCS = main.c virtual.c
OBJS = $(SRCS:.c=.o)

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
SYSTEMD_DIR = /etc/systemd/system
CONFIG_FILE = /etc/xkey.conf
SERVICE_NAME = xkey.service

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) -o $@ $^

%.o: %.c
    $(CC) $(CFLAGS) -c -o $@ $<

clean:
    rm -f $(OBJS) $(TARGET)

install: $(TARGET)
    @echo "Installing $(TARGET) to $(BINDIR)..."
    install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)
    @echo "Installing systemd service..."
    install -Dm644 xkey.service $(SYSTEMD_DIR)/$(SERVICE_NAME)
    @if [ ! -f $(CONFIG_FILE) ]; then \
        echo ""; \
        echo "=================================================="; \
        echo "No configuration found at $(CONFIG_FILE)."; \
        echo "Running $(TARGET) now to select your input device..."; \
        echo "=================================================="; \
        echo ""; \
        $(BINDIR)/$(TARGET) --select-only; \
    fi
    systemctl daemon-reload
    systemctl enable $(SERVICE_NAME)
    systemctl start $(SERVICE_NAME)
    @echo ""
    @echo "Installation complete. Check status with:"
    @echo "  sudo systemctl status $(SERVICE_NAME)"

uninstall:
    @echo "Stopping and disabling service..."
    -systemctl stop $(SERVICE_NAME)
    -systemctl disable $(SERVICE_NAME)
    rm -f $(SYSTEMD_DIR)/$(SERVICE_NAME)
    rm -f $(BINDIR)/$(TARGET)
    systemctl daemon-reload
    @echo "Uninstalled. Config file $(CONFIG_FILE) was kept (remove manually if desired)."
