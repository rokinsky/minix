#include <unistd.h>

int main(int argc, char** argv)
{
	distort_time(getpid(), 2);
	return 0;
}