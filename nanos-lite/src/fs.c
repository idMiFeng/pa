#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;// 文件名
  size_t size; // 文件大小
  size_t disk_offset; // 文件在ramdisk中的偏移
  ReadFn read;
  WriteFn write;
  size_t open_offset;//目前文件操作的位置
} Finfo;



enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

int fs_open(const char *pathname, int flags, int mode){
  for (int i = 3; i < NR_FILES; i++) {
        if (strcmp(file_table[i].name, pathname) == 0) {
            file_table[i].open_offset = 0;
            return i;
        }
    }
  panic("file %s not found", pathname);
}


int fs_close(int fd){
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len){
  ReadFn readFn = file_table[fd].read;
  if (readFn != NULL) {
        // 特殊文件处理
        return readFn(buf, 0, len);
  }
  size_t read_len = len;
  size_t open_offset = file_table[fd].open_offset;
  size_t size = file_table[fd].size;
  size_t disk_offset = file_table[fd].disk_offset;
  if (open_offset > size) return 0;
  if (open_offset + len > size) read_len = size - open_offset;
  ramdisk_read(buf, disk_offset + open_offset, read_len);
  file_table[fd].open_offset += read_len;
  return read_len;

}

size_t fs_write(int fd, const void *buf, size_t len) {
    if (fd == 0) {
        Log("ignore write %s", file_table[fd].name);
        return 0;
    }

    WriteFn writeFn = file_table[fd].write;
    if (writeFn != NULL) {
        // 特殊文件处理
      return writeFn(buf, 0, len);
    }
    size_t write_len = len;
    size_t open_offset = file_table[fd].open_offset;
    size_t size = file_table[fd].size;
    size_t disk_offset = file_table[fd].disk_offset;
    if (open_offset > size) return 0;
    if (open_offset + len > size) write_len = size - open_offset;
    ramdisk_write(buf, disk_offset + open_offset, write_len);
    file_table[fd].open_offset += write_len;
    return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  if (fd <= 2) {
        Log("ignore lseek %s", file_table[fd].name);
        return 0;
  }

  Finfo *file = &file_table[fd];
  size_t new_offset;
  // 根据 whence 参数来计算新的指针位置
    if (whence == SEEK_SET) {
        new_offset = offset;
    } else if (whence == SEEK_CUR) {
        new_offset = file->open_offset + offset;
    } else if (whence == SEEK_END) {
        new_offset = file->size + offset;
    } else {
        Log("Invalid whence value: %d", whence);
        return -1;
    }
     // 检查新的指针位置是否在文件范围内
    if (new_offset < 0 || new_offset > file->size) {
        Log("Seek position out of bounds");
        return -1;
    }
     // 设置新的文件读写指针
    file->open_offset = new_offset;
    
    return new_offset;
}


void init_fs() {
  // TODO: initialize the size of /dev/fb
}
