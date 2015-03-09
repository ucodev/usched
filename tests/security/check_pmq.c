#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mqueue.h>

#define PMQ_NAME	"/uschedpmq01"

static void _exit_failure(const char *err) {
	fprintf(stderr, "Fatal: %s\n", err);

	exit(EXIT_FAILURE);
}

static int _privdrop(void) {
	if (getuid())
		return 0;

	/* Drop privs to any non-root user */
	if (setregid(1, 1) < 0)
		return -1;

	if (setreuid(1, 1) < 0)
		return -1;

	return 0;
}

static int _pmq_open_write(void) {
	mqd_t ret = (mqd_t) -1;

	if ((ret = mq_open(PMQ_NAME, O_WRONLY)) == (mqd_t) -1) {
		fprintf(stderr, "mq_open(\"%s\", O_WRONLY): %s\n", PMQ_NAME, strerror(errno));
		return -(errno != EACCES); /* We expect EACCES */
	}

	mq_close(ret);

	return -1;
}

static int _pmq_open_read(void) {
	mqd_t ret = (mqd_t) -1;

	if ((ret = mq_open(PMQ_NAME, O_RDONLY)) == (mqd_t) -1) {
		fprintf(stderr, "mq_open(\"%s\", O_RDONLY): %s\n", PMQ_NAME, strerror(errno));
		return -(errno != EACCES); /* We expect EACCES */
	}

	mq_close(ret);

	return -1;
}

int main(void) {
	if (_privdrop() < 0)
		_exit_failure("Unable to drop privs");

	if (_pmq_open_write() < 0)
		_exit_failure("Something went wrong while testing message queue for writting.");

	if (_pmq_open_read() < 0)
		_exit_failure("Something went wrong while testing message queue for reading.");

	puts("OK");

	return 0;
}

