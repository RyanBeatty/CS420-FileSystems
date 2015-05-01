/*

*/

#include "syscall.h"

#define N 100

void
createdir(char *name) {
	int result;
	result = MakeDir(name);
	if(result == -1) {
		prints("error: could not create Directory\n", ConsoleOutput);
		Exit(1);
	}
	prints("created directory: ", ConsoleOutput);
	prints(name, ConsoleOutput);
	prints("\n", ConsoleOutput);
}

void
changedir(char *name) {
	int result;
	result = ChangeDir(name);
	if(result == -1) {
		prints("error: ChangeDir() failed\n", ConsoleOutput);
		Exit(1);
	}

	prints("changed directory\n", ConsoleOutput);
}

OpenFileId
create_and_open_file(char *name) {
	OpenFileId id;
	Create(name);
	id = Open(name);
	if(id < 0) {
		prints("error: file open failed\n", ConsoleOutput);
		Close(id);
		Exit(1);
	}

	prints("created file: ", ConsoleOutput);
	prints(name, ConsoleOutput);
	prints("\n", ConsoleOutput);
	return id;
}


int main() {
	char buffer[N];
	OpenFileId in, out;
	int result, num_read;

	in = Open("input");
	if(in < 0) {
		prints("error: file open failed\n", ConsoleOutput);
		Close(in);
		Exit(1);
	}

	createdir("hello");
	createdir("hello/world");
	createdir("hello/world/foo");
	changedir("hello/world/foo");
	out = create_and_open_file("bar");

	while((num_read = Read(buffer, N, in)) > 0)
		Write(buffer, num_read, out);

	prints("copied over file\n", ConsoleOutput);
	Close(in);
	Close(out);
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