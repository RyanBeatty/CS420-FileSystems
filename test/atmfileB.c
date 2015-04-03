/* atomicfile2.c
 *
 * Kid in the test of atomic file output.
 *
 */

#include "syscall.h"

#define ITERATIONS 20
#define DELAY 10000

/* Will adjust SHAREDFD if needed. */

#define SHAREDFD 2

int
main()
{
  int i, j, k;

  for (i=0; i < ITERATIONS; i++) {
    k = 0;
    for (j=0; j < DELAY; j++) k++;
    Write("BBBBB", 5, SHAREDFD);
  }
  Exit(0);
}

/* Print a null-terminated string "s" on open file descriptor "file". */

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


/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i, pos=0, divisor=1000000000, d, zflag=1;
  char c;
  char buffer[11];
  
  if (n < 0) {
    buffer[pos++] = '-';
    n = -n;
  }
  
  if (n == 0) {
    Write("0",1,file);
    return;
  }

  for (i=0; i<10; i++) {
    d = n / divisor; n = n % divisor;
    if (d == 0) {
      if (!zflag) buffer[pos++] =  (char) (d % 10) + '0';
    } else {
      zflag = 0;
      buffer[pos++] =  (char) (d % 10) + '0';
    }
    divisor = divisor/10;
  }
  Write(buffer,pos,file);
}





