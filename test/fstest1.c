#include "syscall.h"

int main() {
	int fd;

	Create("t1.txt");
	fd = Open("t1.txt");
	if(fd < 0) {
		Write("error: could not open file\n", sizeof("error: could not open file\n"), 1);
		Exit(1);
	}
	Write("hello world\n", sizeof("hello world\n"), fd);
	Close(fd);
	Write("finished\n", sizeof("finished\n"), 1);
	Exit(0);
}
