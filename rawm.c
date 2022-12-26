#include <unistd.h>

#include <stdio.h>

#include "term.h"

/* raw mode: 1960 magic shit */

int
enable_raw_mode(struct termios *orig, int fd, int *status)
{
	struct termios raw;

	if (*status) return 0; /* already enabled */
	if (tcgetattr(fd, orig) == -1)
		return -1;

	raw = *orig;  /* modify the original mode */
	/* input modes: no break, no CR to NL, no parity check, no strip char,
	 * no start/stop output control */
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	/* output modes - disable post processing */
	raw.c_oflag &= ~(OPOST);
	/* control modes - set 8 bit chars */
	raw.c_cflag |= (CS8);
	/* local modes - choing off, canonical off, no extended functions,
	 * no signal chars (^Z,^C) */
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	/* control chars - set return condition: min number of bytes and timer */
	raw.c_cc[VMIN] = 0; /* return each byte, or zero for timeout */
	raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second) */

	/* put terminal in raw mode after flushing */
	if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
		return -1;

	*status = 1;
	return 0;
}

void
disable_raw_mode(struct termios *orig, int fd, int *status)
{
	if (*status) {
		tcsetattr(fd, TCSAFLUSH, orig);
		*status = 0;
	}
}
