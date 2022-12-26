#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

FILE *logfile;

#define OOMF_IMPL
#define BUF_IMPL
#include "buf.h"

#include "term.h"

static buf_t input = BUF_INIT;
static buf_t frame = BUF_INIT;
static ibuf_t newlines = IBUF_INIT;
//static ibuf_t marked = IBUF_INIT;

static int keyboard, display, in, out;    /* fd */

static int raw_mode_flag = 0;
static struct termios T;
static pair_t wsize = PAIR_ZERO;

static int hovered_file_line = 1;
static int quit = 0;
static int offset = 0;

int
min(int a, int b) { return (a < b) ? a : b; }

int
max(int a, int b) { return (a > b) ? a : b; }

void
finalize()
{
	fprintf(logfile, "amount of newlines: %zu\n", newlines.len);
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
	if (newlines.len < (unsigned) wsize.y) {
		int istartline = 0;
		for (int y = 1; y < wsize.y; y++) {
			if (y <= (long) newlines.len) {
				if (y == hovered_file_line)
					buf_append(&frame, "\e[4m", 4);
				if (y == 1)
					istartline = 0;
				else
					istartline = newlines.heap[y - 2 + offset] + 1;
				int slen = newlines.heap[y - 1 + offset] - istartline;
				buf_append(&frame, &input.heap[istartline], slen);
			} else {
				buf_append(&frame, "~", 1);
			}
			buf_append(&frame, "\e[0m", 4);
			buf_append(&frame, "\r\n", 2);
		}
	} else {
		fprintf(stderr, "%s:%d: windows is too small\r\n", __FILE__, __LINE__);
		exit(1);
	}
	write(dpy, frame.heap, frame.len);
}

void
print_line(buf_t input, ibuf_t newlines, unsigned long which, int ofd)
{
	unsigned long istartline = 0;
	if (which < 1 || which > newlines.len) return;
	if (which == 1) istartline = 0;
	else istartline = newlines.heap[which - 2] + 1;
	int slen = newlines.heap[which - 1] - istartline;
	write(ofd, &input.heap[istartline], slen);
}

void
update_wsize(int sig)
{
	if (0 != get_window_size(keyboard, display, &wsize)) {
		fprintf(stderr, "%s:%d: cannot get window size\r\n", __FILE__, __LINE__);
		exit(1);
	}
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
		case 'n':
			hovered_file_line += 1 * (hovered_file_line < (long) newlines.len);
			//if (hovered_file_line == wsize.y + offset) offset++;
		break;
		case 'k':
		case 'N':
			hovered_file_line += -1 * (hovered_file_line > 1);
			//if (hovered_file_line == 1 + offset && offset > 0) offset--;
		break;
		case '\r':
			print_line(input, newlines, hovered_file_line, out);
		case '\x1b':
		case 'q':
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

	logfile = fopen("cmenu.log", "w");
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
	fprintf(logfile, "old newlines: %zu\r\n", newlines.len);
	ibuf_scan(&input, '\n', &newlines);

	while (!quit) {
		paint_frame(display, input, frame, newlines);
		process_keys(keyboard);
	}

	return 0;
}
