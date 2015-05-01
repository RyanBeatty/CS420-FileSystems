# CS420-FileSystems
NACHOS File System implementation
Project 2

----My Implementation of Caching File Blocks is as follows----

############################################
Changes Made to synchdisk.h and synchdisk.cc
############################################

* Added CacheEntry class which is used to store an entry in the Cache. has data members "valid", "sector", and "block". "valid" is a boolean stating if the current Entry is valid and in use (meaning the current file block has not been changed from a write operation), "sector" is the sector number that the CacheEntry is holding the data of, and "block" is a character array which holds the actual data on the file block.

* Added Cache class which operates on an array of CacheEntry objects to implement the cache. The Cache Class has public methods Get(), Add(), Delete(), and inCache(). Get() takes the sector number of the file block you are trying to get and returns NULL if it is not in the cache or a character array of the contents of the file block. Add() takes a char * of the file block data and the sector number where it is stored on disk and adds it as an entry to the cache, replacing a previous entry if the cache is full. Delete() takes a sector number and invalidates the CacheEntry with the same sector number or is a no op if the sector number isn't in the cache. inCache() returns true if the sector number is in the cache and false if not. At the moment, the Cache's size is set to 10.

* Added CopySector(char *dst, char *src) method which copies SectorSize number of bytes from "src" to "dst"

* Added a Cache object to the SynchDisk class

* Added a lock to the SynchDisk class to synchronize access to the Cache object

* Changed SynchDisk::ReadSector() so that it uses the Cache. First Acquire() the cacheLock then check if the current sector requested is in the Cache. If it is in the Cache, copy the block from the Cache over to "data", release the cacheLock and return. If it is not in the Cache, Add() it to the Cache after the request for the sector has been serviced.

* Changed SynchDisk::WriteSector() to use the Cache. It now will start out by Acquiring the cacheLock and Deleting the requested sector to read from the Cache. 


----My Implementatino of Directories is as follows----

############################
Changes Made to directory.cc
############################

* Added boolean field "isDir" to the DirectoryEntry class. If true, then the DirecotryEnty is a Directory else it is a regular file.

* Created method AddDirectory() which pretty much does the same thing as Add() except it sets the new DirectoryEntry's "isDir" field to true, signaling the entry is a directory

* Changed Add() to set "isDir" field to false because we are only adding a file to the filesystem

