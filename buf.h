#ifndef BUF_H
#define BUF_H

/* dynamic append-only buffer */

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
	unsigned long pass;
	long *heap;         /* = NULL => not initialized */
};

#define BUF_INIT {0,0,NULL}
#define IBUF_INIT {0,0,0,NULL}

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

#ifdef BUF_IMPL
#include <stdio.h>
#include <string.h>

#ifndef DEBUG
#define DEBUG 0
#endif

void
oomf(void *ptr, const char *filename, const int line)
{
	if (ptr != NULL) return;
	fprintf(stderr, "%s:%d: out of memory\n", filename, line);
	exit(EXIT_FAILURE);
}

void
buf_append(buf_t *dst, char *src, unsigned long slen)
{
	if (dst == NULL) return;
	if (dst->heap == NULL) {
		DEBUG && fprintf(logfile, "[BUF] alloc\n");
		char *tmp = (char*) malloc(sizeof(char) * 8);
		OOMF(tmp);
		dst->heap = tmp;
		dst->cap = 8;
		dst->len = 0;
	}
	while (dst->len + slen >= dst->cap) {
		DEBUG && fprintf(logfile, "[BUF] realloc x2\n");
		char *tmp = (char*) realloc(dst->heap, dst->cap * 2);
		OOMF(tmp);
		dst->heap = tmp;
		dst->cap *= 2;
	}
	memcpy(dst->heap + dst->len, src, slen);
	dst->len += slen;
	DEBUG && fprintf(logfile, "[BUF] cap: %zu, len: %zu\n", dst->cap, dst->len);
	return;
}

void
ibuf_append(ibuf_t *dst, unsigned long idx)
{
	if (dst == NULL) return;
	if (dst->heap == NULL) {
		long *tmp = (long*) malloc(sizeof(long) * 64);
		OOMF(tmp);
		dst->heap = tmp;
		dst->cap = 64;
		dst->len = 0;
		dst->pass = 0;
	}
	while (dst->len + 1 >= dst->cap) {
		long *tmp = (long*) realloc(dst->heap, dst->cap * 2);
		OOMF(tmp);
		dst->heap = tmp;
		dst->cap *= 2;
	}
	dst->heap[dst->len++] = idx;
	return;
}

void
ibuf_scan(buf_t *src, char target, ibuf_t *dst)
{
	for (unsigned long i = 0; i < src->len; i++)
		if (src->heap[i] == target)
			ibuf_append(dst, i);
}

void
buf_empty(buf_t *dst)
{
	DEBUG && fprintf(logfile, "[BUF] empty\n");
	if (dst == NULL) return;
	if (dst->heap == NULL) return;
	dst->len = 0;
}

void
buf_destroy(buf_t *dst)
{
	DEBUG && fprintf(logfile, "[BUF] destroy\n");
	if (dst == NULL) return;
	free(dst->heap);
	dst->heap = NULL;
	dst->cap = 0;
	dst->len = 0;
}

void
ibuf_destroy(ibuf_t *dst)
{
	DEBUG && fprintf(logfile, "[BUF] destroy ibuf\n");
	if (dst == NULL) return;
	free(dst->heap);
	dst->heap = NULL;
	dst->cap = 0;
	dst->len = 0;
}

#endif /* implem */
#endif /* header */
