#ifndef DEV_SERIAL_H
#define DEV_SERIAL_H

#include <../arch/x86_64/io.h>
#include <dev/device.h>
#include <memory/heap/liballoc.h>
#include <stddef.h>
#include <util/string.h>

#define COM1 0x3F8

int com1_write(struct device *dev, const void *buffer, size_t size,
               size_t offset);
int com1_read(struct device *dev, void *buffer, size_t size, size_t offset);
int com1_ioctl(struct device *dev, int request, void *arg);

void serial_init(int port);

void serial_write(int port, char c);
char serial_read(int port);

void dev_serial_init();

#endif // DEV_SERIAL_H