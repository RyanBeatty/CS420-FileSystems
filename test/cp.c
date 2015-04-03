

#define N 100
#define INFILE "in.txt"
#define OUTFILE "res.txt"
#include "syscall.h"

void clear(char *b) {
	int i = 0;
	for(; i < N; ++i)
		b[i] = '\0';
}

int main() {
	int fdin;
	int fdout;
	int read;
	char buffer[N];

	Create(OUTFILE);
	
	fdin = Open(INFILE);
	if(fdin < 0) {
		Write("error: could not open file in.txt\n", sizeof("error: could not open file in.txt\n"), 1);
		Exit(1);
	}

	fdout = Open(OUTFILE);
	if(fdin < 0) {
		Write("error: could not open res.txt\n", sizeof("error: could not open res.txt\n"), 1);
		Close(fdin);
		Exit(1);
	}

	clear(buffer);
	while((read = Read(buffer, N, fdin)) > 0) {
		Write(buffer, read, fdout);
		clear(buffer);
	}

	Close(fdin);
	Close(fdout);
	Write("finished copy\n", sizeof("finished copy\n"), 1);
	Exit(0);
}
