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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB,FD_EVENTS};

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

size_t fs_read(int fd, void *buf, size_t len){
	assert(fd >= 0 && fd < NR_FILES);
	size_t rlen = -1;

	if (file_table[fd].open_offset > file_table[fd].size)
	{Log("fs_read :fd =%d,open_offset>size\n",fd);
		return 0;
	}

	if (file_table[fd].read){
		rlen = file_table[fd].read(buf, file_table[fd].open_offset, len);
		(&file_table[fd])->open_offset += rlen;
	}
	else{

		if(file_table[fd].open_offset + len > file_table[fd].size){
			len = file_table[fd].size - file_table[fd].open_offset;
		}
		rlen = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
		(&file_table[fd])->open_offset = file_table[fd].open_offset + rlen;
	}
	return rlen;
}

size_t fs_write(int fd, const void *buf, size_t len)
{
	assert(fd >= 0 && fd < NR_FILES);
	size_t wlen = -1;
	if (file_table[fd].open_offset > file_table[fd].size){
		//Log("fs_write out of size\n");
		return 0;
	}
	if(file_table[fd].write){
		wlen = file_table[fd].write(buf, file_table[fd].open_offset, len);
		(&file_table[fd])->open_offset += wlen;
	}
	else{

		if(file_table[fd].open_offset + len > file_table[fd].size){
			len = file_table[fd].size - file_table[fd].open_offset;
		}
		/* write via ramdisk API */
		wlen = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
		/* update open offset */
		(&file_table[fd])->open_offset = file_table[fd].open_offset + wlen;
	}
	return wlen;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
	//Log("fs_lseek,whence=%d\n",whence);
	assert(fd < NR_FILES && fd > FD_STDERR);
	Finfo *file = &file_table[fd];
  switch (whence){
  case SEEK_CUR:
    file->open_offset += offset;
		return  file->open_offset;
  break;
  case SEEK_SET:
  		file->open_offset = offset;
		return file->open_offset;
    break;
  case SEEK_END:
  		file->open_offset = file->size + offset;
		return file->open_offset;
    break;
  default:
  		Log("fs_lseek wrong whence=%d\n",whence);
		assert(0);
  break;
  }
  return 0;
}

int fs_close(int fd)
{
	return 0;
}

