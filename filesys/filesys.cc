#ifdef CHANGED

// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"
#include "system.h"
#include <string>
#include <new>

#define VM_FILE_SIZE (1024 * 50) + 100

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)

OpenFile* freeMapFile;   // Bit map of free disk blocks,
          // represented as a file
OpenFile* directoryFile;   // "Root" directory -- list of 
          // file names, represented as a file

int parse_path(char **path, int wdSector) {
    std::string cur_path(*path), dirname;
    std::string::size_type i;
    while((i = cur_path.find("/")) != std::string::npos) {
        dirname = cur_path.substr(0, i);
        cur_path = cur_path.substr(i+1, cur_path.size());

        Directory *dir = new(std::nothrow) Directory(NumDirEntries);
        OpenFile *dirFile = new(std::nothrow) OpenFile(wdSector);
        dir->FetchFrom(dirFile);
        if(!dir->isDirectory((char *) dirname.c_str()))
            return -1;
        wdSector = dir->Find((char *) dirname.c_str());
        delete dir;
        delete dirFile;
    }

    char *filename = new char[cur_path.size() + 1];
    std::copy(cur_path.begin(), cur_path.end(), filename);
    filename[cur_path.size()] = '\0';

    *path = filename;
    return wdSector;
}


//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = true, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = false, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
    DEBUG('f', "Initializing the file system.\n");
    if (format) {
        BitMap *freeMap = new(std::nothrow) BitMap(NumSectors);
        Directory *directory = new(std::nothrow) Directory(NumDirEntries);
	FileHeader *mapHdr = new(std::nothrow) FileHeader();
	FileHeader *dirHdr = new(std::nothrow) FileHeader();

        DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG('f', "Writing headers back to disk.\n");
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new(std::nothrow) OpenFile(FreeMapSector);
        directoryFile = new(std::nothrow) OpenFile(DirectorySector);
     
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
	freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	directory->WriteBack(directoryFile);

	if (DebugIsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();

        delete freeMap; 
	delete directory; 
	delete mapHdr; 
	delete dirHdr;
	}
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new(std::nothrow) OpenFile(FreeMapSector);
        directoryFile = new(std::nothrow) OpenFile(DirectorySector);
    }


    Remove("VM", DirectorySector);
    if(!Create("VM", VM_FILE_SIZE, DirectorySector)) {
        fprintf(stderr, "error: failed to create VM file\n");
        exit(1);
    }
    vmFile = Open("VM", DirectorySector);
    if(vmFile == NULL) {
        fprintf(stderr, "error: failed to open VM file\n");
        exit(1);
    }
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return true if everything goes ok, otherwise, return false.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(char *name, int initialSize, int wdSector)
{
    Directory *directory;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG('f', "Creating file %s, size %d\n", name, initialSize);

    directoryLock->Acquire();
    wdSector = parse_path(&name, wdSector);
    if(wdSector < 0) {
        DEBUG('f', "bad path: %s\n", name);
        directoryLock->Release();
        return false;
    }


    directory = new(std::nothrow) Directory(NumDirEntries);
    OpenFile *dirFile = new(std::nothrow) OpenFile(wdSector);
    directory->FetchFrom(dirFile);

    if (directory->Find(name) != -1)
      success = false;			// file is already in directory
    else {
        diskmapLock->Acquire();	
        freeMap = new(std::nothrow) BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();	// find a sector to hold the file header
    	if (sector == -1) 		
            success = false;		// no free block for file header 
        else if (!directory->Add(name, sector))
            success = false;	// no space in directory
	    else {
            ASSERT(directory->Find(name) != -1);
    	    hdr = new(std::nothrow) FileHeader();
	        if (!hdr->Allocate(freeMap, initialSize))
                success = false;	// no space on disk for data
	        else {	
	    	    success = true; // everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 		
    	    	directory->WriteBack(dirFile);
    	    	freeMap->WriteBack(freeMapFile);
	        }
            delete hdr;
	    }
        delete freeMap;
        diskmapLock->Release();
    }
    directoryLock->Release();
    delete directory;
    delete dirFile;
    return success;
}

