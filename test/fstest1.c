#include "syscall.h"

int main() {
	int fd;

	Create("fstest1.txt");
	fd = Open("fstest1.txt");
	if(fd < 0) {
		Write("error: could not open file\n", sizeof("error: could not open file\n"), 1);
		Exit(1);
	}
	Write("hello world\n", sizeof("hello world\n"), fd);
	Close(fd);
	Exit(0);
}