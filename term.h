#ifndef TERM_H
#define TERM_H

#include <termios.h>
#include "pair.h"

int
enable_raw_mode(struct termios *orig, int fd, int *status);

void
disable_raw_mode(struct termios *orig, int fd, int *status);

int
get_cursor_position(int ifd, int ofd, pair_t *pos);

int
get_window_size(int ifd, int ofd, pair_t *size);

#endif
