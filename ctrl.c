#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "term.h"
#include "pair.h"

/* terminal control miscellaneous */

int
get_cursor_position(int ifd, int ofd, pair_t *pos)
{
	char mem[32];
	unsigned int i = 0;
	int rows, cols;

	/* report cursor location */
	if (write(ofd, "\x1b[6n", 4) != 4) return -1;

	/* read the response: GST. [ rows ; cols R */
	while (i < sizeof(mem)-1) {
		if (read(ifd,mem+i,1) != 1) break;
		if (mem[i] == 'R') break;
		i++;
	}
	mem[i] = '\0';

	/* parse it */
	if (mem[0] != '\x1b' || mem[1] != '[') return -1;
	if (sscanf(mem+2, "%d;%d", &rows, &cols) != 2) return -1;

	pos->x = cols;
	pos->y = rows;
	return 0;
}

int
get_window_size(int ifd, int ofd, pair_t *size)
{
	struct winsize ws;

	/* try ioctl, on fail query the terminal */
	if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		pair_t orig = PAIR_ZERO;
		int retval;

		/* backup original position */
		retval = get_cursor_position(ifd, ofd, &orig);
		if (retval == -1) goto failed;

		/* go to right/bottom margin and query */
		if (write(ofd, "\x1b[999C\x1b[999B", 12) != 12) goto failed;
		retval = get_cursor_position(ifd, ofd, size);
		if (retval == -1) goto failed;

		/* restore original position */
		char seq[32];
		snprintf(seq, 32, "\x1b[%d;%dH", orig.y, orig.x);
		if (write(ofd, seq, strlen(seq)) == -1) goto failed;

		return 0;
	} else {
		size->x = ws.ws_col;
		size->y = ws.ws_row;
		return 0;
	}
	
failed:
	return -1;
}
