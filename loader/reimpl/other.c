#include <errno.h>
#include <psp2/kernel/clib.h>
#include "other.h"


void __assert2(const char *file, int line, const char *func, const char *expr)
{
	sceClibPrintf("assertion failed:\n%s:%d (%s): %s\n", file, line, func, expr);
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	return malloc(len);
/*
	(void)prot;
	(void)flags;

	void *data = malloc(len);

	if (data != NULL) {
		int res = lseek(fd, offset, SEEK_SET);
		if (res != -1) {
			read(fd, data, len);
		}
	} else {
		return NULL;
	}

	if (addr != NULL) {
		addr = data;
	}

	return data;
*/
}

int munmap(void *map, size_t length)
{
	(void)length;

	if (map != NULL) {
		free(map);
	}

	return 0;
}

// From https://github.com/kraj/uClibc/blob/master/libc/misc/dirent/scandir.c
int scandir_hook(const char *dir, struct android_dirent ***namelist,
	int (*selector) (const struct dirent *),
	int (*compar) (const struct dirent **, const struct dirent **))
{
	DIR *dp = opendir (dir);
	struct dirent *current;
	struct android_dirent d;
	struct android_dirent *android_current = &d;
	struct android_dirent **names = NULL;
	size_t names_size = 0, pos;

	if (dp == NULL)
		return -1;

	pos = 0;
	while ((current = readdir (dp)) != NULL) {
		int use_it = selector == NULL;
		
		memcpy(android_current->d_name, current->d_name, 256);
		android_current->d_type = SCE_S_ISDIR(current->d_stat.st_mode) ? 4 : 8;

		if (! use_it) {	
			use_it = (*selector) (android_current);
		}
		if (use_it) {
			struct android_dirent *vnew;
			size_t dsize;

			if (pos == names_size)
			{
				struct android_dirent **new;
				if (names_size == 0)
					names_size = 10;
				else
					names_size *= 2;
				new = (struct android_dirent **) realloc (names, names_size * sizeof (struct android_dirent *));
				if (new == NULL)
					break;
				names = new;
			}

			dsize = &android_current->d_name[256+1] - (char*)android_current;
			vnew = (struct android_dirent *) malloc (dsize);
			if (vnew == NULL)
				break;

			names[pos++] = (struct android_dirent *) memcpy (vnew, android_current, dsize);
		}
	}

	if (errno != 0) {
		closedir (dp);
		while (pos > 0)
			free (names[--pos]);
		free (names);
		return -1;
	}

	closedir (dp);

	/* Sort the list if we have a comparison function to sort with.  */
	if (compar != NULL)
		qsort (names, pos, sizeof (struct android_dirent *), (__compar_fn_t) compar);
	*namelist = names;
	return pos;
}

int stat_hook(const char *pathname, void *statbuf)
{
	struct stat st;
	int res = stat(pathname, &st);
	if (res == 0)
		*(int *)(statbuf + 0x50) = st.st_mtime;
	return res;
}
