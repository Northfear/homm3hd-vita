/*
 * utils.h
 *
 * Common helper utilities.
 *
 * Copyright (C) 2021 Rinnegatamante
 * Copyright (C) 2022 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef __UTILS_H
#define __UTILS_H

#include <SDL2/SDL.h>

int ret0(void);
int ret1(void);
void debugPrintf(char *text, ...);
int check_module(char *module_name);
int file_exists(const char *path);
SDL_Texture* loadSDLTexture(char *path, SDL_Renderer *renderer);

#endif // __UTILS_H
