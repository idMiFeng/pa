#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  SDL_Surface* IMG_Load(const char *filename) {
    FILE * fp = fopen(filename, "r");
    if (!fp) return NULL;

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);

    rewind(fp);
    unsigned char * buf = (unsigned char *)malloc(size * sizeof(unsigned char));
    assert(fread(buf, 1, size, fp) == size);

    SDL_Surface * surface = STBIMG_LoadFromMemory(buf, size);
    assert(surface != NULL);

    fclose(fp);
    free(buf);

    return surface;
}
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
