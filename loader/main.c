/* main.c -- World of Goo .so loader
 *
 * Copyright (C) 2021 Andy Nguyen
 * Copyright (C) 2022 Rinnegatamante
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <taihen.h>

#include <psp2/appmgr.h>
#include <psp2/power.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>

#include <wctype.h>
#include <locale.h>
#include <fcntl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <GLES2/gl2.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <theora/theoradec.h>
#include <zlib.h>
#include <stdbool.h>

#include "main.h"
#include "config.h"
#include "so_util.h"
#include "reimpl/pthr.h"
#include "reimpl/other.h"
#include "reimpl/memory.h"
#include "utils/sfp2hfp.h"
#include "utils/dialog.h"
#include "utils/utils.h"


int _newlib_heap_size_user = MEMORY_NEWLIB_MB * 1024 * 1024;
unsigned int sceLibcHeapSize = MEMORY_LIBC_MB * 1024 * 1024;
unsigned int sceUserMainThreadStackSize = 1 * 1024 * 1024;

so_module homm3_mod;
SDL_Window* window = NULL;
int language = 0;

int (* InputTextMessageHandler)(SDL_Event*);
int (* MouseMessageHandler)(SDL_Event*);
int (* TouchMessageHandler)(SDL_Event*);
unsigned int (* MultiTouchMessageHandler)(SDL_Event*);
unsigned int (* KeyboardMessageHandler)(SDL_Event*);


void loadConfig(void)
{
	char buffer[30];
	int value;

	FILE *config = fopen(CONFIG_FILE_PATH, "r");

	if (config) {
		while (EOF != fscanf(config, "%[^ ] %d\n", buffer, &value)) {
			if (strcmp("language", buffer) == 0) language = value;
		}
		fclose(config);
	}
}

char *SDL_AndroidGetExternalStoragePath()
{
	return DATA_PATH;
}

int SDL_Android_Init()
{
	return 1;
}

int getLanguage_fake()
{
	// 0 - en
	// 1 - fr
	// 2 - it
	// 3 - de
	// 4 - es
	// 5 - pl
	// 6 - ru
	return language;
}

SDL_Window* SDL_CreateWindow_hook(const char *title, int x, int y, int w, int h, Uint32 flags)
{
	// helps a bit with fast unit movemement by limiting the framerate
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

	MouseMessageHandler = (void *)so_symbol(&homm3_mod, "_Z19MouseMessageHandlerP9SDL_Event");
	InputTextMessageHandler = (void *)so_symbol(&homm3_mod, "_Z23InputTextMessageHandlerP9SDL_Event");
	KeyboardMessageHandler = (void *)so_symbol(&homm3_mod, "_Z22KeyboardMessageHandlerP9SDL_Event");
	TouchMessageHandler = (void *)so_symbol(&homm3_mod, "_Z19TouchMessageHandlerP9SDL_Event");
	MultiTouchMessageHandler = (void *)so_symbol(&homm3_mod, "_Z24MultiTouchMessageHandlerP9SDL_Event");

	window = SDL_CreateWindow(title, x, y, w, h, 0);
	return window;
}
 
int mobProcessSDLEvents(void *userdata, SDL_Event *event)
{
	switch(event->type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP: {
			KeyboardMessageHandler(event);
			return 0;
		}

		case SDL_CONTROLLERAXISMOTION: {
			return 0;
		}

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP: {
			return 0;
		}

		case SDL_TEXTINPUT: {
			InputTextMessageHandler(event);
			return 0;
		}

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION: {
			// ignore rear touchpad
			if (event->tfinger.touchId == 1 ) {
				return 0;
			}

			TouchMessageHandler(event);

			// for some weird reason cursor coordinates are always stuck at topleft corner. let's fix this manually
			SDL_Rect *dotemu_mouseDstRect = (SDL_Rect *)(homm3_mod.text_base + 0x002a3098);

			dotemu_mouseDstRect->x = event->tfinger.x * VITA_WIDTH - 14;
			dotemu_mouseDstRect->y = event->tfinger.y * VITA_HEIGHT - 13;

			return 1;
		}

		case SDL_MULTIGESTURE: {
			MultiTouchMessageHandler(event);
			return 1;
		}

		case SDL_QUIT: {
			*(uint8_t *)(homm3_mod.text_base + 0x0037ec7c) = 1;
			return 0;
		}

		case SDL_APP_WILLENTERBACKGROUND:
		case SDL_APP_DIDENTERFOREGROUND:
		case SDL_APP_DIDENTERBACKGROUND:
		case SDL_APP_WILLENTERFOREGROUND: {
			return 0;
		}

		case SDL_USEREVENT: {
			*(uint8_t *)(homm3_mod.text_base + 0x0037ec98) = 1;
			return 0;
		}

		default:
			break;
	}

	return 1;
}

// IME events aren't proceeded w/o a screen update, so we're just forcing it while on-screen KBD is active
bool dotemu_needsScreenUpdate()
{
	bool *dotemu_refreshScreen = (bool *)(homm3_mod.text_base + 0x002a3121);
	bool need_refresh = *dotemu_refreshScreen;

	if (need_refresh) {
		*dotemu_refreshScreen = false;
	}

	return need_refresh || SDL_IsScreenKeyboardShown(window);
}

void patch_game(void)
{
	hook_addr(so_symbol(&homm3_mod, "_ZN18AndroidEventLogger12logLastEventEv"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSystemManager11getLanguageEv"), (uintptr_t)&getLanguage_fake);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSystemManager14isCrappyDeviceEv"), (uintptr_t)&ret1);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSystemManager8isNexus9Ev"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSystemManager7openURLEPKc"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSocialManager11isConnectedEv"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSocialManager11isSignedOutEv"), (uintptr_t)&ret1);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSocialManager7connectEv"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSocialManager10disconnectEv"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSocialManager16showAchievementsEv"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSocialManager17unlockAchievementE14AchievementsIdf"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN20AndroidSystemManager6hideUIEv"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN7_JNIEnv14CallVoidMethodEP8_jobjectP10_jmethodIDz"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN7_JNIEnv17CallBooleanMethodEP8_jobjectP10_jmethodIDz"), (uintptr_t)&ret0);
	hook_addr(so_symbol(&homm3_mod, "_ZN7_JNIEnv13CallIntMethodEP8_jobjectP10_jmethodIDz"), (uintptr_t)&ret0);

	hook_addr(so_symbol(&homm3_mod, "_Z19mobProcessSDLEventsPvP9SDL_Event"), (uintptr_t)&mobProcessSDLEvents);
	hook_addr(so_symbol(&homm3_mod, "_Z24dotemu_needsScreenUpdatev"), (uintptr_t)&dotemu_needsScreenUpdate);

	// disable videos if libgpu_es4_kernel_ext plugin is not installed (crashing on campaign videos otherwise)
	if (check_module("libgpu_es4_kernel_ext") < 0)
	{
		hook_addr(so_symbol(&homm3_mod, "_Z9VideoOpeniiiiiibbb"), (uintptr_t)&ret1);	// vids in general
	}

	// disable logos vid. there's a higher chance of campaign vids bugging out with them enabled. why? I've no idea
	hook_addr(so_symbol(&homm3_mod, "_Z9VideoPlayiiiiib"), (uintptr_t)&ret1);			// logos intro
}

static int __stack_chk_guard_fake = 0x42424242;
static FILE __sF_fake[0x100][3];
static char *__ctype_ = (char *)&_ctype_;

const short *_tolower_tab_ = _C_tolower_;
const short *_toupper_tab_ = _C_toupper_;


static so_default_dynlib default_dynlib[] = {
	{ "__aeabi_atexit", (uintptr_t)&__aeabi_atexit },
	{ "__assert2", (uintptr_t)&__assert2 },
	{ "__cxa_atexit", (uintptr_t)&__cxa_atexit },
	{ "__cxa_finalize", (uintptr_t)&__cxa_finalize },
	{ "__gnu_Unwind_Find_exidx", (uintptr_t)&ret0 },
	{ "__sF", (uintptr_t)&__sF_fake },
	{ "__stack_chk_fail", (uintptr_t)&__stack_chk_fail },
	{ "__stack_chk_guard", (uintptr_t)&__stack_chk_guard_fake },
	{ "_ctype_", (uintptr_t)&__ctype_ },
	{ "_tolower_tab_", (uintptr_t)&_tolower_tab_ },
	{ "_toupper_tab_", (uintptr_t)&_toupper_tab_ },
	{ "abort", (uintptr_t)&abort },
	{ "access", (uintptr_t)&access },
	{ "alphasort", (uintptr_t)&alphasort },
	{ "atan", (uintptr_t)&atan_sfp },
	{ "atan2", (uintptr_t)&atan2_sfp },
	{ "atoi", (uintptr_t)&atoi },
	{ "ceil", (uintptr_t)&ceil_sfp },
	{ "close", (uintptr_t)&close },
	{ "compress2", (uintptr_t)&compress2 },
	{ "cosf", (uintptr_t)&cosf_sfp },
	{ "crc32", (uintptr_t)&crc32 },
	{ "dlclose", (uintptr_t)&ret1 },
	{ "dlopen", (uintptr_t)&ret1 },
	{ "dlsym", (uintptr_t)&ret1 },
	{ "exit", (uintptr_t)&exit },
	{ "fclose", (uintptr_t)&fclose },
	{ "fcntl", (uintptr_t)&fcntl },
	{ "fflush", (uintptr_t)&fflush },
	{ "fgetpos", (uintptr_t)&fgetpos },
	{ "fmod", (uintptr_t)&fmod_sfp },
	{ "fopen", (uintptr_t)&fopen },
	{ "fprintf", (uintptr_t)&fprintf },
	{ "fread", (uintptr_t)&fread },
	{ "free", (uintptr_t)&free },
	{ "fseek", (uintptr_t)&fseek },
	{ "fsetpos", (uintptr_t)&fsetpos },
	{ "fstat", (uintptr_t)&fstat },
	{ "ftell", (uintptr_t)&ftell },
	{ "fwrite", (uintptr_t)&fwrite },
	{ "getc", (uintptr_t)&getc },
	{ "glActiveTexture", (uintptr_t)&glActiveTexture },
	{ "glAttachShader", (uintptr_t)&glAttachShader },
	{ "glBindAttribLocation", (uintptr_t)&glBindAttribLocation },
	{ "glBindRenderbuffer", (uintptr_t)&glBindRenderbuffer },
	{ "glBindTexture", (uintptr_t)&glBindTexture },
	{ "glBlendFuncSeparate", (uintptr_t)&glBlendFuncSeparate },
	{ "glClear", (uintptr_t)&glClear },
	{ "glClearDepthf", (uintptr_t)&glClearDepthf_sfp },
	{ "glCompileShader", (uintptr_t)&glCompileShader },
	{ "glCompressedTexImage2D", (uintptr_t)&glCompressedTexImage2D },
	{ "glCreateProgram", (uintptr_t)&glCreateProgram },
	{ "glCreateShader", (uintptr_t)&glCreateShader },
	{ "glDeleteRenderbuffers", (uintptr_t)&glDeleteRenderbuffers },
	{ "glDeleteTextures", (uintptr_t)&glDeleteTextures },
	{ "glDepthFunc", (uintptr_t)&glDepthFunc },
	{ "glDepthMask", (uintptr_t)&glDepthMask },
	{ "glDepthRangef", (uintptr_t)&glDepthRangef_sfp },
	{ "glDisable", (uintptr_t)&glDisable },
	{ "glDisableVertexAttribArray", (uintptr_t)&glDisableVertexAttribArray },
	{ "glDrawArrays", (uintptr_t)&glDrawArrays },
	{ "glEnable", (uintptr_t)&glEnable },
	{ "glEnableVertexAttribArray", (uintptr_t)&glEnableVertexAttribArray },
	{ "glFramebufferRenderbuffer", (uintptr_t)&glFramebufferRenderbuffer },
	{ "glGenRenderbuffers", (uintptr_t)&glGenRenderbuffers },
	{ "glGenTextures", (uintptr_t)&glGenTextures },
	{ "glGetActiveUniform", (uintptr_t)&glGetActiveUniform },
	{ "glGetAttribLocation", (uintptr_t)&glGetAttribLocation },
	{ "glGetError", (uintptr_t)&glGetError },
	{ "glGetIntegerv", (uintptr_t)&glGetIntegerv },
	{ "glGetProgramInfoLog", (uintptr_t)&glGetProgramInfoLog },
	{ "glGetProgramiv", (uintptr_t)&glGetProgramiv },
	{ "glGetRenderbufferParameteriv", (uintptr_t)&glGetRenderbufferParameteriv },
	{ "glGetShaderInfoLog", (uintptr_t)&glGetShaderInfoLog },
	{ "glGetShaderiv", (uintptr_t)&glGetShaderiv },
	{ "glGetString", (uintptr_t)&glGetString },
	{ "glGetUniformLocation", (uintptr_t)&glGetUniformLocation },
	{ "glIsEnabled", (uintptr_t)&glIsEnabled },
	{ "glLinkProgram", (uintptr_t)&glLinkProgram },
	{ "glRenderbufferStorage", (uintptr_t)&glRenderbufferStorage },
	{ "glScissor", (uintptr_t)&glScissor },
	{ "glShaderSource", (uintptr_t)&glShaderSource },
	{ "glTexImage2D", (uintptr_t)&glTexImage2D },
	{ "glTexParameterf", (uintptr_t)&glTexParameterf_sfp },
	{ "glTexParameteri", (uintptr_t)&glTexParameteri },
	{ "glUniform1f", (uintptr_t)&glUniform1f_sfp },
	{ "glUniform1i", (uintptr_t)&glUniform1i },
	{ "glUniform2f", (uintptr_t)&glUniform2f_sfp },
	{ "glUniform3f", (uintptr_t)&glUniform3f_sfp },
	{ "glUniform4f", (uintptr_t)&glUniform4f_sfp },
	{ "glUseProgram", (uintptr_t)&glUseProgram },
	{ "glVertexAttribPointer", (uintptr_t)&glVertexAttribPointer },
	{ "gzclose", (uintptr_t)&gzclose },
	{ "gzopen", (uintptr_t)&gzopen },
	{ "gzread", (uintptr_t)&gzread },
	{ "gzwrite", (uintptr_t)&gzwrite },
	{ "IMG_Load", (uintptr_t)&IMG_Load },
	{ "isalpha", (uintptr_t)&isalpha },
	{ "iscntrl", (uintptr_t)&iscntrl },
	{ "islower", (uintptr_t)&islower },
	{ "isprint", (uintptr_t)&isprint },
	{ "ispunct", (uintptr_t)&ispunct },
	{ "isspace", (uintptr_t)&isspace },
	{ "isupper", (uintptr_t)&isupper },
	{ "iswalpha", (uintptr_t)&iswalpha },
	{ "iswcntrl", (uintptr_t)&iswcntrl },
	{ "iswdigit", (uintptr_t)&iswdigit },
	{ "iswlower", (uintptr_t)&iswlower },
	{ "iswprint", (uintptr_t)&iswprint },
	{ "iswpunct", (uintptr_t)&iswpunct },
	{ "iswspace", (uintptr_t)&iswspace },
	{ "iswupper", (uintptr_t)&iswupper },
	{ "iswxdigit", (uintptr_t)&iswxdigit },
	{ "isxdigit", (uintptr_t)&isxdigit },
	{ "localtime", (uintptr_t)&localtime },
	{ "lrand48", (uintptr_t)&lrand48 },
	{ "lseek", (uintptr_t)&lseek },
	{ "malloc", (uintptr_t)&malloc },
	{ "memcmp", (uintptr_t)&memcmp },
	{ "memcpy", (uintptr_t)&memcpy },
	{ "memmove", (uintptr_t)&memmove },
	{ "memset", (uintptr_t)&memset },
	{ "Mix_AllocateChannels", (uintptr_t)&Mix_AllocateChannels },
	{ "Mix_CloseAudio", (uintptr_t)&Mix_CloseAudio },
	{ "Mix_FadeInMusic", (uintptr_t)&Mix_FadeInMusic },
	{ "Mix_FadeOutMusic", (uintptr_t)&Mix_FadeOutMusic },
	{ "Mix_FreeChunk", (uintptr_t)&Mix_FreeChunk },
	{ "Mix_FreeMusic", (uintptr_t)&Mix_FreeMusic },
	{ "Mix_GroupAvailable", (uintptr_t)&Mix_GroupAvailable },
	{ "Mix_GroupChannel", (uintptr_t)&Mix_GroupChannel },
	{ "Mix_GroupChannels", (uintptr_t)&Mix_GroupChannels },
	{ "Mix_GroupOldest", (uintptr_t)&Mix_GroupOldest },
	{ "Mix_HaltChannel", (uintptr_t)&Mix_HaltChannel },
	{ "Mix_HaltMusic", (uintptr_t)&Mix_HaltMusic },
	{ "Mix_HookMusic", (uintptr_t)&Mix_HookMusic },
	{ "Mix_LoadMUS", (uintptr_t)&Mix_LoadMUS },
	{ "Mix_LoadWAV_RW", (uintptr_t)&Mix_LoadWAV_RW },
	{ "Mix_OpenAudio", (uintptr_t)&Mix_OpenAudio },
	{ "Mix_Pause", (uintptr_t)&Mix_Pause },
	{ "Mix_PausedMusic", (uintptr_t)&Mix_PausedMusic },
	{ "Mix_PauseMusic", (uintptr_t)&Mix_PauseMusic },
	{ "Mix_PlayChannelTimed", (uintptr_t)&Mix_PlayChannelTimed },
	{ "Mix_Playing", (uintptr_t)&Mix_Playing },
	{ "Mix_PlayingMusic", (uintptr_t)&Mix_PlayingMusic },
	{ "Mix_QuerySpec", (uintptr_t)&Mix_QuerySpec },
	{ "Mix_ReserveChannels", (uintptr_t)&Mix_ReserveChannels },
	{ "Mix_Resume", (uintptr_t)&Mix_Resume },
	{ "Mix_ResumeMusic", (uintptr_t)&Mix_ResumeMusic },
	{ "Mix_Volume", (uintptr_t)&Mix_Volume },
	{ "Mix_VolumeMusic", (uintptr_t)&Mix_VolumeMusic },
	{ "mkdir", (uintptr_t)&mkdir },
	{ "mktime", (uintptr_t)&mktime },
	{ "mmap", (uintptr_t)&mmap },
	{ "munmap", (uintptr_t)&munmap },
	{ "ogg_page_bos", (uintptr_t)&ogg_page_bos },
	{ "ogg_page_serialno", (uintptr_t)&ogg_page_serialno },
	{ "ogg_stream_clear", (uintptr_t)&ogg_stream_clear },
	{ "ogg_stream_init", (uintptr_t)&ogg_stream_init },
	{ "ogg_stream_packetout", (uintptr_t)&ogg_stream_packetout },
	{ "ogg_stream_pagein", (uintptr_t)&ogg_stream_pagein },
	{ "ogg_sync_buffer", (uintptr_t)&ogg_sync_buffer },
	{ "ogg_sync_clear", (uintptr_t)&ogg_sync_clear },
	{ "ogg_sync_init", (uintptr_t)&ogg_sync_init },
	{ "ogg_sync_pageout", (uintptr_t)&ogg_sync_pageout },
	{ "ogg_sync_wrote", (uintptr_t)&ogg_sync_wrote },
	{ "open", (uintptr_t)&open },
	{ "perror", (uintptr_t)&perror },
	{ "pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast_soloader },
	{ "pthread_cond_wait", (uintptr_t)&pthread_cond_wait_soloader },
	{ "pthread_create", (uintptr_t)&pthread_create_soloader },
	{ "pthread_getspecific", (uintptr_t)&pthread_getspecific },
	{ "pthread_join", (uintptr_t)&pthread_join_soloader },
	{ "pthread_key_create", (uintptr_t)&pthread_key_create },
	{ "pthread_key_delete", (uintptr_t)&pthread_key_delete },
	{ "pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy_soloader },
	{ "pthread_mutex_init", (uintptr_t)&pthread_mutex_init_soloader },
	{ "pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock_soloader },
	{ "pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock_soloader },
	{ "pthread_setspecific", (uintptr_t)&pthread_setspecific },
	{ "putc", (uintptr_t)&putc },
	{ "puts", (uintptr_t)&puts },
	{ "qsort", (uintptr_t)&qsort },
	{ "raise", (uintptr_t)&raise },
	{ "read", (uintptr_t)&read },
	{ "realloc", (uintptr_t)&realloc },
	{ "rename", (uintptr_t)&rename },
	{ "rewind", (uintptr_t)&rewind },
	{ "scandir", (uintptr_t)&scandir_hook },
	{ "SDL_AddTimer", (uintptr_t)&SDL_AddTimer },
	{ "SDL_Android_Init", (uintptr_t)&SDL_Android_Init },
	{ "SDL_AndroidGetExternalStoragePath", (uintptr_t)&SDL_AndroidGetExternalStoragePath },
	{ "SDL_CondSignal", (uintptr_t)&SDL_CondSignal },
	{ "SDL_CondWait", (uintptr_t)&SDL_CondWait },
	{ "SDL_ConvertSurfaceFormat", (uintptr_t)&SDL_ConvertSurfaceFormat },
	{ "SDL_CreateCond", (uintptr_t)&SDL_CreateCond },
	{ "SDL_CreateMutex", (uintptr_t)&SDL_CreateMutex },
	{ "SDL_CreateRenderer", (uintptr_t)&SDL_CreateRenderer },
	{ "SDL_CreateRGBSurface", (uintptr_t)&SDL_CreateRGBSurface },
	{ "SDL_CreateTexture", (uintptr_t)&SDL_CreateTexture },
	{ "SDL_CreateTextureFromSurface", (uintptr_t)&SDL_CreateTextureFromSurface },
	{ "SDL_CreateThread", (uintptr_t)&SDL_CreateThread },
	{ "SDL_CreateWindow", (uintptr_t)&SDL_CreateWindow_hook },
	{ "SDL_Delay", (uintptr_t)&SDL_Delay },
	{ "SDL_DestroyMutex", (uintptr_t)&SDL_DestroyMutex },
	{ "SDL_DestroyRenderer", (uintptr_t)&SDL_DestroyRenderer },
	{ "SDL_DestroyTexture", (uintptr_t)&SDL_DestroyTexture },
	{ "SDL_DestroyWindow", (uintptr_t)&SDL_DestroyWindow },
	{ "SDL_FillRect", (uintptr_t)&SDL_FillRect },
	{ "SDL_FreeSurface", (uintptr_t)&SDL_FreeSurface },
	{ "SDL_GetCurrentDisplayMode", (uintptr_t)&SDL_GetCurrentDisplayMode },
	{ "SDL_GetError", (uintptr_t)&SDL_GetError },
	{ "SDL_GetModState", (uintptr_t)&SDL_GetModState },
	{ "SDL_GetMouseState", (uintptr_t)&SDL_GetMouseState },
	{ "SDL_GetRendererInfo", (uintptr_t)&SDL_GetRendererInfo },
	{ "SDL_GetTextureBlendMode", (uintptr_t)&SDL_GetTextureBlendMode },
	{ "SDL_GetTextureColorMod", (uintptr_t)&SDL_GetTextureColorMod },
	{ "SDL_GetTicks", (uintptr_t)&SDL_GetTicks },
	{ "SDL_GL_BindTexture", (uintptr_t)&SDL_GL_BindTexture },
	{ "SDL_GL_GetCurrentContext", (uintptr_t)&SDL_GL_GetCurrentContext },
	{ "SDL_GL_MakeCurrent", (uintptr_t)&SDL_GL_MakeCurrent },
	{ "SDL_GL_SetAttribute", (uintptr_t)&SDL_GL_SetAttribute },
	{ "SDL_Init", (uintptr_t)&SDL_Init },
	{ "SDL_InitSubSystem", (uintptr_t)&SDL_InitSubSystem },
	{ "SDL_IntersectRect", (uintptr_t)&SDL_IntersectRect },
	{ "SDL_LockMutex", (uintptr_t)&SDL_LockMutex },
	{ "SDL_LockSurface", (uintptr_t)&SDL_LockSurface },
	{ "SDL_Log", (uintptr_t)&SDL_Log },
	{ "SDL_LogError", (uintptr_t)&SDL_LogError },
	{ "SDL_LogSetPriority", (uintptr_t)&SDL_LogSetPriority },
	{ "SDL_MapRGB", (uintptr_t)&SDL_MapRGB },
	{ "SDL_MinimizeWindow", (uintptr_t)&SDL_MinimizeWindow },
	{ "SDL_PeepEvents", (uintptr_t)&SDL_PeepEvents },
	{ "SDL_PumpEvents", (uintptr_t)&SDL_PumpEvents },
	{ "SDL_QueryTexture", (uintptr_t)&SDL_QueryTexture },
	{ "SDL_Quit", (uintptr_t)&SDL_Quit },
	{ "SDL_RemoveTimer", (uintptr_t)&SDL_RemoveTimer },
	{ "SDL_RenderClear", (uintptr_t)&SDL_RenderClear },
	{ "SDL_RenderCopy", (uintptr_t)&SDL_RenderCopy },
	{ "SDL_RenderFillRect", (uintptr_t)&SDL_RenderFillRect },
	{ "SDL_RenderPresent", (uintptr_t)&SDL_RenderPresent },
	{ "SDL_RWFromFile", (uintptr_t)&SDL_RWFromFile },
	{ "SDL_RWFromMem", (uintptr_t)&SDL_RWFromMem },
	{ "SDL_SetColorKey", (uintptr_t)&SDL_SetColorKey },
	{ "SDL_SetEventFilter", (uintptr_t)&SDL_SetEventFilter },
	{ "SDL_SetHint", (uintptr_t)&SDL_SetHint },
	{ "SDL_SetMainReady_REAL", (uintptr_t)&SDL_SetMainReady },
	{ "SDL_SetRenderDrawBlendMode", (uintptr_t)&SDL_SetRenderDrawBlendMode },
	{ "SDL_SetRenderDrawColor", (uintptr_t)&SDL_SetRenderDrawColor },
	{ "SDL_SetRenderTarget", (uintptr_t)&SDL_SetRenderTarget },
	{ "SDL_SetTextureBlendMode", (uintptr_t)&SDL_SetTextureBlendMode },
	{ "SDL_SetTextureColorMod", (uintptr_t)&SDL_SetTextureColorMod },
	{ "SDL_ShowCursor", (uintptr_t)&SDL_ShowCursor },
	{ "SDL_ShowSimpleMessageBox", (uintptr_t)&SDL_ShowSimpleMessageBox },
	{ "SDL_StartTextInput", (uintptr_t)&SDL_StartTextInput },
	{ "SDL_StopTextInput", (uintptr_t)&SDL_StopTextInput },
	{ "SDL_strdup_REAL", (uintptr_t)&SDL_strdup },
	{ "SDL_UnlockMutex", (uintptr_t)&SDL_UnlockMutex },
	{ "SDL_UnlockSurface", (uintptr_t)&SDL_UnlockSurface },
	{ "SDL_UpdateTexture", (uintptr_t)&SDL_UpdateTexture },
	{ "SDL_UpperBlit", (uintptr_t)&SDL_UpperBlit },
	{ "SDL_WaitThread", (uintptr_t)&SDL_WaitThread },
	{ "setlocale", (uintptr_t)&setlocale },
	{ "setvbuf", (uintptr_t)&setvbuf },
	{ "sinf", (uintptr_t)&sinf_sfp },
	{ "snprintf", (uintptr_t)&snprintf },
	{ "sprintf", (uintptr_t)&sprintf },
	{ "srand48", (uintptr_t)&srand48 },
	{ "stat", (uintptr_t)&stat_hook },
	{ "strcasecmp", (uintptr_t)&strcasecmp },
	{ "strcat", (uintptr_t)&strcat },
	{ "strchr", (uintptr_t)&strchr },
	{ "strcmp", (uintptr_t)&strcmp },
	{ "strcpy", (uintptr_t)&strcpy },
	{ "strlen", (uintptr_t)&strlen },
	{ "strncasecmp", (uintptr_t)&strncasecmp },
	{ "strncat", (uintptr_t)&strncat },
	{ "strncmp", (uintptr_t)&strncmp },
	{ "strncpy", (uintptr_t)&strncpy },
	{ "strpbrk", (uintptr_t)&strpbrk },
	{ "strrchr", (uintptr_t)&strrchr },
	{ "strstr", (uintptr_t)&strstr },
	{ "strtod", (uintptr_t)&strtod },
	{ "strtok", (uintptr_t)&strtok },
	{ "sysconf", (uintptr_t)&sysconf },
	{ "th_comment_clear", (uintptr_t)&th_comment_clear },
	{ "th_comment_init", (uintptr_t)&th_comment_init },
	{ "th_decode_alloc", (uintptr_t)&th_decode_alloc },
	{ "th_decode_ctl", (uintptr_t)&th_decode_ctl },
	{ "th_decode_free", (uintptr_t)&th_decode_free },
	{ "th_decode_headerin", (uintptr_t)&th_decode_headerin },
	{ "th_decode_packetin", (uintptr_t)&th_decode_packetin },
	{ "th_decode_ycbcr_out", (uintptr_t)&th_decode_ycbcr_out },
	{ "th_granule_time", (uintptr_t)&th_granule_time_sfp },
	{ "th_info_clear", (uintptr_t)&th_info_clear },
	{ "th_info_init", (uintptr_t)&th_info_init },
	{ "th_setup_free", (uintptr_t)&th_setup_free },
	{ "time", (uintptr_t)&time },
	{ "tolower", (uintptr_t)&tolower },
	{ "toupper", (uintptr_t)&toupper },
	{ "towlower", (uintptr_t)&towlower },
	{ "towupper", (uintptr_t)&towupper },
	{ "uncompress", (uintptr_t)&uncompress },
	{ "ungetc", (uintptr_t)&ungetc },
	{ "unlink", (uintptr_t)&unlink },
	{ "usleep", (uintptr_t)&usleep },
	{ "vorbis_block_clear", (uintptr_t)&vorbis_block_clear },
	{ "vorbis_block_init", (uintptr_t)&vorbis_block_init },
	{ "vorbis_comment_clear", (uintptr_t)&vorbis_comment_clear },
	{ "vorbis_comment_init", (uintptr_t)&vorbis_comment_init },
	{ "vorbis_dsp_clear", (uintptr_t)&vorbis_dsp_clear },
	{ "vorbis_granule_time", (uintptr_t)&vorbis_granule_time_sfp },
	{ "vorbis_info_clear", (uintptr_t)&vorbis_info_clear },
	{ "vorbis_info_init", (uintptr_t)&vorbis_info_init },
	{ "vorbis_synthesis", (uintptr_t)&vorbis_synthesis },
	{ "vorbis_synthesis_blockin", (uintptr_t)&vorbis_synthesis_blockin },
	{ "vorbis_synthesis_headerin", (uintptr_t)&vorbis_synthesis_headerin },
	{ "vorbis_synthesis_init", (uintptr_t)&vorbis_synthesis_init },
	{ "vorbis_synthesis_pcmout", (uintptr_t)&vorbis_synthesis_pcmout },
	{ "vorbis_synthesis_read", (uintptr_t)&vorbis_synthesis_read },
	{ "vsprintf", (uintptr_t)&vsprintf },
	{ "wcscmp", (uintptr_t)&wcscmp },
	{ "wcslen", (uintptr_t)&wcslen },
	{ "wcsncpy", (uintptr_t)&wcsncpy },
	{ "wmemcpy", (uintptr_t)&wmemcpy },
	{ "wmemmove", (uintptr_t)&wmemmove },
	{ "wmemset", (uintptr_t)&wmemset },
	{ "write", (uintptr_t)&write },
};


int main(int argc, char *argv[])
{
	sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
	SceAppUtilAppEventParam eventParam;
	sceClibMemset(&eventParam, 0, sizeof(SceAppUtilAppEventParam));
	sceAppUtilReceiveAppEvent(&eventParam);
	if (eventParam.type == 0x05)
	{
		char buffer[2048];
		sceAppUtilAppEventParseLiveArea(&eventParam, buffer);
		if (strstr(buffer, "-config"))
		{
			sceAppMgrLoadExec("app0:/config.bin", NULL, NULL);
		}
	}

	init_memory();

	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);

	if (check_module("kubridge") < 0)
	{
		fatal_error("Error kubridge.skprx is not installed.");
	}

	if (check_module("libgpu_es4_kernel_ext") < 0)
	{
		SceUID mod_id = -1;
		mod_id = taiLoadStartKernelModule(APP_PATH "module/libgpu_es4_kernel_ext.skprx", 0, NULL, 0);
		if (mod_id < 0) {
			show_message("WARNING!\nlibgpu_es4_kernel_ext.skprx plugin is not installed!\nVideo playback is disabled. Game WILL have some visual bugs without it.");
		}
	}

	if (so_file_load(&homm3_mod, SO_PATH, LOAD_ADDRESS) < 0)
	{
		fatal_error("Error could not load %s.", SO_PATH);
	}

	so_relocate(&homm3_mod);
	so_resolve(&homm3_mod, default_dynlib, sizeof(default_dynlib), 0);

	patch_game();

	so_flush_caches(&homm3_mod);
	so_initialize(&homm3_mod);

	loadConfig();

	// make sure that pref folder exists (game saves will fail w/o it)
	mkdir(DATA_PATH "/pref", 0777);

	int (* SDL_main)(void) = (void *)so_symbol(&homm3_mod, "SDL_main");
	SDL_main();

	term_memory();

	return 0;
}
