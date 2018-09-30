/*
 * stdio redirection
 */

#include <stdio.h>
#include "eusci.h"

#define BLOCK	true

int _write(int fd, const void *buf, size_t count) {
	for (fd = 0; (size_t) fd < count; fd++) {
		if (_putc(SERIAL_DEBUG, BLOCK, *((char *) buf++)))
			return fd;
	}
	return count;
}

int _read(int fd, const void *buf, size_t count) {
	for (fd = 0; (size_t) fd < count; fd++) {
		if (_getc(SERIAL_DEBUG, BLOCK, (char *) buf++))
			return fd;
	}
	return count;
}
