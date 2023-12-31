#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
	//_yield();
	for(size_t i = 0; i < len; i++){
		_putc(((char *)buf)[i]);
	}
	return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
	//_yield();
	//Log("evetns_read\n");
  int key = read_key();

  //Log("read_key suceesss\n");
  //Log("key is%d\n",key);
	if(key != _KEY_NONE)
  {		//Log("key_event\n");
		if(key & 0x8000)
    {
			key = key ^ 0x8000;
			change_front_program(key);
			sprintf(buf, "kd %s\n", keyname[key]);
			//Log("buf now is %s\n",buf);
		}
		else
    {
			sprintf(buf, "ku %s\n", keyname[key]);
			//Log("buf now is %s\n",buf);
		}
	}
	else
  {		//Log("time_event\n");
  //Log("time is %d",uptime());
		sprintf(buf, "t %d\n", uptime());
		//Log("buf now is %s\n",buf);
	}
	//Log("finish events_read\n");
	return strlen(buf);
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
	//Log("displayinfo_read offset: %d, and len: %d.\n", offset, len);
    strncpy((char *)buf, dispinfo + offset, len);
	return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
	//_yield();
  	//Log("fb_write offset: %d, and size: %d.\n", offset, len);
	int w = len / 4;
	int h = 1;
	offset = offset / 4;
	int x = offset % screen_width();
	int y = offset / screen_width();
	draw_rect((uint32_t *)buf, x, y, w, h);
	return len;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
draw_sync();
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  memset(dispinfo, 0, 128 * sizeof(char));
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
  Log("dispinfo:\n%s\n", dispinfo);
}
