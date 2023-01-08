#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct buf_t buf_t;
struct buf_t {
	unsigned long cap;
	unsigned long len;
	char *heap;         /* = NULL => not initialized */
};

typedef struct ibuf_t ibuf_t;
struct ibuf_t {
	unsigned long cap;
	unsigned long len;
	long *heap;         /* = NULL => not initialized */
};

#define BUF_INIT {0,0,NULL}

void
buf_append(buf_t *dst, char *src, unsigned long slen);

void
ibuf_append(ibuf_t *dst, unsigned long idx);

void
ibuf_scan(buf_t *src, char target, ibuf_t *dst);

void
ibuf_destroy(ibuf_t *dst);

void
buf_empty(buf_t *dst);

void
buf_destroy(buf_t *dst);

void
oomf(void *ptr, const char *filename, const int line);

#define OOMF(ptr) do { oomf((ptr), __FILE__, __LINE__); } while (0);
