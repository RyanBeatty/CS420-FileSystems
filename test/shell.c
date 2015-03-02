#ifdef CHANGED
#include "syscall.h"

#define NUM_ARGS 11
#define ARG_LEN 60

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char *argv[NUM_ARGS], args[NUM_ARGS][ARG_LEN], prompt[2], ch, buffer[ARG_LEN];
    int i, argIndex, curChar;

    prompt[0] = '-';
    prompt[1] = '-';

    
    while( 1 )
    {
    	for(i = 0; i < ARG_LEN; ++i)
    		buffer[i] = '\0';
	Write(prompt, 2, output);

	i = 0;
	
	do {
	
	    Read(&buffer[i], 1, input); 

	} while( buffer[i++] != '\n' );

	buffer[--i] = '\0';

	if( i > 0 ) {

		for(argIndex = 0; argIndex < NUM_ARGS; ++argIndex)
			args[argIndex][0] = '\0';

		for(i = 0, argIndex = 0, curChar = 0; i < ARG_LEN; ++i) {
			if(buffer[i] == ' ') {
				args[argIndex++][curChar] = '\0';
				curChar = 0;
			}
			else
				args[argIndex][curChar++] = buffer[i];
		}

		for(i = 0; i <= argIndex; ++i)
			argv[i] = args[i];

		argv[argIndex+1] = (char *) 0;

		newProc = Exec(argv[0], argv, 0);
		Join(newProc);

	}
    }
}
#else
#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], ch, buffer[60];
    int i;

    prompt[0] = '-';
    prompt[1] = '-';

    while( 1 )
    {
	Write(prompt, 2, output);

	i = 0;
	
	do {
	
	    Read(&buffer[i], 1, input); 

	} while( buffer[i++] != '\n' );

	buffer[--i] = '\0';

	if( i > 0 ) {
		newProc = Exec(buffer);
		Join(newProc);
	}
    }
}
#endif //CHANGED