bool
FileSystem::MakeDir(char *name, int initialSize, int wdSector)
{
    Directory *directory;
    OpenFile *dirFile;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG('f', "Creating file %s, size %d\n", name, initialSize);

    directoryLock->Acquire();
    wdSector = parse_path(&name, wdSector);
    if(wdSector < 0) {
        DEBUG('f', "bad path: %s\n", name);
        directoryLock->Release();
        return false;
    }

    directory = new(std::nothrow) Directory(NumDirEntries);
    dirFile = new(std::nothrow) OpenFile(wdSector);
    directory->FetchFrom(dirFile);

    if (directory->Find(name) != -1)
      success = false;          // file is already in directory
    else {
        diskmapLock->Acquire(); 
        freeMap = new(std::nothrow) BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();   // find a sector to hold the file header
        if (sector == -1)       
            success = false;        // no free block for file header 
        else if (!directory->AddDirectory(name, sector))
            success = false;    // no space in directory
        else {
            ASSERT(directory->Find(name) != -1);
            hdr = new(std::nothrow) FileHeader();
            if (!hdr->Allocate(freeMap, initialSize))
                success = false;    // no space on disk for data
            else {  
                success = true; // everthing worked, flush all changes back to disk
                hdr->WriteBack(sector);         
                directory->WriteBack(dirFile);
                freeMap->WriteBack(freeMapFile);

                Directory *newDir = new(std::nothrow) Directory(NumDirEntries);
                OpenFile *newFile = new(std::nothrow) OpenFile(sector);
                ASSERT(newDir->AddDirectory(".", sector));
                ASSERT(newDir->AddDirectory("..", wdSector));
                newDir->WriteBack(newFile);
                delete newDir;
                delete newFile;
            }
            delete hdr;
        }
        delete freeMap;
        diskmapLock->Release();
    }
    directoryLock->Release();
    delete directory;
    delete dirFile;
    return success;
}

