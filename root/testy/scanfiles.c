#include <fcntl.h>
#include <unistd.h>

#define CYCLES_COUNT 500
#define BUFFER_SIZE  1
#define STDOUT       1

char buf[BUFFER_SIZE];

void scan(char* file)
{
	int fd, bytes;
	fd = open(file, O_RDONLY);
	while ((bytes = read(fd, buf, BUFFER_SIZE)))
		write(STDOUT, buf, bytes);
	close(fd);
}

int main(int argc, char** argv)
{
	int i, j;
	for (i = 1; i < argc; ++i)
		for (j = 0; j < CYCLES_COUNT; ++j)
			scan(argv[i]);
	return 0;
}
