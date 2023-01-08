#include "buf.h"

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
		dst->cap = 16;
		dst->len = 0;
		char *tmp = (char*) malloc(dst->cap * sizeof(dst->heap[0]));
		OOMF(tmp);
		dst->heap = tmp;
	}
	while (dst->len + slen >= dst->cap) {
		dst->cap *= 2;
		char *tmp = (char*) realloc(dst->heap, dst->cap * sizeof(dst->heap[0]));
		OOMF(tmp);
		dst->heap = tmp;
	}
	memcpy(dst->heap + dst->len, src, slen);
	dst->len += slen;
	return;
}

void
ibuf_append(ibuf_t *dst, unsigned long idx)
{
	if (dst == NULL) return;
	if (dst->heap == NULL) {
		dst->cap = 8;
		dst->len = 0;
		long *tmp = (long*) malloc(dst->cap * sizeof(dst->heap[0]));
		OOMF(tmp);
		dst->heap = tmp;
	}
	if (dst->len + 1 >= dst->cap) {
		dst->cap *= 2;
		long *tmp = (long*) realloc(dst->heap, dst->cap * sizeof(dst->heap[0]));
		OOMF(tmp);
		dst->heap = tmp;
	}
	dst->heap[dst->len++] = idx;
	return;
}

void
ibuf_scan(buf_t *src, char target, ibuf_t *dst)
{
	for (unsigned long i = 0; i < src->len; i++) {
		if (src->heap[i] == target) {
			ibuf_append(dst, i);
		}
	}
}

void
buf_empty(buf_t *dst)
{
	if (dst == NULL) return;
	if (dst->heap == NULL) return;
	dst->len = 0;
}

void
buf_destroy(buf_t *dst)
{
	if (dst == NULL) return;
	free(dst->heap);
	dst->heap = NULL;
	dst->cap = 0;
	dst->len = 0;
}

void
ibuf_destroy(ibuf_t *dst)
{
	if (dst == NULL) return;
	free(dst->heap);
	dst->heap = NULL;
	dst->cap = 0;
	dst->len = 0;
}