int
FileSystem::ChangeDir(char *name, int wdSector) {
    Directory *directory;
    OpenFile *dirFile;
    int sector;

    directoryLock->Acquire();
    wdSector = parse_path(&name, wdSector);
    if(wdSector < 0) {
        DEBUG('f', "bad path: %s\n", name);
        directoryLock->Release();
        return -1;
    }


    dirFile = new(std::nothrow) OpenFile(wdSector);
    directory = new(std::nothrow) Directory(NumDirEntries);
    directory->FetchFrom(dirFile);

    if(!directory->isDirectory(name)) {
        DEBUG('f', "could not find directory %s\n", name);
        directoryLock->Release();
        delete directory;
        delete dirFile;
        return -1;
    }

    sector = directory->Find(name);

    directoryLock->Release();
    delete directory;
    delete dirFile;
    return sector;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(char *name, int wdSector)
{
    Directory *directory = new(std::nothrow) Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG('f', "Opening file %s\n", name);
    directoryLock->Acquire();
    wdSector = parse_path(&name, wdSector);
    if(wdSector < 0) {
        DEBUG('f', "bad path: %s\n", name);
        directoryLock->Release();
        return false;
    }

    OpenFile *dirFile = new(std::nothrow) OpenFile(wdSector);
    directory->FetchFrom(dirFile);
    sector = directory->Find(name); 
    if (sector >= 0) 		
	openFile = new(std::nothrow) OpenFile(sector);	// name was found in directory
    directoryLock->Release();
    delete directory;
    delete dirFile;
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return true if the file was deleted, false if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(char *name, int wdSector)
{ 
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    DEBUG('r', "starting filesystem remove\n");
    directoryLock->Acquire();
    wdSector = parse_path(&name, wdSector);
    if(wdSector < 0) {
        DEBUG('f', "bad path: %s\n", name);
        directoryLock->Release();
        return false;
    }

    OpenFile *dirFile = new(std::nothrow) OpenFile(wdSector);
    directory = new(std::nothrow) Directory(NumDirEntries);
    directory->FetchFrom(dirFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       delete dirFile;
       directoryLock->Release();
       return false;			 // file not found 
    }

    if(directory->isDirectory(name)) {
        DEBUG('f', "cannot Remove() directory\n");
        delete directory;
        delete dirFile;
        directoryLock->Release();
        return false;
    }

    fileHdr = new(std::nothrow) FileHeader();
    fileHdr->FetchFrom(sector);

    diskmapLock->Acquire();
    freeMap = new(std::nothrow) BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(dirFile);        // flush to disk
    directoryLock->Release();
    diskmapLock->Release();
    delete fileHdr;
    delete directory;
    delete dirFile;
    delete freeMap;

    printf("finished removing file\n");
    DEBUG('r', "finished removing file\n");
    return true;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List(int dirSector)
{
    OpenFile *dirFile = new(std::nothrow) OpenFile(dirSector);
    Directory *directory = new(std::nothrow) Directory(NumDirEntries);
    directoryLock->Acquire();
    directory->FetchFrom(dirFile);
    directory->List(0);
    directoryLock->Release();
    delete directory;
    delete dirFile;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new(std::nothrow) FileHeader();
    FileHeader *dirHdr = new(std::nothrow) FileHeader();
    BitMap *freeMap = new(std::nothrow) BitMap(NumSectors);
    Directory *directory = new(std::nothrow) Directory(NumDirEntries);

    printf("Bit map file header:\n");
    diskmapLock->Acquire();
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    directoryLock->Acquire();
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    directoryLock->Release();
    diskmapLock->Release();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
}

// getter method for freeMapFile
OpenFile *
GetFreeMapFile() {
    return freeMapFile;
}

//getter method for directoryFile
OpenFile *
GetDirectoryFile() {
    return directoryFile;
}







#else




















// filesys.cc 
//  Routines to manage the overall operation of the file system.
//  Implements routines to map from textual file names to files.
//
//  Each file in the file system has:
//     A file header, stored in a sector on disk 
//      (the size of the file header data structure is arranged
//      to be precisely the size of 1 disk sector)
//     A number of data blocks
//     An entry in the file system directory
//
//  The file system consists of several data structures:
//     A bitmap of free disk sectors (cf. bitmap.h)
//     A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//  files.  Their file headers are located in specific sectors
//  (sector 0 and sector 1), so that the file system can find them 
//  on bootup.
//
//  The file system assumes that the bitmap and directory files are
//  kept "open" continuously while Nachos is running.
//
//  For those operations (such as Create, Remove) that modify the
//  directory and/or bitmap, if the operation succeeds, the changes
//  are written immediately back to disk (the two files are kept
//  open during all this time).  If the operation fails, and we have
//  modified part of the directory and/or bitmap, we simply discard
//  the changed version, without writing it back to disk.
//
//  Our implementation at this point has the following restrictions:
//
//     there is no synchronization for concurrent accesses
//     files have a fixed size, set when the file is created
//     files cannot be bigger than about 3KB in size
//     there is no hierarchical directory structure, and only a limited
//       number of files can be added to the system
//     there is no attempt to make the system robust to failures
//      (if Nachos exits in the middle of an operation that modifies
//      the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"
#include <new>

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector       0
#define DirectorySector     1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize     (NumSectors / BitsInByte)
#define NumDirEntries       10
#define DirectoryFileSize   (sizeof(DirectoryEntry) * NumDirEntries)

//----------------------------------------------------------------------
// FileSystem::FileSystem
//  Initialize the file system.  If format = true, the disk has
//  nothing on it, and we need to initialize the disk to contain
//  an empty directory, and a bitmap of free sectors (with almost but
//  not all of the sectors marked as free).  
//
//  If format = false, we just have to open the files
//  representing the bitmap and the directory.
//
//  "format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
    DEBUG('f', "Initializing the file system.\n");
    if (format) {
        BitMap *freeMap = new(std::nothrow) BitMap(NumSectors);
        Directory *directory = new(std::nothrow) Directory(NumDirEntries);
    FileHeader *mapHdr = new(std::nothrow) FileHeader;
    FileHeader *dirHdr = new(std::nothrow) FileHeader;

        DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
    freeMap->Mark(FreeMapSector);       
    freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

    ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
    ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG('f', "Writing headers back to disk.\n");
    mapHdr->WriteBack(FreeMapSector);    
    dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new(std::nothrow) OpenFile(FreeMapSector);
        directoryFile = new(std::nothrow) OpenFile(DirectorySector);
     
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
    freeMap->WriteBack(freeMapFile);     // flush changes to disk
    directory->WriteBack(directoryFile);

    if (DebugIsEnabled('f')) {
        freeMap->Print();
        directory->Print();

        delete freeMap; 
    delete directory; 
    delete mapHdr; 
    delete dirHdr;
    }
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new(std::nothrow) OpenFile(FreeMapSector);
        directoryFile = new(std::nothrow) OpenFile(DirectorySector);
    }
}

//----------------------------------------------------------------------
// FileSystem::Create
//  Create a file in the Nachos file system (similar to UNIX create).
//  Since we can't increase the size of files dynamically, we have
//  to give Create the initial size of the file.
//
//  The steps to create a file are:
//    Make sure the file doesn't already exist
//        Allocate a sector for the file header
//    Allocate space on disk for the data blocks for the file
//    Add the name to the directory
//    Store the new file header on disk 
//    Flush the changes to the bitmap and the directory back to disk
//
//  Return true if everything goes ok, otherwise, return false.
//
//  Create fails if:
//          file is already in directory
//      no free space for file header
//      no free entry for file in directory
//      no free space for data blocks for the file 
//
//  Note that this implementation assumes there is no concurrent access
//  to the file system!
//
//  "name" -- name of file to be created
//  "initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(char *name, int initialSize)
{
    Directory *directory;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG('f', "Creating file %s, size %d\n", name, initialSize);

    directory = new(std::nothrow) Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1)
      success = false;          // file is already in directory
    else {  
        freeMap = new(std::nothrow) BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();   // find a sector to hold the file header
        if (sector == -1)       
            success = false;        // no free block for file header 
        else if (!directory->Add(name, sector))
            success = false;    // no space in directory
    else {
            hdr = new(std::nothrow) FileHeader;
        if (!hdr->Allocate(freeMap, initialSize))
                success = false;    // no space on disk for data
        else {  
            success = true;
        // everthing worked, flush all changes back to disk
                hdr->WriteBack(sector);         
                directory->WriteBack(directoryFile);
                freeMap->WriteBack(freeMapFile);
        }
            delete hdr;
    }
        delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
//  Open a file for reading and writing.  
//  To open a file:
//    Find the location of the file's header, using the directory 
//    Bring the header into memory
//
//  "name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(char *name)
{ 
    Directory *directory = new(std::nothrow) Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG('f', "Opening file %s\n", name);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); 
    if (sector >= 0)        
    openFile = new(std::nothrow) OpenFile(sector);  // name was found in directory 
    delete directory;
    return openFile;                // return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
//  Delete a file from the file system.  This requires:
//      Remove it from the directory
//      Delete the space for its header
//      Delete the space for its data blocks
//      Write changes to directory, bitmap back to disk
//
//  Return true if the file was deleted, false if the file wasn't
//  in the file system.
//
//  "name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(char *name)
{ 
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    directory = new(std::nothrow) Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return false;             // file not found 
    }
    fileHdr = new(std::nothrow) FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new(std::nothrow) BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);       // remove data blocks
    freeMap->Clear(sector);         // remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);        // flush to disk
    directory->WriteBack(directoryFile);        // flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    return true;
} 

//----------------------------------------------------------------------
// FileSystem::List
//  List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
    Directory *directory = new(std::nothrow) Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
//  Print everything about the file system:
//    the contents of the bitmap
//    the contents of the directory
//    for each file in the directory,
//        the contents of the file header
//        the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new(std::nothrow) FileHeader;
    FileHeader *dirHdr = new(std::nothrow) FileHeader;
    BitMap *freeMap = new(std::nothrow) BitMap(NumSectors);
    Directory *directory = new(std::nothrow) Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
} 



#endif
