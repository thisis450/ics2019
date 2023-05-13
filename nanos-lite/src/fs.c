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
  // // TODO: initialize the size of /dev/fb
  	int fd = fs_open("/bin/text", 0, 0);
	Finfo * file = &file_table[fd];
	file->size = sizeof(uint32_t) * screen_height() * screen_width();
}
size_t fs_filesize(int fd)
{
 assert(fd>=0&&fd<NR_FILES);
 return file_table[fd].size;
}
size_t disk_offset(int fd)
{
  assert(fd>=0&&fd<NR_FILES);
  return file_table[fd].disk_offset;
}
size_t open_offset(int fd)
{
  assert(fd>=0&&fd<NR_FILES);
  return file_table[fd].open_offset;
}
void set_open_offset(int fd,size_t offset)
{
  assert(fd>=0&&fd<NR_FILES);
  if(offset>file_table[fd].size)
  {
    offset=file_table[fd].size;

  }
  file_table[fd].open_offset=offset;
}



int fs_open(const char *filename,int flags,int mode)
{
	for (int i = 0; i < NR_FILES; i++){
		if(strcmp(filename, file_table[i].name)==0){
      Log("fs_open open file %s, fd: %d\n", filename, i);
			return i;
      
		}
	}
	panic("fs_open:no such file named %s",filename);
	return -1;
}
size_t fs_read(int fd,void*buf,size_t len)
{
	assert(fd >= 0 && fd < NR_FILES);
	/* operate reading via ramdisk APIs */
	Finfo file = file_table[fd];
	size_t rlen = -1;

	/* set the correct read size */
	if (file.open_offset > file.size)
	{
		printf("[fs_read failed] file %d current open offset %lu > file size %lu.\n", fd, file.open_offset, file.size);
		return 0; //EOF
	}

	if (file.read){
		rlen = file.read(buf, file.open_offset, len);
		(&file_table[fd])->open_offset += rlen;
	}
	else{
		/* set the correct read size */
		/* set correct base offset */
		size_t base_offset = file.open_offset < 0 ? 0 : file.open_offset;
		len = file.open_offset < 0 ? len + file.open_offset : len;
		if (len < 0){
			return 0;
		}
		if(base_offset + len > file.size){
			len = file.size - base_offset;
		}
		/* read via ramdisk API */
		rlen = ramdisk_read(buf, file.disk_offset + base_offset, len);
		/* update open offset */
		(&file_table[fd])->open_offset = base_offset + rlen;
	}
	return rlen;
}
int fs_close(int fd)
{
  assert(fd>=0&&fd<NR_FILES);
  return 0;
}
size_t fs_lseek(int fd,size_t offset,int whence)
{
	printf("[fs_lseek] try to seek fd %d, offset: %lu, whence: %d\n", fd, offset, whence);
	assert(fd < NR_FILES && fd > FD_STDERR);
	Finfo *file = &file_table[fd];
	size_t ret = -1;
	if(whence == SEEK_SET){
		file->open_offset = offset;
		ret = offset;
	}
	/* file offset is set to its current location plus offset bytes */
	else if(whence == SEEK_CUR){
		file->open_offset += offset;
		ret = file->open_offset;
	}
	/* file offset is set to the size of the file plus offset bytes */
	else if(whence == SEEK_END){
		file->open_offset = file->size + offset;
		ret = file->open_offset;
	}
	/* should not reach here! */
	else{
		printf("[fs_lseek] should not reach here.\n");
		assert(0);
	}
	return ret;
}
size_t fs_write(int fd, const void *buf, size_t len)
{
	assert(fd >= 0 && fd < NR_FILES);
	size_t wlen = -1;
	Finfo file = file_table[fd];
	if (file.open_offset > file.size){
		printf("[fs_write failed] file %d current open offset %lu> file size %lu.\n", fd, file.open_offset, file.size);
		return 0;
	}
	if(file.write){
		wlen = file.write(buf, file.open_offset, len);
		(&file_table[fd])->open_offset += wlen;
	}
	else{
		/* set correct base offset */
		size_t base_offset = file.open_offset < 0 ? 0 : file.open_offset;
		len = file.open_offset < 0 ? len + file.open_offset : len;
		if(len < 0){
			return 0;
		}
		if(base_offset + len > file.size){
			len = file.size - base_offset;
		}
		/* write via ramdisk API */
		wlen = ramdisk_write(buf, file.disk_offset + base_offset, len);
		/* update open offset */
		(&file_table[fd])->open_offset = base_offset + wlen;
	}
	return wlen;
}