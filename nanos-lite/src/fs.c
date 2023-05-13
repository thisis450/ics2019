#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB,FD_EVENTS,FD_FBSYNC,FD_DISPINFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t fbsync_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0,  0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0,invalid_read, serial_write},
  {"stderr", 0, 0, 0,invalid_read, serial_write},
  {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  {"/dev/events", 0, 0, 0, events_read, invalid_write},
  {"/dev/fbsync", 0, 0, 0, invalid_read, fbsync_write},
  {"/proc/dispinfo", 128, 0, 0, dispinfo_read, invalid_write}, 
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
	int fd = fs_open("/dev/fb", 0, 0);
	file_table[fd].size = sizeof(uint32_t) * screen_height() * screen_width();
}

int fs_open(const char *pathname, int flags, int mode){
	int fd = -1;
	for (int i = 0; i < NR_FILES; i++){
		if(!strcmp(pathname, file_table[i].name)){
			fd = i;
			file_table[i].open_offset = 0;
			break;
		}
	}
	assert(fd >= 0);
	Log("fs_open file %s, fd: %d\n", pathname, fd);
	return fd;
}
int fs_close(int fd)
{
	return 0;
}
size_t fs_filesz(int fd)
{
  return file_table[fd].size;
}
size_t disk_offset(int fd)
{
  return file_table[fd].disk_offset;
}
size_t get_open_offset(int fd)
{
  return file_table[fd].open_offset;
}
void set_open_offset(int fd,size_t n)
{
  assert(n>=0);
  if(n>file_table[fd].size)
  {
    n=file_table[fd].size;
  }
  file_table[fd].open_offset=n;
}
size_t fs_lseek(int fd,size_t offset,int whence)
{
  switch(whence)
  {
    case SEEK_SET:
    set_open_offset(fd,offset);
    return get_open_offset(fd);
    case SEEK_CUR:
    set_open_offset(fd,get_open_offset(fd)+offset);
    return get_open_offset(fd);
    case SEEK_END:
    set_open_offset(fd,fs_filesz(fd)+offset);
    return get_open_offset(fd);
    default:
    Log("wrong whence=%d\n",whence);
    return -1;
  }
}
size_t fs_read(int fd,void*buf,size_t len)
{
  if(fd<4)
  {
Log("fs_read wrong fd<4\n");
return 0;
  }
    if(fd==FD_EVENTS)
  {
    return events_read(buf,0,len);
  }
  int n=fs_filesz(fd)-get_open_offset(fd);
  if(n>len)
  {
    n=len;
  }
  if(fd==FD_DISPINFO)
  {
    dispinfo_read(buf,get_open_offset(fd),n);
  }
  else
  {
    ramdisk_read(buf,disk_offset(fd)+get_open_offset(fd),n);
  }
  set_open_offset(fd,get_open_offset(fd)+n);
  return n;
}
size_t fs_write(int fd,void*buf,size_t len)
{
  if(fd<3||fd==FD_DISPINFO)
  {
    Log("fs_write wrong fd=%d\n",fd);
    return 0;
  }
  int n=fs_filesz(fd)-get_open_offset(fd);
    if(n>len)
  {
    n=len;
  }
  if(fd==FD_FB)
  {
    fb_write(buf,get_open_offset(fd),n);

  }
  else
  {
    ramdisk_write(buf,disk_offset(fd)+get_open_offset(fd),n);
  }
  set_open_offset(fd,get_open_offset(fd)+n);
  return n;
}

