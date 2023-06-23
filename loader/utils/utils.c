/*
 * utils.c
 *
 * Common helper utilities.
 *
 * Copyright (C) 2021 Rinnegatamante
 * Copyright (C) 2022 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "utils.h"

#include <stdio.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <SDL2/SDL_image.h>


// For check_kubridge
SceUID _vshKernelSearchModuleByName(const char *, int *);

int ret0(void) {
    return 0;
}

int ret1(void) {
    return 1;
}

void debugPrintf(char *text, ...) {
    va_list list;
    static char string[0x8000];

    va_start(list, text);
    vsprintf(string, text, list);
    va_end(list);

    sceClibPrintf("%s", string);
}

int check_module(char *module_name) {
    int search_unk[2];
    return _vshKernelSearchModuleByName(module_name, search_unk);
}

SDL_Texture* loadSDLTexture(char *path, SDL_Renderer *renderer)
{
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path);

	if(loadedSurface == NULL)
	{
		sceClibPrintf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
	}
	else
	{
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if(newTexture == NULL)
		{
			sceClibPrintf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
		}

		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}
