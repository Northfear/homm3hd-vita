#ifndef __CONFIG_H__
#define __CONFIG_H__

#define GRAPHICS_API_VITAGL 0
#define GRAPHICS_API_PVR 1

// some memory has to be left for internal PVR stuff too
#define MEMORY_NEWLIB_MB 224    // problems with less than 248 on first dungeon campaign into vid (if into logos aren't disabled)
#define MEMORY_LIBC_MB 96       // problems with less than 84 (dissapearing border sprites, etc)

#define LOAD_ADDRESS 0x98000000

#define APP_PATH "ux0:app/HOMM00003/"
#define DATA_PATH "ux0:data/homm3hd"
#define SO_PATH DATA_PATH "/" "libhomm3.so"
#define CONFIG_FILE_PATH "ux0:data/homm3hd/config.txt"

#endif
