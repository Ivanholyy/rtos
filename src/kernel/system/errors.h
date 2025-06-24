#ifndef ERRORS_H
#define ERRORS_H 1

#define EOK      0
#define ENOMEM   1  // no memory
#define EINVAL   2  // invalid argument
#define EIO      3  // input/output error
#define EACCES   4  // permission denied
#define ENOENT   5  // no such file or directory
#define EBUSY    6  // device or resource busy
#define EUNFB    7  // undefined behavior
#define ENOIMPL  8  // no implementation
#define ENOTDIR  9  // no such directory
#define ENULLPTR 10 // NULL pointer found

#define EPCIENOENT 101 // special
#endif                 // ERRORS_H
