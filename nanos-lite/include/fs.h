#ifndef __FS_H__
#define __FS_H__

#include <common.h>
size_t serial_write(const void *buf, size_t offset, size_t len);
#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

#endif
