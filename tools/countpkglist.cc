#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <rpm/rpmlib.h>

int main(int argc, char *argv[])
{
	Header header = NULL;
	long count = 0;
	FD_t fd;
	if (argc != 2) {
		fprintf(stderr, "usage: countpkglist <pkglist>\n");
		exit(1);
	}
	fd = Fopen(argv[1], "r");
	if (fd == NULL) {
		fprintf(stderr, "error: can't open %s: %s",
			argv[1], strerror(errno));
		exit(1);
	}
	while ((header = headerRead(fd, HEADER_MAGIC_YES)) != NULL) {
		headerFree(header);
		count++;
	}
	Fclose(fd);
	printf("%ld\n", count);
	return 0;
}
