#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "qotd.h"

static const char *DEFAULT_DEVICE = "/dev/qotd0";
static const char *VERSION = "1.0";

static void show_usage(const char *);
static void get_size(const char *);
static void set_size(const char *, size_t);
static void reset_dev(const char *);

int
main(int argc, char *argv[])
{
	int op = -1;
	int opt;
	int invalid_usage = 0;
	size_t sz_arg;
	const char *device = DEFAULT_DEVICE;

	while ((opt = getopt(argc, argv,
	    "d:(device)g(get-size)h(help)r(reset)s:(set-size)V(version)"))
	    != -1) {
		switch (opt) {
		case 'd':
			device = optarg;
			break;
		case 'g':
			if (op >= 0)
				invalid_usage++;

			op = QOTDIOCGSZ;
			break;
		case 'n':
			show_usage(argv[0]);
			exit(0);
			/*NOTREACHED*/
		case 'r':
			if (op >= 0)
				invalid_usage++;

			op = QOTDIOCDISCARD;
			break;
		case 's':
			if (op >= 0)
				invalid_usage++;

			op = QOTDIOCSSZ;
			sz_arg = (size_t)atol(optarg);
			break;
		case 'V':
			(void) printf("qotdctl %s\n", VERSION);
			exit(0);
			/*NOTREACHED*/
		default:
			invalid_usage++;
			break;
		}
	}

	if (invalid_usage > 0 || op < 0) {
		show_usage(argv[0]);
		exit(1);
	}

	switch (op) {
	case QOTDIOCGSZ:
		get_size(device);
		break;
	case QOTDIOCSSZ:
		set_size(device, sz_arg);
		break;
	case QOTDIOCDISCARD:
		reset_dev(device);
		break;
	default:
		(void) fprintf(stderr,
		    "internal error - invalid operation %d\n", op);
		exit(2);
	}

	return (0);
}

static void
show_usage(const char *execname)
{
	(void) fprintf(stderr,
	    "Usage: %s [-d device] {-g | -h | -r | -s size | -V}\n", execname);
}

static void
get_size(const char *dev)
{
	size_t sz;
	int fd;

	if ((fd = open(dev, O_RDONLY)) < 0) {
		perror("open");
		exit(3);
	}

	if (ioctl(fd, QOTDIOCGSZ, &sz) < 0) {
		perror("QOTDIOCGSZ");
		exit(4);
	}

	(void) close(fd);

	(void) printf("%zu\n", sz);
}

static void
set_size(const char *dev, size_t sz)
{
	int fd;

	if ((fd = open(dev, O_RDWR)) < 0) {
		perror("open");
		exit(3);
	}

	if (ioctl(fd, QOTDIOCSSZ, &sz) < 0) {
		perror("QOTDIOCSSZ");
		exit(4);
	}

	(void) close(fd);
}

static void
reset_dev(const char *dev)
{
	int fd;

	if ((fd = open(dev, O_RDWR)) < 0) {
		perror("open");
		exit(3);
	}

	if (ioctl(fd, QOTDIOCDISCARD) < 0) {
		perror("QOTDIOCDISCARD");
		exit(4);
	}

	(void) close(fd);
}
