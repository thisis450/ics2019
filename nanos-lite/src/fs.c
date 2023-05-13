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
  assert(fd>=0&&fd<NR_FILES);
  //Log("fs_read:try to open fd%d",fd);
  if(fd<3||fd==FD_FB)
  {
    //Log("fs_read:try to open fd%d,system file",fd);
    return 0;
  }
    if(fd==FD_EVENTS)
  {
    return events_read(buf,0,len);
  }
    int n=fs_filesize(fd)-open_offset(fd);
    if(len>n)
  {
    len=n;
  }
 
   if(fd==FD_DISPINFO)
   {
    dispinfo_read(buf,open_offset(fd),len);
    set_open_offset(fd,open_offset(fd)+len);
    return len;
   }

  
  ramdisk_read(buf,disk_offset(fd)+open_offset(fd),len);
  set_open_offset(fd,open_offset(fd)+len);
  return len;
}
int fs_close(int fd)
{
  assert(fd>=0&&fd<NR_FILES);
  return 0;
}
size_t fs_lseek(int fd,size_t offset,int whence)
{
  switch(whence)
  {
    case SEEK_SET:
    set_open_offset(fd,offset);
    return open_offset(fd);
    case SEEK_CUR:
    set_open_offset(fd,open_offset(fd)+offset);
    return open_offset(fd);
    case SEEK_END:
    set_open_offset(fd,fs_filesize(fd));
    return open_offset(fd);
    default:
    printf("fs_lseek: whence=%d,unhandled\n",whence);
		assert(0);

  }
  return 0;
}
size_t fs_write(int fd, const void *buf, size_t len)
{
	// if(fd == 1 || fd == 2){
	// 	size_t i;
	// 	for(i = 0; i < len; i++){
	// 		_putc(((char *)buf)[i]);
	// 	}
  //   return len;
	// }
  size_t ret;
	int n=fs_filesize(fd)-open_offset(fd);
    if(len>n)
  {
    len=n;
  }
	if (file_table[fd].write!=NULL)
  {
		ret=file_table[fd].write(buf,file_table[fd].open_offset,len);
  }
	else
	{

		ret=ramdisk_write(buf,disk_offset(fd)+open_offset(fd),len);
	}
	set_open_offset(fd,open_offset(fd)+len);
	return ret;
}