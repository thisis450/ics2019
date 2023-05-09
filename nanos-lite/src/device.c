#include "common.h"
#include <amdev.h>
#define KEYDOWN_MASK 0x8000
size_t serial_write(const void *buf, size_t offset, size_t len) {
  char *p_buf = (char *)buf;
	for(size_t i = 0; i < len; i++){
		_putc(p_buf[i]);
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
  int keycode = read_key();
	if(keycode != _KEY_NONE){
		if(keycode & KEYDOWN_MASK)
    {
			keycode = keycode ^ KEYDOWN_MASK;
			sprintf(buf, "kd %s\n", keyname[keycode]);
		}
		else{
			sprintf(buf, "ku %s\n", keyname[keycode]);
		}
	}
	else{
		sprintf(buf, "t %u\n", uptime());
	}
	return strlen(buf);
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
	printf("[displayinfo_read] try to read offset: %lu, and len: %lu.\n", offset, len);
  strncpy((char *)buf, dispinfo + offset, len);
	return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
	printf("[fb_write] try to write offset: %lu, and size: %lu.\n", offset, len);
	/* write the len bytes from buf to the screen(offset -> coord) */
	/* calc w from len */
	int w = len / 4;
	int h = 1;
	/* convert to int unit offset */
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
	printf("dispinfo:\n%s\n", dispinfo);
}
