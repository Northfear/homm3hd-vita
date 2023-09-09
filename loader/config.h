#ifndef __CONFIG_H__
#define __CONFIG_H__

#if GRAPHICS_API_PVR
// some memory has to be left for internal PVR stuff too
#define MEMORY_NEWLIB_MB 160    // problems with less than 248 on first dungeon campaign into vid (if into logos aren't disabled)
#define MEMORY_LIBC_MB 128      // problems with less than 84 (dissapearing border sprites, etc)
#else
#define MEMORY_NEWLIB_MB 256
#define MEMORY_LIBC_MB 0
#endif

#define LOAD_ADDRESS 0x98000000

#define APP_PATH "ux0:app/HOMM00003/"
#define DATA_PATH "ux0:data/homm3hd"
#define SO_PATH DATA_PATH "/" "libhomm3.so"
#define CONFIG_FILE_PATH DATA_PATH "/" "config.txt"
#define TROPHIES_FILE DATA_PATH "/" "trophies.chk"

#endif
