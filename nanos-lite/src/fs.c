#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
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
  {"stdin", 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, invalid_read, invalid_write},
  {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
size_t fs_filesize(int fd)
{
 assert(fd>=0&&fd<NR_FILES);
 return file_table[fd].size;
}
int disk_offset(int fd)
{
  assert(fd>=0&&fd<NR_FILES);
  return file_table[fd].disk_offset;
}

int fs_open(const char *filename,int flags,int mode)
{
	for (int i = 0; i < NR_FILES; i++){
		if(strcmp(filename, file_table[i].name)==0){
      printf("fs_open open file %s, fd: %d\n", filename, i);
			return i;
      
		}
	}
	panic("fs_open:no such file named %s",filename);
	return -1;
}
size_t fs_read(int fd,void*buf,size_t len)
{
  assert(fd>=0&&fd<NR_FILES);
  if(fd<3)
  {
    Log("fs_read:try to open fd%d,system file",fd);
    return 0;
  }
  return 0;
}
