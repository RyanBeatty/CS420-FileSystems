# CS420-FileSystems
NACHOS File System implementation
Project 2

----My Implementation of VM is as follows----

* I had to make ReadAt and WriteAt thread safe so that VM would work. I added a global lock called "diskLock" which is acquired at the start of each method and then released at the end. In ReadAt, you only acquire and release "diskLock" if the current thread does not currently have the lock.

* VM is now a Nachos file on the Nachos filesystem. processes read from and write to the file when paging in and out pages of memory.

* Added global OpenFile object called "vmFile" that acts as an interface for reading and writing to and from the VM file.

* When the FileSystem object is constructed at system start up, I first try to Remove the VM file from the filesystem (if it isn't there, Remove will just return false and not crash) and then I create a new VM file and Open it, setting "vmFile" to the OpenFile object that is returned by Open.

* I changed my HandleTLBFault in exception.cc to use "vmFile" instead of "synchDisk" to page in and out pages. It now uses WriteAt and ReadAt to write and read sectors of the VM.

