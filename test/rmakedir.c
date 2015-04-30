/*
tests userland ability to make directories
and specify relative paths for creating files
*/

#include "syscall.h"

int main() {
	OpenFileId id;
	int result;

	result = MakeDir("test");
	if(result == -1) {
		prints("error: could not create Directory\n", ConsoleOutput);
		Exit(1);
	}
	prints("created directory\n", ConsoleOutput);

	Create("test/foo");
	id = Open("test/foo");
	if(id < 0) {
		prints("error: file open failed\n", ConsoleOutput);
		Close(id);
		Exit(1);
	}

	prints("test passed\n", ConsoleOutput);
	Close(id);
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