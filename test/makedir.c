/*
tests userland ability to make directories
and specify relative paths for creating files
*/

#include "syscall.h"

int main() {
	MakeDir("test");
	Create("test/foo");
	if(Open("test/foo") < 0) {
		prints("error: file open failed\n");
		Exit(1);
	}

	prints("test passed\n");
	Exit(0);
}

prints(s,file)
char *s;
OpenFileId file;

{
  int count = 0;
  char *p;

  p = s;
  while (*p++ != '\0') count++;
  Write(s, count, file);  

}