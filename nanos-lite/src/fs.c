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
size_t fs_filesz(int fd) {
	return file_table[fd].size;
}
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
	size_t fs_size = fs_filesz(fd);
	//Log("in the read, fd = %d, file size = %d, len = %d, file open_offset = %d\n", fd, fs_size, len, file_table[fd].open_offset);
	switch(fd) {
		case FD_STDOUT:
		case FD_FB:
			//Log("in the fs_read fd_fb\n");
			break;
		case FD_EVENTS:
			len = events_read((void *)buf, 0,len);
			break;
		case FD_DISPINFO:
			if (file_table[fd].open_offset >= file_table[fd].size)
				return 0;
			if (file_table[fd].open_offset + len > file_table[fd].size)
				len = file_table[fd].size - file_table[fd].open_offset;
			dispinfo_read(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;	
			break;
		default:
			if(file_table[fd].open_offset >= fs_size || len == 0)
				return 0;
			if(file_table[fd].open_offset + len > fs_size)
				len = fs_size - file_table[fd].open_offset;
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

size_t fs_write(int fd, const void *buf, size_t len)
{
	size_t fs_size = fs_filesz(fd);
	//Log("in the write, fd = %d, file size = %d, len = %d, file open_offset = %d\n", fd, fs_size, len, file_table[fd].open_offset);
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:
			// call _putc()
			for(int i = 0; i < len; i++) {
					_putc(((char*)buf)[i]);
			}
			break;
		case FD_FB:
			// write to frame buffer
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:
			// write to ramdisk
			if(file_table[fd].open_offset >= fs_size)
				return 0;	
			if(file_table[fd].open_offset + len > fs_size)
				len = fs_size - file_table[fd].open_offset;
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
	size_t result = -1;

	switch(whence) {
		case SEEK_SET:
			if (offset >= 0 && offset <= file_table[fd].size) {
				file_table[fd].open_offset = offset;
				result = file_table[fd].open_offset = offset;
			}
			break;
		case SEEK_CUR:
			if ((offset + file_table[fd].open_offset >= 0) && 
					(offset + file_table[fd].open_offset <= file_table[fd].size)) {
				file_table[fd].open_offset += offset;
				result = file_table[fd].open_offset;
			}
			break;
		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size + offset;
			result = file_table[fd].open_offset;
			break;
	}
	
	return result;
}

int fs_close(int fd)
{
	return 0;
}

