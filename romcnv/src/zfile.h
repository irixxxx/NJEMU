#ifndef ZFILE_H
#define ZFILE_H

#include <limits.h>
#include <stdint.h>
#include <stddef.h>

struct zip_find_t
{
	char name[PATH_MAX];
	uint32_t length;
	uint32_t crc32;
};

int64_t zip_open(const char *path, const char *mode);
void zip_close(void);

int zip_findfirst(struct zip_find_t *file);
int zip_findnext(struct zip_find_t *file);

int64_t zopen(const char *filename);
size_t zread(int64_t fd, void *buf, unsigned size);
int zwrite(int64_t fd, void *buf, unsigned size);
int zgetc(int64_t fd);
int zclose(int64_t fd);
size_t zsize(int64_t fd);
int zcrc(int64_t fd);

#endif /* ZFILEH */