* Added isDirectory(char *name) which returns true if "name" is a Directory (the corresponding DirectoryEntry's "isDir" field is true) and false if it is a file or does not exist in the current directory

* Changed List() so that it will recursively print out the names of all files, directories, and files in subdirectories. It also now takes a parameter "int tabs" which specifies how many tabs to print out before printing the name of the current Directory Entry (This is so that when printing out the home directory it is easy to tell which files fall under which directories). List iteratores over the directory entry table and prints out each name for each entry. If the entry is a Directory (and is not the "." or ".." Directories), instaniate a new Directory object and fetch it from disk and then call List() on the Directory to print out its directory entry table.

##########################
Changes Made to filesys.cc
##########################

* Added function parse_path(char **path, int wdSector) which takes a pointer to the path string that a user has specified to a file or directory and the sector of the current working directory that the user is in as parameters. Returns -1 if there is a bad path or the sector which the desired file or directory to be reached is in. parse_path also modifies "path" such that after the method finishes, path will be the name of the file or directory the user wanted to reach. For example, if the path is "hello/world/foo" and "world" is not a directory within the "hello" directory, then it will return -1 else it will return the sector of the "world" directory and "path" will be "foo". parse_path works by spliting the "path" string into seperate strings based on the "/" delimiter and checking if every string but the last string is a Directory.

* Added MakeDir() which does pretty much the same thing as Create() except it adds a Directory to the filesystem instead of a file. Also the new Directory will by default have the Directories "." and ".." as the first two entries in its directory entry table which specify the current directory and the parent directory respectively.

* I've changed Create(), Open(), MakeDir(), ChangeDir(), Remove(), so that they all now take in an extra parameter "wdSector" that is the sector of the current working directory of the user. Also they all now use parse_path() to parse the path name the user passes to them to implement relative and absolute paths.

* Can Remove files from subdirectories but cannot Remove directories at all

############################
Changes Made to addrspace.cc
############################

* added new field called "wdSector" which is the sector number of the current working directory of the AddrSpace object. This is passed to the FileSystem object when doing any operations on the filesystem.

############################
Changes Made to exception.cc
############################

* Added a new system call MakeDir which allows userland processes to make a new directory in the file system.

#############
Misc. Changes
#############

* Changed fstest.cc to compile with the new parameters that all of the FileSystem's methods take now.

* Running ./nachos -l will now print out all of the files and directories in the file system as well as where they are in the hierarchy.

* running ./nachos -md [dirname] will now create the directory [dirname] in the nachos file systems. [dirname] may also be a relative path.


----My Implementation of VM is as follows----

* I had to make ReadAt and WriteAt thread safe so that VM would work. I added a global lock called "diskLock" which is acquired at the start of each method and then released at the end. In ReadAt, you only acquire and release "diskLock" if the current thread does not currently have the lock.

* VM is now a Nachos file on the Nachos filesystem. processes read from and write to the file when paging in and out pages of memory.

* Added global OpenFile object called "vmFile" that acts as an interface for reading and writing to and from the VM file.

* When the FileSystem object is constructed at system start up, I first try to Remove the VM file from the filesystem (if it isn't there, Remove will just return false and not crash) and then I create a new VM file and Open it, setting "vmFile" to the OpenFile object that is returned by Open.

* I changed my HandleTLBFault in exception.cc to use "vmFile" instead of "synchDisk" to page in and out pages. It now uses WriteAt and ReadAt to write and read sectors of the VM.






----Testing-----

#########
rmakedir.c
#########

* Tests that a userland process can create a new directory and create and open a file within that directory by using a relative path for the file.

files to cp over:
/test/rmakedir

to run:
./nachos -x rmakedir
./nachos -l 		// print filesystem

*****output****
finished removing file
created directory
test passed

finished removing file
VM
rmakedir
test 					// created directory
	.
	..
	foo 				// created file in directory


##########
amakedir.c
##########

* Tests that a userland process can create a new directory, change directories into that directory, and make a file within the new directory

files to cp over:
/test/amakedir

to run:
./nachos -x amakedir
./nachos -l 			// print filesystem

****output****
finished removing file
created directory
changed into directory
test passed


finished removing file
VM
amakedir
test
        .
        ..
        foo





#########
matmult.c
#########

* matmult from the third nachos project that is designed to stress VM.

files to cp over:
/test/matmult

to run:
./nachos -x matmult

****output****
finished removing file
Starting matmult
Initialization Complete
i = 0
i = 1
i = 2
i = 3
i = 4
i = 5
i = 6
i = 7
i = 8
i = 9
i = 10
i = 11
i = 12
i = 13
i = 14
i = 15
i = 16
i = 17
i = 18
i = 19
i = 20
i = 21
i = 22
i = 23
i = 24
i = 25
i = 26
i = 27
i = 28
i = 29
C[29,29] = 25230
Machine halting!




#########
vmtorture
#########

* really stressed VM. runs a bunch of Xkids, qmatmult, and qsort.

files to cp over:
/test/vmtorture
/test/qmatmult
/test/qsort
/test/Xkid

to run:
./nachos -x vmtorture

****output****
finished removing file
VMTORTURE beginning
QMATMULT
QSORT
Console kids starting
ABCDEBCAEDBCAEDCBEADBCDAECDBAEBCDEACBDEABDCEABDCAEBDCAEBDAECBAEDCBAEDCBAEDCBDAECBADCEBADCEBADCEBACDEABDCEADBCEADBCEADBCEADCBEADCBEACDBEACDBEADCBEADCBEACDBEADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBACDEBADCEBACDEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCEBADCBEADCEBADCBEADCEBADCBEADCBEADCEBADCEBADCEBDACEBDACEBDACEBDCAEBDACEBADCEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEBDACEB

Kids done

Awaiting qmatmult
qmatmult Exit value is 25230
Awaiting qsort
qsort Exit value is 1

VMTORTURE terminating normally
Machine halting!

Ticks: total 9693998162, idle 9497361838, system 150987080, user 45649244
Disk I/O: reads 657228, writes 131505
Console I/O: reads 0, writes 686
TLB: misses 5161231
Paging: faults 131281



#################
Performance Tests
#################


# Without Caching
finished removing file
Starting file system performance test:
Ticks: total 632940, idle 628860, system 4080, user 0
Disk I/O: reads 98, writes 25
Console I/O: reads 0, writes 0
TLB: misses 0
Paging: faults 0
Network I/O: packets received 0, sent 0
Sequential write of 500 byte file, in 10 byte chunks
Sequential read of 500 byte file, in 10 byte chunks
finished removing file
Ticks: total 1941050, idle 1916980, system 24070, user 0
Disk I/O: reads 617, writes 91
Console I/O: reads 0, writes 0
TLB: misses 0
Paging: faults 0
Network I/O: packets received 0, sent 0

# with Caching
finished removing file
Starting file system performance test:
Ticks: total 612980, idle 607280, system 5700, user 0
Disk I/O: reads 42, writes 25 									// fewer reads
Console I/O: reads 0, writes 0
TLB: misses 0
Paging: faults 0
Network I/O: packets received 0, sent 0
Sequential write of 500 byte file, in 10 byte chunks
Sequential read of 500 byte file, in 10 byte chunks
finished removing file
Ticks: total 1813050, idle 1787030, system 26020, user 0
Disk I/O: reads 126, writes 91 									// notice alot less reads
Console I/O: reads 0, writes 0
TLB: misses 0
Paging: faults 0
Network I/O: packets received 0, sent 0

