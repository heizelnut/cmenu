#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define OOMF_IMPL
#define BUF_IMPL
#include "buf.h"

#include "term.h"

static buf_t input = BUF_INIT;
static buf_t frame = BUF_INIT;
static ibuf_t newlines = BUF_INIT;

static int keyboard, display, in, out;    /* fd */

static int raw_mode_flag = 0;
static struct termios T;
static pair_t wsize = PAIR_ZERO;

static unsigned int title_rows = 0;
static unsigned int hovered_row = 1;
static int quit = 0;
static unsigned int offset = 0;

static int exit_code = EXIT_FAILURE;

int
min(int a, int b) { return (a < b) ? a : b; }

int
max(int a, int b) { return (a > b) ? a : b; }

void
finalize()
{
	write(display, "\x1b[?25h", 6);
	disable_raw_mode(&T, display, &raw_mode_flag);
	close(keyboard);
	buf_destroy(&input);
	buf_destroy(&frame);
	close(display);
	close(in);
	close(out);
}

void
init_streams(int *keyboard, int *display, int *in, int *out)
{
	*keyboard = STDIN_FILENO;
	*in = dup(*keyboard);
	(void) freopen("/dev/tty", "r", stdin);
	*out = STDOUT_FILENO;
	*display = STDERR_FILENO;

	if (isatty(*in) != 0) {
		fprintf(stderr, "%s:%d: no piped input\r\n", __FILE__, __LINE__);
		exit(1);
	}
	if (isatty(*display) != 1) {
		fprintf(stderr, "%s:%d: output is not tty\n\n", __FILE__, __LINE__);
		exit(1);
	}
}

void
paint_frame(int dpy, buf_t input, buf_t frame, ibuf_t newlines)
{
	write(dpy, "\x1b[2J\x1b[H", 7);
	buf_empty(&frame);
	buf_append(&frame, "\x1b[?25l", 6);
	buf_append(&frame, "\x1b[H", 3);
	for (unsigned int row = 1 + offset; row < offset + wsize.y; row++) {
		if (row <= title_rows) buf_append(&frame, "\e[1m", 4);
		if (row == hovered_row) buf_append(&frame, "\e[4m", 4);
		if (row - 1 >= newlines.len) {
			buf_append(&frame, "~\r\n", 3);
		} else {
			int istart = (row > 1) ? (newlines.heap[row - 2] + 1) : 0;
			int slen = newlines.heap[row - 1] - istart;
			buf_append(&frame, &input.heap[istart], slen);
			buf_append(&frame, "\e[0m", 4);
			buf_append(&frame, "\r\n", 2);
		}
	}
	write(dpy, frame.heap, frame.len);
}

void
print_line(buf_t input, ibuf_t newlines, unsigned long which, int ofd)
{
	unsigned long istart = 0;
	if (which < 1 || which > newlines.len) return;
	if (which == 1) istart = 0;
	else istart = newlines.heap[which - 2] + 1;
	int slen = newlines.heap[which - 1] - istart;
	write(ofd, &input.heap[istart], slen);
}

void
update_wsize(int sig)
{
	if (0 != get_window_size(keyboard, display, &wsize)) {
		fprintf(stderr, "%s:%d: cannot get window size\r\n", __FILE__, __LINE__);
		exit(1);
	}
	offset = 0;
	if (sig == SIGWINCH) paint_frame(display, input, frame, newlines);
}

int
read_key(int fd) {
	int n;
	char c;
	while ((n = read(fd, &c, 1)) == 0);
	if (n == -1) {
		fprintf(stderr, "%s:%d: read key failure\r\n", __FILE__, __LINE__);
		exit(1);
	}
	return c;
}

void
process_keys(int kbd)
{
	char k = read_key(kbd);

	switch(k) {
		case 'j':
			hovered_row += 1 * (hovered_row < (long) newlines.len);
			if (hovered_row - offset == wsize.y)
				offset += 1 * (offset <= newlines.len - wsize.y);
		break;
		case 'k':
			hovered_row += -1 * (hovered_row > 1 + title_rows);
			if (hovered_row - offset == title_rows)
				offset -= 1 * (offset > 0);
		break;
		case '\r':
			print_line(input, newlines, hovered_row, out);
			exit_code = EXIT_SUCCESS;
		case '\x1b':
			quit = 1;
		break;
	}
	return;
}

int
main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	int opt;
	while ((opt = getopt(argc, argv, "t:d:")) != -1) {
		switch (opt) {
		case 't':
			title_rows = max(0, strtol(optarg, NULL, 10));
		break;
		case 'd':
			hovered_row = max(0, strtol(optarg, NULL, 10));
		break;
		default:
			fprintf(stderr, "Usage: %s [-d n] [-t m]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	hovered_row = (hovered_row <= title_rows) ? 1 + title_rows : hovered_row;

	atexit(finalize);

	init_streams(&keyboard, &display, &in, &out);

	enable_raw_mode(&T, display, &raw_mode_flag);

	update_wsize(0);

	signal(SIGWINCH, update_wsize);
	signal(SIGINT, finalize);

	char chunk[1000];
	FILE *in_stream = fdopen(in, "r");
	while (1) {
		fgets(chunk, 1000, in_stream);
		if (feof(in_stream)) break;
		buf_append(&input, chunk, strlen(chunk));
	}
	
	fclose(in_stream);
	ibuf_scan(&input, '\n', &newlines);

	while (!quit) {
		paint_frame(display, input, frame, newlines);
		process_keys(keyboard);
	}

	return exit_code;
}
