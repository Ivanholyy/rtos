#include "null.h"

void dev_null_init() {
    device_t *dev = kmalloc(sizeof(device_t));
    memcpy(dev->name, "null", DEVICE_NAME_MAX);
    dev->type  = DEVICE_TYPE_CHAR;
    dev->read  = dev_null_read;
    dev->write = dev_null_write;
    dev->data  = NULL;
    register_device(dev);
}

int dev_null_read(struct device *dev, void *buffer, size_t size,
                  size_t offset) {
    (void)dev;
    (void)offset;
    for (size_t i = 0; i < size; i++) {
        ((char *)buffer)[i] = 0;
    }
    return 0;
}
int dev_null_write(struct device *dev, const void *buffer, size_t size,
                   size_t offset) {
    (void)dev;
    (void)offset;
    (void)buffer;
    (void)size;
    return 0;
}