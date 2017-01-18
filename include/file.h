#ifndef TARI_FILE
#define TARI_FILE

#include <kos.h>

#include "stdint.h"

typedef struct {
  uint32_t length;
  void* data;
} Buffer;

#define fileOpen(x,y) fs_open(x,y)
#define fileRead(x,y,z) fs_read(x,y,z)
#define fileWrite(x,y,z) fs_write(x,y,z)
#define fileSeek(x,y,z) fs_seek(x,y,z)
#define fileTell(x) fs_tell(x)
#define fileTotal(x) fs_total(x)
#define fileClose(x) fs_close(x)
#define fileUnlink(x) fs_unlink(x)
#define fileMemoryMap(x) fs_mmap(x)
Buffer fileToBuffer(char* path);

char* getPureFileName(char* path);

#endif
