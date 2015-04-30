/* vmtotture.c 
 *    Driver program for VM torture suite.
 */

#include "syscall.h"

int
main()
{
  SpaceId qmatmult, qsort, Akid, Bkid, Ckid, Dkid, Ekid;
  int retA, retB, retC, retD, retE, multresult, sortresult;
  char *args[3];

  prints("VMTORTURE beginning\n", ConsoleOutput);


/*********************************************/

  prints("QMATMULT\n", ConsoleOutput);
  /* QMATMULT */
  args[0] = "qmatmult";
  args[1] = (char *)0;
  if ((qmatmult =Exec("qmatmult", args, 0)) == -1) {
    prints("Could not Exec qmatmult\n");
    Halt();
  }

/*********************************************/

  prints("QSORT\n", ConsoleOutput);
  /* QSORT */
  args[0] = "qsort";
  args[1] = (char *)0;  
  if ((qsort = Exec("qsort", args, 0)) == -1) {
    prints("Could not Exec qsort\n");
    Halt();
  }

/*********************************************/

  prints("Console kids starting\n", ConsoleOutput);
  /* AKID */
  args[0] = "Xkid";
  args[1] = "A";
  args[2] = (char *)0;
  if ((Akid = Exec("Xkid", args, 0)) == -1) {
    prints("Could not Exec Akid\n");
    Halt();
  }

  /* BKID */
  args[0] = "Xkid";
  args[1] = "B";
  args[2] = (char *)0;
  if ((Bkid = Exec("Xkid", args, 0)) == -1) {
    prints("Could not Exec Bkid\n");
    Halt();
  }

  /* CKID */
  args[0] = "Xkid";
  args[1] = "C";
  args[2] = (char *)0;
  if ((Ckid = Exec("Xkid", args, 0)) == -1) {
    prints("Could not Exec Ckid\n");
    Halt();
  }

  /* DKID */
  args[0] = "Xkid";
  args[1] = "D";
  args[2] = (char *)0;
  if ((Dkid = Exec("Xkid", args, 0)) == -1) { 
    prints("Could not Exec Dkid\n");
    Halt();
  }

  /* EKID */
  args[0] = "Xkid";
  args[1] = "E";
  args[2] = (char *)0;
  if ((Ekid = Exec("Xkid", args, 0)) == -1) {
    prints("Could not Exec Ekid\n");
    Halt();
  }

  /* Collect Xkids silently. Get result from Exit value of qmatmult and qsort. */

  retA = Join(Akid);
  retB = Join(Bkid);
  retC = Join(Ckid);
  retD = Join(Dkid);
  retE = Join(Ekid);

  prints("\n\nKids done\n\n", ConsoleOutput);

  if (retA !=0 ) {
    prints("Akid bad exit value: ", ConsoleOutput); printd(retA, ConsoleOutput); prints("\n", ConsoleOutput);
  }
  if (retB !=0 ) {
    prints("Bkid bad exit value: ", ConsoleOutput); printd(retB, ConsoleOutput); prints("\n", ConsoleOutput);
  }
  if (retC !=0 ) {
    prints("Ckid bad exit value: ", ConsoleOutput); printd(retC, ConsoleOutput); prints("\n", ConsoleOutput);
  }
  if (retD !=0 ) {
    prints("Dkid bad exit value: ", ConsoleOutput); printd(retD, ConsoleOutput); prints("\n", ConsoleOutput);
  }
  if (retE !=0 ) {
    prints("Ekid bad exit value: ", ConsoleOutput); printd(retE, ConsoleOutput); prints("\n", ConsoleOutput);
  }

  prints("Awaiting qmatmult\n", ConsoleOutput);
  multresult = Join(qmatmult);
  prints("qmatmult Exit value is ", ConsoleOutput);
  printd(multresult, ConsoleOutput);
  prints("\n", ConsoleOutput);
  prints("Awaiting qsort\n", ConsoleOutput);
  sortresult = Join(qsort);
  prints("qsort Exit value is ", ConsoleOutput);
  printd(sortresult, ConsoleOutput);

  prints("\n\nVMTORTURE terminating normally\n", ConsoleOutput);
  Halt();
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
