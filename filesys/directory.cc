#ifdef CHANGED
// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include <new>

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new(std::nothrow) DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
	table[i].inUse = false;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    // printf("fetch start\n");
    // char *str = (char *) calloc(16, sizeof(char));
    // snprintf(str, 16, "%d", tableSize);

    file->Seek(0);
    file->Read((char *) &tableSize, sizeof(int));
    file->Read((char *) table, tableSize * sizeof(DirectoryEntry));
    file->Seek(0);

    // printf("fetch finished\n");
    // tableSize = strtol(str, (char **) NULL, 10);
    // free(str);
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    // printf("writeback table size: %d\n", tableSize);

    // char *str = (char *) calloc(16, sizeof(char));
    // snprintf(str, 16, "%d", tableSize);

    file->Seek(0);                          // make sure we are at beggining of directory
    file->Write((char *) &tableSize, sizeof(int));
    file->Write((char *) table, tableSize * sizeof(DirectoryEntry));    // for exstensible files
    file->Seek(0);

    // printf("write back finished\n");
    fflush(stdout);
    // free(str);
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    // printf("find index print\n");
    // for(int i = 0; i < tableSize; ++i) {
    //     printf("entry: %s\n", table[i].name);
    // }
    // printf("end find index print\n");
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
	    return i;
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    // printf("table size: %d\n", tableSize);
    int i = FindIndex(name);

    if (i != -1)
	return table[i].sector;
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return true if successful;
//	return false if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
    if (FindIndex(name) != -1)
	return false;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = true;
            strncpy(table[i].name, name, FileNameMaxLen); 
            table[i].sector = newSector;
        return true;
	}

    DirectoryEntry *newTable = new(std::nothrow) DirectoryEntry[tableSize * 2];
    for(int i = 0; i < tableSize * 2; ++i) {
        if(i < tableSize)
            newTable[i] = table[i];
        else
            newTable[i].inUse = false;
    }
    delete[] table;
    table = newTable;
    tableSize *= 2;

    // for(int i = 0; i < tableSize; ++i) {
    //     printf("entry: %s\n", table[i].name);
    // }
    for (int i = 0; i < tableSize; i++) {
        if (!table[i].inUse) {
            table[i].inUse = true;
            strncpy(table[i].name, name, FileNameMaxLen); 
            table[i].sector = newSector;
            printf("return true in ADD\n");
            return true;
        }
    }

    ASSERT(false);  // should not happen
    return false;	// no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return true if successful;
//	return false if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return false; 		// name not in directory
    table[i].inUse = false;
    return true;	
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
   for (int i = 0; i < tableSize; i++)
	if (table[i].inUse)
	    printf("%s\n", table[i].name);
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new(std::nothrow) FileHeader();

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
	    printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
	}
    printf("\n");
    delete hdr;
}










#else












// directory.cc 
//  Routines to manage a directory of file names.
//
//  The directory is a table of fixed length entries; each
//  entry represents a single file, and contains the file name,
//  and the location of the file header on disk.  The fixed size
//  of each directory entry means that we have the restriction
//  of a fixed maximum size for file names.
//
//  The constructor initializes an empty directory of a certain size;
//  we use ReadFrom/WriteBack to fetch the contents of the directory
//  from disk, and to write back any modifications back to disk.
//
//  Also, this implementation has the restriction that the size
//  of the directory cannot expand.  In other words, once all the
//  entries in the directory are used, no more files can be created.
//  Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include <new>

//----------------------------------------------------------------------
// Directory::Directory
//  Initialize a directory; initially, the directory is completely
//  empty.  If the disk is being formatted, an empty directory
//  is all we need, but otherwise, we need to call FetchFrom in order
//  to initialize it from disk.
//
//  "size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new(std::nothrow) DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
    table[i].inUse = false;
}

//----------------------------------------------------------------------
// Directory::~Directory
//  De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
//  Read the contents of the directory from disk.
//
//  "file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
//  Write any modifications to the directory back to disk
//
//  "file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
//  Look up file name in directory, and return its location in the table of
//  directory entries.  Return -1 if the name isn't in the directory.
//
//  "name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
        return i;
    return -1;      // name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
//  Look up file name in directory, and return the disk sector number
//  where the file's header is stored. Return -1 if the name isn't 
//  in the directory.
//
//  "name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
    return table[i].sector;
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
//  Add a file into the directory.  Return true if successful;
//  return false if the file name is already in the directory, or if
//  the directory is completely full, and has no more space for
//  additional file names.
//
//  "name" -- the name of the file being added
//  "newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
    if (FindIndex(name) != -1)
    return false;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = true;
            strncpy(table[i].name, name, FileNameMaxLen); 
            table[i].sector = newSector;
        return true;
    }
    return false;   // no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
//  Remove a file name from the directory.  Return true if successful;
//  return false if the file isn't in the directory. 
//
//  "name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
    return false;       // name not in directory
    table[i].inUse = false;
    return true;    
}

//----------------------------------------------------------------------
// Directory::List
//  List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
   for (int i = 0; i < tableSize; i++)
    if (table[i].inUse)
        printf("%s\n", table[i].name);
}

//----------------------------------------------------------------------
// Directory::Print
//  List all the file names in the directory, their FileHeader locations,
//  and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new(std::nothrow) FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
    if (table[i].inUse) {
        printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
        hdr->FetchFrom(table[i].sector);
        hdr->Print();
    }
    printf("\n");
    delete hdr;
}


#endif
