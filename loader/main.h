#ifndef __MAIN_H__
#define __MAIN_H__

#include "config.h"
#include "so_util.h"

#define VITA_WIDTH 960
#define VITA_HEIGHT 544

int kuKernelCpuUnrestrictedMemcpy(void *dst, const void *src, SceSize len);
bool ShouldProcessControllerOnTick();

extern so_module homm3_mod;

#endif
