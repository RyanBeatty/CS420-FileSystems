/*
tests userland ability to make directories
and specify relative paths for creating files
*/

#include "syscall.h"

int main() {
	MakeDir("test");
	Create("foo");
	if(Open("test/foo") < 0) {
		Write("error: file open failed\n", sizeof("error: file open failed\n"), 1);
		Exit(1);
	}

	Exit(0);
}