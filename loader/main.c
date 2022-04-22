/* main.c -- World of Goo .so loader
 *
 * Copyright (C) 2021 Andy Nguyen
 * Copyright (C) 2022 Rinnegatamante
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2/kernel/clib.h>
#include <psp2/power.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h> 
#include <kubridge.h>

#include <dirent.h>
#include <ctype.h>
#include <wctype.h>
#include <locale.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <vitaGL.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <theora/theoradec.h>
#include <pthread.h>
#include <zlib.h>
#include <stdbool.h>

#include "main.h"
#include "config.h"
#include "dialog.h"
#include "so_util.h"

#define printf sceClibPrintf


int _newlib_heap_size_user = 224 * 1024 * 1024;

so_module homm3_mod;

void *__wrap_memcpy(void *dest, const void *src, size_t n) {
  return sceClibMemcpy(dest, src, n);
}

void *__wrap_memmove(void *dest, const void *src, size_t n) {
  return sceClibMemmove(dest, src, n);
}

void *__wrap_memset(void *s, int c, size_t n) {
  return sceClibMemset(s, c, n);
}

int debugPrintf(char *text, ...) {
#ifdef DEBUG
  va_list list;
  static char string[0x8000];

  va_start(list, text);
  vsprintf(string, text, list);
  va_end(list);

  SceUID fd = sceIoOpen("ux0:data/homm3_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
  if (fd >= 0) {
    sceIoWrite(fd, string, strlen(string));
    sceIoClose(fd);
  }
#endif
  return 0;
}

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
#ifdef DEBUG
  va_list list;
  static char string[0x8000];

  va_start(list, fmt);
  vsprintf(string, fmt, list);
  va_end(list);

  printf("[LOG] %s: %s\n", tag, string);
#endif
  return 0;
}

int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list list) {
#ifdef DEBUG
  static char string[0x8000];

  vsprintf(string, fmt, list);
  va_end(list);

  printf("[LOGV] %s: %s\n", tag, string);
#endif
  return 0;
}

int ret0(void) {
  return 0;
}

int ret1(void) {
  return 1;
}

void __assert2(const char *file, int line, const char *func, const char *expr) {
  printf("assertion failed:\n%s:%d (%s): %s\n", file, line, func, expr);
}

#define CTYPE_NUM_CHARS       256

static const short _C_toupper_[] = {
	-1,
	0x00,	0x01,	0x02,	0x03,	0x04,	0x05,	0x06,	0x07,
	0x08,	0x09,	0x0a,	0x0b,	0x0c,	0x0d,	0x0e,	0x0f,
	0x10,	0x11,	0x12,	0x13,	0x14,	0x15,	0x16,	0x17,
	0x18,	0x19,	0x1a,	0x1b,	0x1c,	0x1d,	0x1e,	0x1f,
	0x20,	0x21,	0x22,	0x23,	0x24,	0x25,	0x26,	0x27,
	0x28,	0x29,	0x2a,	0x2b,	0x2c,	0x2d,	0x2e,	0x2f,
	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,
	0x38,	0x39,	0x3a,	0x3b,	0x3c,	0x3d,	0x3e,	0x3f,
	0x40,	0x41,	0x42,	0x43,	0x44,	0x45,	0x46,	0x47,
	0x48,	0x49,	0x4a,	0x4b,	0x4c,	0x4d,	0x4e,	0x4f,
	0x50,	0x51,	0x52,	0x53,	0x54,	0x55,	0x56,	0x57,
	0x58,	0x59,	0x5a,	0x5b,	0x5c,	0x5d,	0x5e,	0x5f,
	0x60,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	0x7b,	0x7c,	0x7d,	0x7e,	0x7f,
	0x80,	0x81,	0x82,	0x83,	0x84,	0x85,	0x86,	0x87,
	0x88,	0x89,	0x8a,	0x8b,	0x8c,	0x8d,	0x8e,	0x8f,
	0x90,	0x91,	0x92,	0x93,	0x94,	0x95,	0x96,	0x97,
	0x98,	0x99,	0x9a,	0x9b,	0x9c,	0x9d,	0x9e,	0x9f,
	0xa0,	0xa1,	0xa2,	0xa3,	0xa4,	0xa5,	0xa6,	0xa7,
	0xa8,	0xa9,	0xaa,	0xab,	0xac,	0xad,	0xae,	0xaf,
	0xb0,	0xb1,	0xb2,	0xb3,	0xb4,	0xb5,	0xb6,	0xb7,
	0xb8,	0xb9,	0xba,	0xbb,	0xbc,	0xbd,	0xbe,	0xbf,
	0xc0,	0xc1,	0xc2,	0xc3,	0xc4,	0xc5,	0xc6,	0xc7,
	0xc8,	0xc9,	0xca,	0xcb,	0xcc,	0xcd,	0xce,	0xcf,
	0xd0,	0xd1,	0xd2,	0xd3,	0xd4,	0xd5,	0xd6,	0xd7,
	0xd8,	0xd9,	0xda,	0xdb,	0xdc,	0xdd,	0xde,	0xdf,
	0xe0,	0xe1,	0xe2,	0xe3,	0xe4,	0xe5,	0xe6,	0xe7,
	0xe8,	0xe9,	0xea,	0xeb,	0xec,	0xed,	0xee,	0xef,
	0xf0,	0xf1,	0xf2,	0xf3,	0xf4,	0xf5,	0xf6,	0xf7,
	0xf8,	0xf9,	0xfa,	0xfb,	0xfc,	0xfd,	0xfe,	0xff
};

static const short _C_tolower_[] = {
  -1,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 'a',  'b',  'c',  'd',  'e',  'f',  'g',
  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
  'x',  'y',  'z',  0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
  0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
  0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
  0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
  0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

extern void *__aeabi_atexit;
extern void *__cxa_atexit;
extern void *__cxa_finalize;
extern void *__stack_chk_fail;

static int __stack_chk_guard_fake = 0x42424242;
static FILE __sF_fake[0x100][3];
static char *__ctype_ = (char *)&_ctype_;

const short *_tolower_tab_ = _C_tolower_;
const short *_toupper_tab_ = _C_toupper_;

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	(void)prot;
	(void)flags;
	if (addr != NULL) {
		if ((addr = malloc(len))) {
			lseek(fd, offset, SEEK_SET);
			read(fd, addr, len);
		}

		if (!addr) {
                	return NULL;
        	}

		return addr;
	}
	else {
		void *data = malloc(len);
		if (data != NULL) {
                        lseek(fd, offset, SEEK_SET);
                        read(fd, data, len);
		}

		if (!data) {
			return NULL;
		}

		return data;
	}
}

int munmap(void *map, size_t length)
{
	(void)length;
	if (map!=NULL)
		free(map);

	return 0;
}

int pthread_mutex_init_fake(pthread_mutex_t **uid, const pthread_mutexattr_t *mutexattr) {
	pthread_mutex_t *m = calloc(1, sizeof(pthread_mutex_t));
	if (!m)
		return -1;

	const int recursive = (mutexattr && *(const int *)mutexattr == 1);
	*m = recursive ? PTHREAD_RECURSIVE_MUTEX_INITIALIZER : PTHREAD_MUTEX_INITIALIZER;

	int ret = pthread_mutex_init(m, mutexattr);
	if (ret < 0) {
		free(m);
		return -1;
	}

	*uid = m;

	return 0;
}

int pthread_mutex_destroy_fake(pthread_mutex_t **uid) {
	if (uid && *uid && (uintptr_t)*uid > 0x8000) {
		pthread_mutex_destroy(*uid);
		free(*uid);
		*uid = NULL;
	}
	return 0;
}

int pthread_mutex_lock_fake(pthread_mutex_t **uid) {
	int ret = 0;
	if (!*uid) {
		ret = pthread_mutex_init_fake(uid, NULL);
	} else if ((uintptr_t)*uid == 0x4000) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		ret = pthread_mutex_init_fake(uid, &attr);
		pthread_mutexattr_destroy(&attr);
	} else if ((uintptr_t)*uid == 0x8000) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		ret = pthread_mutex_init_fake(uid, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	if (ret < 0)
		return ret;
	return pthread_mutex_lock(*uid);
}

int pthread_mutex_unlock_fake(pthread_mutex_t **uid) {
	int ret = 0;
	if (!*uid) {
		ret = pthread_mutex_init_fake(uid, NULL);
	} else if ((uintptr_t)*uid == 0x4000) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		ret = pthread_mutex_init_fake(uid, &attr);
		pthread_mutexattr_destroy(&attr);
	} else if ((uintptr_t)*uid == 0x8000) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		ret = pthread_mutex_init_fake(uid, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	if (ret < 0)
		return ret;
	return pthread_mutex_unlock(*uid);
}

int pthread_cond_init_fake(pthread_cond_t **cnd, const int *condattr) {
	pthread_cond_t *c = calloc(1, sizeof(pthread_cond_t));
	if (!c)
		return -1;

	*c = PTHREAD_COND_INITIALIZER;

	int ret = pthread_cond_init(c, NULL);
	if (ret < 0) {
		free(c);
		return -1;
	}

	*cnd = c;

	return 0;
}

int pthread_cond_broadcast_fake(pthread_cond_t **cnd) {
	if (!*cnd) {
		if (pthread_cond_init_fake(cnd, NULL) < 0)
			return -1;
	}
	return pthread_cond_broadcast(*cnd);
}

int pthread_cond_signal_fake(pthread_cond_t **cnd) {
	if (!*cnd) {
		if (pthread_cond_init_fake(cnd, NULL) < 0)
			return -1;
	}
	return pthread_cond_signal(*cnd);
}

int pthread_cond_destroy_fake(pthread_cond_t **cnd) {
	if (cnd && *cnd) {
		pthread_cond_destroy(*cnd);
		free(*cnd);
		*cnd = NULL;
	}
	return 0;
}

int pthread_cond_wait_fake(pthread_cond_t **cnd, pthread_mutex_t **mtx) {
	if (!*cnd) {
		if (pthread_cond_init_fake(cnd, NULL) < 0)
			return -1;
	}
	return pthread_cond_wait(*cnd, *mtx);
}

int pthread_cond_timedwait_fake(pthread_cond_t **cnd, pthread_mutex_t **mtx, const struct timespec *t) {
	if (!*cnd) {
		if (pthread_cond_init_fake(cnd, NULL) < 0)
			return -1;
	}
	return pthread_cond_timedwait(*cnd, *mtx, t);
}

int pthread_create_fake(pthread_t *thread, const void *unused, void *entry, void *arg) {
	return pthread_create(thread, NULL, entry, arg);
}

int pthread_once_fake(volatile int *once_control, void (*init_routine)(void)) {
	if (!once_control || !init_routine)
		return -1;
	if (__sync_lock_test_and_set(once_control, 1) == 0)
		(*init_routine)();
	return 0;
}

/*
int scandir(const char *restrict dirp,
            struct dirent ***restrict namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **,
                          const struct dirent **))
{
  return 0;
}

int alphasort(const struct dirent **a, const struct dirent **b)
{
  return 0;
}
*/

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
  // fake GL_RENDERBUFFER_DEPTH_SIZE (?) to avoid assert
  sceClibPrintf("glGetRenderbufferParameteriv   target: %d   pname: %d\n", target, pname);
  *params = 1;
}

