#ifndef JOSHUA
#define JOSHUA
#include <linux/ioctl.h>

#define JOSHUA_MAX 40

#define JOSHUA_GET_QUOTE _IOR(0xff, 1, char *)

#endif