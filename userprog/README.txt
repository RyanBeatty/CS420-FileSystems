Nachos2

CS444 Project, Fall 2014
Hareesh Nagaraj, Ryan Beatty, Joe Soultanis

Project 2
*************************************************

    ----------------system-wide changes we made are as follows-----------------------------------

    We created handler functions called SC_OPEN, SC_CREATE, SC_CLOSE, SC_WRITE, SC_READ, SC_JOIN, 
    SC_EXEC, SC_EXIT which implement the behavior of each system call.
    When a SystemCallException is raised, the corresponding handler function is invoked and excutes 
    the behavior of the system call. 

    In order to implement address translation we created a method, V2P(), in the AddrSpace class
    that takes in a virtual address and returns the corresponding physical address in memory for
    that AddrSpace. *NOTE* we set a max limit on the length of user-space strings to be 128

    In order to load user-land argument strings into kernal space, we created LoadStringFromMemory() 
    which takes in the virtual starting address of the user-space string, translates the virtual 
    address into a physical one using V2P(), then grabs the string out of the machine's main memory 
    and copies it into a buffer. *NOTE* we set a max limit on the length of user-space strings to be 
    128

    In order to save kernel strings into user-space, we created SaveStringToMemory() which takes in 
    a string to save and the virtual address of where to save it and then translates the virtual 
    address and stores the string in main memory.

    In order to provide an interface for managing file descriptors and Openfile's, we created 
    several classes: TableEntry, Table, OpenFileTable, and FileVector.


    "bitMap": manages allocation of physical pages for processes in the system.

    "timeSlicer": a timer that will allow time slicing between processes, simulating a multiprogrammed 
    system. The interrupt handler for the timer is TimeSlicerInterruptHandler which simply Yields() execution
     of the current Thread to another thread.

    "processList": a ProcessList object that manages exit statuses of running processes (see 
    explanation below)

    "processId": the id number that the next new process that is created will be assigned.


    "ioLock": lock that provides mutual exclusion for Read()/Write() allowing those operations
    to be atomic.

    "globalFileTable": An OpenFileTable object (see explanation below) that provides an interface to all 
    processes for managing OpenFile objects


    "synchConsole": Provides an interface for reading from and writing to the Console
    --** synchconsole.cc **--

    -implements routines to synchronously access the console.

    -char SynchConsole::SynchConsole(char *in, char *out): creates a readLock, writeLock, read Semaphore, 
    write Semaphore. Initializes the normal console where arguments are the file descriptors to be written 
    and read from.

    -char SynchConsole::ReadChar(): P's the read semaphore (makes sure there is content to read), ensures 
    mutex access to console, gets a char from the console and returns it.

    -void SynchConsole::ReadAvail(): just V's the read semaphore to indicate there is data to be read and 
    allows SynchConsole::ReadChar() to continue.

    -void SynchConsole::WriteChar(char c): aquires the writeLock to ensure mutex, calls  PutChar on the 
    console, then P's on the write semaphore, waiting for SynchConsole::WriteDone(). Then releases the 
    writeLock.

    -void SynchConsole::WriteDone(): just V's the write semaphore to indicate that the write is complete any
     allows SynchConsole::WriteChar(char c) to release the lock and terminate.




    ------------Our implementation for managing open files is as follows-------------
    implementation for these classes can be found in userprog/filetable.cc, filetable.h

    TableEntry: Object that stores a reference to an OpenFile and a reference count of the amount of 
    references in the system to that OpenFile.

    Table: Maintains an array of TableEntries and provides an interface for Reading, Writing, and 
    Clearing TableEntries. The TableEntry array is indexed by OpenFileId, meaning OpenFileId's act 
    essentially as Unix file descriptors.

    OpenFileTable: A system wide table which provides an interface to all processes for managing 
    OpenFile objects by using a Table object to maintain open files. A process can store a new 
    OpenFile object into the table by calling Insert(), passing in the OpenFile to store as an 
    argument. A process can access a particular OpenFile object by calling Resolve() with the 
    OpenFileId of the file. A process can remove its own reference to an OpenFile by calling Remove()
    with the OpenFileId of the file. *NOTE* 1) Remove will only remove an OpenFile object from the 
    OpenFileTable if its reference count is zero. 2) The first 2 slots in the table are reserved for 
    ConsoleInput and ConsoleOutput

    FileVector: a per process vector of OpenFileId's that maps OpenFileId's for a single process to 
    the global system wide OpenFileTable. In order to create a new reference to an OpenFile, the 
    FileVector first inserts the OpenFile into the global file table, saves the OpenFileId of its 
    position in the global file table, inserts that id into the local vector, and then returns the 
    index of that id as the OpenFileId to access the file. In order to gain access to an OpenFile 
    object through its OpenFileId, a process can call Resolve() and pass in the id; Resolve() will 
    use the passed in id as an index in order to get the global OpenFileId of the file in the system-
    wide OpenFileTable, use the global id to get the OpenFile object from the global file table, and 
    return the reference to the file. Finally, a process can close/remove its reference to an 
    OpenFile object by calling Remove() with the id; Remove() will decrement the reference count of 
    the file in the global file table, removing it if the reference count is zero. In order to 
    provide file sharing between processes when calling Exec(), there are two constructors for a 
    FileVector. One constructor is used when there is no file sharing and simply constructs a 
    FileVector with only file descriptors pointing to ConsoleInput and ConsoleOutput. The other 
    constructor takes in the parent processes' FileVector and copies all references over into the 
    new FileVector object and also increments the reference count to the corresponding files in the 
    global file table. **NOTE** by using the constructor for sharing file descriptors, we ensure that 
    seperate opens of the same file do not result in a shared shared file descriptor. 
    (alsosee representation of FileVector and OpenFileTable relationship down below)

                        FileVector                              OpenFileTable
                      ________________                        _________________
    local file id => | global file id | => global file id => | OpenFile object |
                     |----------------|                      |-----------------|
                     | other file id  |                      | other file obj  |
                     |----------------|                      |-----------------|
                     |                |                      |                 |
                     |----------------|                      |-----------------|

    

    -----------------our implementation for managing processes is as follows-------------------------
    implementation for these classes can be found in userprog/processlist.cc, processlist.h

    ***NOTE*** we modified the List and SynchList classes to provide SortedFind methods which take a sortkey and 
    return the object in the list with that sortkey value


    ProcessEntry: Object that stores the Space Id of a process and the exit status for that process. 
    Also contains a binary Semaphore which is initially set as 0 and then set to 1 when a process 
    sets its exit status.

    ProcessList: A global Synchlist of ProcessEntries that are sorted based by process SpaceId's.When a new process is 
    created, a new ProcessEntry is Inserted() into the list sorted by SpaceId.
    When a process calls a Join() for a specific SpaceId of a process, the process calls GetStatus()
    on the ProcessList and passes in the SpaceId of the process it wishes to wait for; GetStatus() 
    will find the ProcessEntry corressponding to the passed in SpaceId and will first wait on the 
    semaphore for that Entry until signaled and then return the exit status of the process.
    When a process Exits(), it sets its status that is stored in the ProcessEntry with the same 
    SpaceId and also signals the semaphore for the Entry, allowing other processes to read the exit 
    status at that Entry.

    


    -------------------our changes to the AddrSpace class are as follows----------------------------

    Added Fields:   

        "pageMap": an integer array that contains the mapping from the AddrSpace objects' virtual page numbers to 
        physical page frames. Index in the array corresponds to a virtual page number, and the value at an index 
        corresponds to page frame number.

        "id": a SpaceId identifying the process number that the addrspace object is hooked up to.

        "success": a boolean flag. If true, the addrspace object was built successfully. If false, an error occurred 
        while building the addrspace.

        "fileVector": A FileVector object which holds the OpenFileId's of all openfiles that this
        addrspace object has access to.

    Added/Changed Methods:
        - various getter and setter methods added for newly added private fields

        int V2P(int vAddr): Provides virtual to physical address translation. Takes in a virtual address, calculates 
        the virtual page number and offset in the page using the system PageSize, uses the virtual page number to 
        index into the pageMap to find the physical page frame.

        Changed AddrSpace constructor: Constructor now assignes a specific SpaceId corresponding to the process number 
        it is hooked up to. It Also will not try to run binaries that are too big for memory. Constructor uses the 
        bitmap to create the mapping from virtual pages to physical page frames. For each virtual page, the bitmap is 
        queried to find the next free page frame and then that frame is stored in the page map.

        void AddrSpace::SaveState(): changed so that it invalidates all of the TLB entries so that it will be flushed 
        when the next executing process runs.







    --------------------Our implementation for the system calls is as follows-----------------------

    Helper functions added:
        - char *LoadStringFromMemory(int vAddr): copies string from user-land into kernal-land. Accepts a virtual 
        address "vAddr", uses the method V2P() to translate the virtual address into a physical one, then grabs the 
        string out of the machine's main memory and copies it into a buffer, which is then forcibly null-terminated 
        and returned. If the string is longer than the defined "MAX_STRING_LENGTH" of 128, we truncate the string.

        -void SaveStringToMemory(char* buffer, int numRead, int vAddr): Saves a string from kernel-space to user-
        space. Converts the given virtual address of where to store the string to a physical using V2P() and writes 
        the data from the buffer into the machine's main memory for the given number of bytes.

        -void StartProcess(int args): initializes currentThread's registers and restores the state. Casts and sets our 
        vector of argument strings from the parameter "int args" to a char** named "argStrings". Calculate "argc" for 
        this new process based on the number of entries in "argStrings". Determine address location of the stack 
        pointer "sp" by reading the register "StackReg". Construct an array that will contain the virtual address 
        locations of our arguments. We then iterate over the arguments, calculate the length of the argument string 
        and move the stack pointer "sp" down by that amount, then store the string in main memory one character at a 
        time by translating the stack pointer's virtual address and the individual character in the argument's offset 
        to a physical memory location using V2P(). After the string is fully copied into main memory, we store the 
        current stack pointer location in argvAddr. We then offset the stack pointer and then store argc and the 
        current location of the stack pointer in registers and start the new process by calling "machine->Run()"

        -void HandleTLBFault(int vaddr): called on TLB fault. Not necessarily a page fault, the page may be in memory. 
        Determine virtual page number "vpn" by dividing the virtual address by the page size. Try to find a free slot, 
        and if located we will be modifying that slot. Otherwise, clobber a random TLB slot. We changed so that if a 
        process makes a bad memory reference (i.e. passes in a virtual address outside of the processes address space) 
        then we force the process to Exit() gracefully with exit status 1.

        -void ExceptionHandler(ExceptionType which): entry point in the Nachos kernel. Called when a user program is 
        executing, and either does a syscall, or generates an addressing or arithmetic exception.  When the 
        ExceptionHandler is called, we read the type of exception out of register 2. Using a switch statement, we call 
        our appropriate syscall implementation, which are defined above (ex. SC_CREATE()). We also must increment our 
        PC to prevent looping in the same syscall.




    -void SC_CREATE(): our implementation for the Create() system call. Reads register 4 for the address of the 
    filename, then calls LoadStringFromMemory at that address to grab the filename string. We then use the fileSystem 
    defined Create() method to create the actual file.

    -int SC_OPEN(): our implementation for the Open() system call. Reads register 4 for the address of the filename. 
    We then use the fileSystem defined Open() method to get an OpenFile pointer to the file. If we have trouble 
    reading the filename or opening the file, we return -1. We also save the OpenFileId to the file in the processes's 
    FileVector object.

    -void SC_CLOSE(): our implementation for the Close() system call. A Process reads the OpenFileId from register 4 
    then attempts to close the file by removing the reference to the file by calling Remove() on the its addrspaces' 
    fileVector.

    -void SC_WRITE(): our implementation for the Write() system call. Accepts string buffer of characters to write. If 
    the buffer is NULL, its a no-op. We also determine how many bytes to write by reading from register 5, and the 
    OpenFileId to write into by reading from register 6. If the id is ConsoleOutput, we write "size" amount of bytes 
    to the Console using synchConsole. Else, we resolve the OpenFileId to an OpenFile object using the process's 
    fileVector. If the user passes in a bad OpenFileID, we do nothing. Otherwise, we call Write() on the OpenFile 
    object to write in "size" amount of bytes from buffer to that OpenFile object. *NOTE* around the write operations 
    for writing to both ConsoleOutput and File objects, the "ioLock" is Acquire()-ed and Release()-ed in order to make 
    Write()'s atomic.

    -int SC_READ(): our implementation for the Read() system call. Accepts a buffer to read characters into and an 
    OpenFileId of the file we want to read from. If we are reading from "ConsoleInput", then we read from the Console 
    using the "ReadChar()" method of "synchConsole". Else, we resolve the OpenFileId to an OpenFile object using the 
    process's fileVector. We then call Read() on the file object and save the buffer using "SaveStringToMemory()" back 
    into user-space and return the number of bytes read. *NOTE* we return -1 on error

    -SpaceId SC_EXEC(): our implementation for the Exec() system call. Loads the filename, the virtual address of the 
    start of the first argument (calles "argVectorBase"), and share flag from the registers. Create a new OpenFile 
    object for the executable using the fileSystem's "Open()" method with the filename. Then pull in all of the 
    argument strings from user-space. The physical address of the starting virtual address of the next argument 
    strings are in offsets of 4 bytes from "argVectorBase". We then grab the virtual address of the start of the 
    argument string from main memory by indexing into main memory and using macho type-casting. We then load the 
    argument from user-space to kernel-space using "LoadStringFromMemory". We then build a new AddrSpace and Thread 
    object for the new process. If the shareflag is set as non zero, we build the FileVector object by passing in the 
    parent's FileVector object to the constructer, else we build a blank FileVector. We save the SpaceId of the new 
    process to return it to the parent. We create a new entry in the ProcessList for the new process and then Fork() 
    off the new process by passing in "StartProcess()" and the vector of argument strings cast as an int. ***NOTE*** 
    we impose a limitation of 11 arguments

    -int SC_JOIN(): our exception handler for the Join() system call. Reads the pid of the process for the Join() from 
    the registers, and gets the exit status of that process from the ProcessList by calling "GetStatus()" on the 
    processList, then returns the status of the join.

    -int SC_EXIT(): our exception handler for the Exit() system call. Reads the exit status for the process from the 
    register, and stores that status in the ProcessList by calling "SetStatus()" on the processList. We then close all 
    of the open files that the process has open and kill the thread with "Finish()"


    ----------------------Our testing is as follows-----------------------------------------

    All tests seem to pass.

    NOTE: all tests we ran by "cp"'ing the tests and Makefile for each set of tests into our test directory and then 
    running those tests from the "test" directory.

    DOUBLE NOTE: Some of the tests run correctly but do not halt the system after they are finished, so you must
    control-c out of them
 
    TRIPLE NOTE: We believe the test "deepexec" runs correctly but outputs confusing output because Blocks of
    Write() calls aren't atomic (only single Write() calls are). Therefore, we explain our output to 
    "deepexe":

    PARENT exists
    PARENT after exec; kid pid is KID4 after exec; kid5 pid is 1
    2
    PARENT about to Join kid
    KID4 about to Join kid5
    KID4 off Join with value of 5
    PARENT off Join with value of 4



    we believe that "kid pid is" and "KID4 after exec; kid5 pid is" are inter spliced because the parent and child
    in deepexec both print out a string, then an int. so it really is "kid pid is 1; kid5 pid is 2" but because
    blocks of Write()'s arent't atomit, this is not so





    ------------------our implementation of cat and cp are as follows----------------
    invoke shell by cd'ing into nachos/test and doing the following:
        ../userprog/nachos -x shell
    This will execute the shell.

    The two shell commands our shell supports are "cat" and "cp".

    **** cat ****

    "cat" can be invoked in the following ways (note, the "--" is just the shell prefix):
        --cat
        --cat <somefile>
        --cat <inputfile> <outputfile>

    Invoking cat without any file argument models the Unix terminal: any input typed into the console by the user will 
    be echo'd back to the user once a newline character is entered. The only way to exit this mode is by entering 
    CTRL+C, which will generate a  SIGINT and bring down the whole machine. We use i to denote whether content has 
    been entered to the console, and if there is anything to echo back. We read one character at a time and store it 
    in a buffer (up to 60 chars) once a newline character is entered. We then write one character at a time out of the 
    buffer and into ConsoleOutput, clearing the buffer with null terminators as we iterate.

    Invoking cat with one arg, a proper filename, will output the contents of that file to the console. We read one 
    char at a time, and while our Read()'s are valid (returns > 0), we write that one char back to ConsoleOutput.

    Invoking cat with two files copies the contents of inputfile to output file. In this way, it mimics the 
    functionality of "cp" (see below), but the key distinction is that cat will NOT create the outputfile if it does 
    not already exist. Sidenote: we Exec() cat from cp, which is part of the reason why this functionality exists.

    Errors:
    Invoking cat with one arg, an improper filenam, will generate the following error:
        Error: Invalid input file
    Invoking cat with two args and an improper inputfile will generate the following error:
        Error: Invalid input file
    Invoking cat with two args and an improper inputfile will generate the following error:
        Error: Invalid output file



    **** cp ****

    "cp" can be invoked in the following ways (note, the "--" is just the shell prefix):
        --cp <inputfile> <outputfile>

    This will copy the contents of inputfile into outputfile. Note, this will erase/override whatever was previously 
    in outputfile and creates outputfile if it does not already exist. inputfile can also be the same file as 
    outputfile...although that is not particularly useful functionality.

    It is also worth noting that we Exec() cat with two args from cp for the copying logic/functionality.

    Errors:
    Invoking cp with the wrong number of arguments will generate the following error:
        Invalid number of arguments
    Invoking cp with a bad inputfile will generate the following error:
        Error: Invalid input file