char *SDL_AndroidGetExternalStoragePath()
{
  return DATA_PATH;
}

int SDL_Android_Init()
{
  return 1;
}

int SDL_Init_fake(Uint32 flags)
{
  SDL_SetHint("SDL_HINT_RENDER_BATCHING", "0");
  return SDL_Init(flags);
}

long sysconf_fake(int name)
{
  switch(name)
  {
    case _SC_THREAD_STACK_MIN:
      return PTHREAD_STACK_MIN;
    default:
      return -1;
  }
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
  return 0;
}

int stat_hook(const char *pathname, void *statbuf) {
  struct stat st;
  int res = stat(pathname, &st);
  if (res == 0)
    *(int *)(statbuf + 0x50) = st.st_mtime;
  return res;
}

void dotemu_hideCursor_fake(bool param_1)
{
  *(uint8_t *)(homm3_mod.text_base + 0x002a33d9) = 1;
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

  // trying to make cursor always visible
  hook_addr(so_symbol(&homm3_mod, "_Z17dotemu_hideCursorb"), (uintptr_t)&dotemu_hideCursor_fake);

  // failed allocations/crashes during video playback. disable it for now
  //hook_addr(so_symbol(&homm3_mod, "_Z17ShowBorderedMoviei"), (uintptr_t)&ret1);
  //hook_addr(so_symbol(&homm3_mod, "_Z9VideoPlayiiiiib"), (uintptr_t)&ret1);
  hook_addr(so_symbol(&homm3_mod, "_Z9VideoOpeniiiiiibbb"), (uintptr_t)&ret1);
}

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
  { "atan", (uintptr_t)&atan },
  { "atan2", (uintptr_t)&atan2 },
  { "atoi", (uintptr_t)&atoi },
  { "ceil", (uintptr_t)&ceil },
  { "close", (uintptr_t)&close },
  { "compress2", (uintptr_t)&compress2 },
  { "cosf", (uintptr_t)&cosf },
  { "crc32", (uintptr_t)&crc32 },
  { "dlclose", (uintptr_t)&ret1 },
  { "dlopen", (uintptr_t)&ret1 },
  { "dlsym", (uintptr_t)&ret1 },
  { "exit", (uintptr_t)&exit },
  { "fclose", (uintptr_t)&fclose },
  { "fcntl", (uintptr_t)&fcntl },
  { "fflush", (uintptr_t)&fflush },
  { "fgetpos", (uintptr_t)&fgetpos },
  { "fmod", (uintptr_t)&fmod },
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
  { "glBindRenderbuffer", (uintptr_t)&ret0 },
  { "glBindTexture", (uintptr_t)&glBindTexture },
  { "glBlendFuncSeparate", (uintptr_t)&glBlendFuncSeparate },
  { "glClear", (uintptr_t)&glClear },
  { "glClearDepthf", (uintptr_t)&glClearDepthf },
  { "glCompileShader", (uintptr_t)&glCompileShader },
  { "glCompressedTexImage2D", (uintptr_t)&glCompressedTexImage2D },
  { "glCreateProgram", (uintptr_t)&glCreateProgram },
  { "glCreateShader", (uintptr_t)&glCreateShader },
  { "glDeleteRenderbuffers", (uintptr_t)&ret0 },
  { "glDeleteTextures", (uintptr_t)&glDeleteTextures },
  { "glDepthFunc", (uintptr_t)&glDepthFunc },
  { "glDepthMask", (uintptr_t)&glDepthMask },
  { "glDepthRangef", (uintptr_t)&glDepthRangef },
  { "glDisable", (uintptr_t)&glDisable },
  { "glDisableVertexAttribArray", (uintptr_t)&glDisableVertexAttribArray },
  { "glDrawArrays", (uintptr_t)&glDrawArrays },
  { "glEnable", (uintptr_t)&glEnable },
  { "glEnableVertexAttribArray", (uintptr_t)&glEnableVertexAttribArray },
  { "glFramebufferRenderbuffer", (uintptr_t)&ret0 },
  { "glGenRenderbuffers", (uintptr_t)&ret0 },
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
  { "glRenderbufferStorage", (uintptr_t)&ret0 },
  { "glScissor", (uintptr_t)&glScissor },
  { "glShaderSource", (uintptr_t)&glShaderSource },
  { "glTexImage2D", (uintptr_t)&glTexImage2D },
  { "glTexParameterf", (uintptr_t)&glTexParameterf },
  { "glTexParameteri", (uintptr_t)&glTexParameteri },
  { "glUniform1f", (uintptr_t)&glUniform1f },
  { "glUniform1i", (uintptr_t)&glUniform1i },
  { "glUniform2f", (uintptr_t)&glUniform2f },
  { "glUniform3f", (uintptr_t)&glUniform3f },
  { "glUniform4f", (uintptr_t)&glUniform4f },
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
  { "pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast_fake },
  { "pthread_cond_wait", (uintptr_t)&pthread_cond_wait_fake },
  { "pthread_create", (uintptr_t)&pthread_create_fake },
  { "pthread_getspecific", (uintptr_t)&pthread_getspecific },
  { "pthread_join", (uintptr_t)&pthread_join },
  { "pthread_key_create", (uintptr_t)&pthread_key_create },
  { "pthread_key_delete", (uintptr_t)&pthread_key_delete },
  { "pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy_fake },
  { "pthread_mutex_init", (uintptr_t)&pthread_mutex_init_fake },
  { "pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock_fake },
  { "pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock_fake },
  { "pthread_setspecific", (uintptr_t)&pthread_setspecific },
  { "putc", (uintptr_t)&putc },
  { "puts", (uintptr_t)&puts },
  { "qsort", (uintptr_t)&qsort },
  { "raise", (uintptr_t)&raise },
  { "read", (uintptr_t)&read },
  { "realloc", (uintptr_t)&realloc },
  { "rename", (uintptr_t)&rename },
  { "rewind", (uintptr_t)&rewind },
  { "scandir", (uintptr_t)&scandir },
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
  { "SDL_CreateWindow", (uintptr_t)&SDL_CreateWindow },
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
  { "SDL_Init", (uintptr_t)&SDL_Init_fake },
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
  { "sinf", (uintptr_t)&sinf },
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
  { "sysconf", (uintptr_t)&sysconf_fake },
  { "th_comment_clear", (uintptr_t)&th_comment_clear },
  { "th_comment_init", (uintptr_t)&th_comment_init },
  { "th_decode_alloc", (uintptr_t)&th_decode_alloc },
  { "th_decode_ctl", (uintptr_t)&th_decode_ctl },
  { "th_decode_free", (uintptr_t)&th_decode_free },
  { "th_decode_headerin", (uintptr_t)&th_decode_headerin },
  { "th_decode_packetin", (uintptr_t)&th_decode_packetin },
  { "th_decode_ycbcr_out", (uintptr_t)&th_decode_ycbcr_out },
  { "th_granule_time", (uintptr_t)&th_granule_time },
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
  { "vorbis_granule_time", (uintptr_t)&vorbis_granule_time },
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

int check_kubridge(void) {
  int search_unk[2];
  return _vshKernelSearchModuleByName("kubridge", search_unk);
}


int main(int argc, char *argv[]) {
  sceSysmoduleLoadModule(9);

  scePowerSetArmClockFrequency(444);
  scePowerSetBusClockFrequency(222);
  scePowerSetGpuClockFrequency(222);
  scePowerSetGpuXbarClockFrequency(166);

  if (check_kubridge() < 0)
    fatal_error("Error kubridge.skprx is not installed.");

  if (so_load(&homm3_mod, SO_PATH, LOAD_ADDRESS) < 0)
    fatal_error("Error could not load %s.", SO_PATH);

  so_relocate(&homm3_mod);
  so_resolve(&homm3_mod, default_dynlib, sizeof(default_dynlib), 0);

  patch_game();

  so_flush_caches(&homm3_mod);
  so_initialize(&homm3_mod);

  int (* SDL_main)(void) = (void *)so_symbol(&homm3_mod, "SDL_main");
  SDL_main();

  return 0;
}
