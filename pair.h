#ifndef PAIR_H
#define PAIR_H

typedef struct {
	int x;
	int y;
} pair_t;

#define PAIR_ZERO (pair_t){0,0}
#define are_pairs_equal(a, b) ((a).x == (b).x && (a).y == (b).y)
#define is_pair_zero(pair) are_pairs_equal((pair), (PAIR_ZERO))

#endif
