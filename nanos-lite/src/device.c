#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; ++i) putch(*((char *)buf + i));
  return len;
}

#define TEMP_BUF_SIZE 32
static char temp_buf[TEMP_BUF_SIZE]; // for events_read
size_t events_read(void *buf, size_t offset, size_t len) {
   memset(temp_buf, 0, TEMP_BUF_SIZE); // reset

  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) return 0;
  const char *name = keyname[ev.keycode];
  int ret = ev.keydown ? sprintf(temp_buf, "kd %s\n", name) : sprintf(temp_buf, "ku %s\n", name);
  // ret -> exclude terminating null character
  if (ret >= len) {
      strncpy(buf, temp_buf, len - 1);
      ret = len;
  } else {
      strncpy(buf, temp_buf, ret);
  }
  return ret; // ret -> include terminating null character
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
