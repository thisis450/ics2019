#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;//文件在ramdisk中的偏移
  size_t open_offset; // 文件被打开之后的读写指针
  ReadFn read;
  WriteFn write;
} Finfo;
//typedef size_t ssize_t;
//typedef size_t off_t;
enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS,FD_FB,FD_SYNC,FD_DISP};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t fbsync_write(const void *buf, size_t offset, size_t len);
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0,0, invalid_read, invalid_write},
  {"stdout", 0, 0,0, invalid_read, serial_write},
  {"stderr", 0, 0,0, invalid_read, serial_write},
  {"/dev/events", 0, 0,0, events_read, invalid_write},
  {"/dev/fb",0,0,0,invalid_read,fb_write},
  {"/dev/fbsync",0,0,0,invalid_read,fbsync_write},
  {"/proc/dispinfo",128,0,0,dispinfo_read,invalid_write},
#include "files.h"
};
#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))
int fs_open(const char *pathname)
{
	//TODO();
	for (int i=0;i<NR_FILES;++i)
		if (strcmp(pathname,file_table[i].name)==0)
		{
			file_table[i].open_offset=0;
			return i;
		}
	assert(0);//should not reach here
}
size_t fs_read(int fd, void *buf, size_t len)
{
	//TODO();
	size_t true_len;
	//Log("start");
	//Log("len=%d,size=%d,open=%d\n",len,file_table[fd].size,file_table[fd].open_offset);
	if (fd ==FD_EVENTS||file_table[fd].open_offset+len<file_table[fd].size)
		true_len=len;
	else
		true_len=file_table[fd].size-file_table[fd].open_offset;
	size_t ret;
	if (file_table[fd].read!=NULL)
  {
    ret=file_table[fd].read(buf,file_table[fd].open_offset,true_len);
  }	
	else
  {
		ret=ramdisk_read(buf,file_table[fd].disk_offset+file_table[fd].open_offset,true_len);
  }
	file_table[fd].open_offset+=ret;
	//if (file_table[fd].open_offset==4548) while(1);
	return ret;
}
size_t fs_write(int fd,const void *buf,size_t len)
{
	
	size_t ret;
	if (file_table[fd].write!=NULL){
		ret=file_table[fd].write(buf,file_table[fd].open_offset,len);
  }
	else
	{
		size_t true_len;
		if (file_table[fd].open_offset+len<file_table[fd].size)
			true_len=len;
		else
			true_len=file_table[fd].size-file_table[fd].open_offset;
		ret=ramdisk_write(buf,file_table[fd].disk_offset+file_table[fd].open_offset,true_len);
	}
	file_table[fd].open_offset+=ret;
	return ret;

}
int fs_close(int fd)
{
	//TODO();
	return 0;
}
size_t fs_lseek(int fd,size_t offset,int whence)
{
	switch (whence)
	{
		case SEEK_SET:
    file_table[fd].open_offset=offset;
    break;
		case SEEK_CUR:
    file_table[fd].open_offset+=offset;
    break;
		case SEEK_END:
    file_table[fd].open_offset=file_table[fd].size+offset;
    break;
		default:
    Log("wrong whence=%d\n",whence);
    return -1;
	}
	if (file_table[fd].open_offset>file_table[fd].size)
  { 
  file_table[fd].open_offset=file_table[fd].size;
  }
	return file_table[fd].open_offset;
}
void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size=screen_width()*screen_height()*4;
}