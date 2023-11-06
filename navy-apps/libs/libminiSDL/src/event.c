#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  unsigned buf_size = 32;
  char *buf = (char *)malloc(buf_size * sizeof(char));
  if (NDL_PollEvent(buf, buf_size) == 1) {
      if (strncmp(buf, "kd", 2) == 0) {
          ev->key.type = SDL_KEYDOWN;
      } else {
          ev->key.type = SDL_KEYUP;
      }

      int flag = 0;
      for (unsigned i = 0; i < sizeof(keyname) / sizeof(keyname[0]); ++i) {
          if (strncmp(buf + 3, keyname[i], strlen(buf) - 4) == 0
                  && strlen(keyname[i]) == strlen(buf) - 4) {
              flag = 1;
              ev->key.keysym.sym = i;
              break;
          }
      }

      
      free(buf);
      return 1;
  } else {
      return 0;
  }
}

int SDL_WaitEvent(SDL_Event *ev) {
    unsigned buf_size = 32;
    char *buf = (char *)malloc(buf_size * sizeof(char));

    while (NDL_PollEvent(buf, buf_size) == 0); // wait ...

    if (strncmp(buf, "kd", 2) == 0)
        ev->key.type = SDL_KEYDOWN;
    else
        ev->key.type = SDL_KEYUP;


    int flag = 0;
    for (unsigned i = 0; i < sizeof(keyname) / sizeof(keyname[0]); ++i) {
        if (strncmp(buf + 3, keyname[i], strlen(buf) - 4) == 0
                && strlen(keyname[i]) == strlen(buf) - 4) {
            flag = 1;
            ev->key.keysym.sym = i;
            break;
        }
    }

   

    free(buf);
    return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
