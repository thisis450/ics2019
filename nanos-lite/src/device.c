#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
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
	Log("evetns_read\n");
	char str[20];
	bool down=false;
  int key = read_key();
  Log("read_key success");
  
	if(key != _KEY_NONE)
  {
		if(key & 0x8000)
    {
			key = key ^ 0x8000;
			sprintf(str, "kd %s\n", keyname[key]);
		}
		else
    {
			sprintf(str, "ku %s\n", keyname[key]);
		}
	}
	else
  {
		sprintf(str, "t %u\n", uptime());
	}
	return strlen(str);
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
