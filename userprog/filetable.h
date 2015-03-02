#ifdef CHANGED
#ifndef FILETABLE_H
#define FILETABLE_H
//------------------------------------------------------------------------------------------
// Defines data structures needed to manage references to open files and file descriptors
//
//	OpenFileIdTable:
//
//		- an system-wide array of OpenFile objects that can be accessed by processes for
//		  reading and writing
//
//	Table:
//		- A simple List of Entries containing references to OpenFileObjects.
//
//	TableEntry:
//		- an Entry that has an OpenFile pointer and reference count of the amount of
//		  references that exist to that OpenFile
//		  (NOTE) probably should have made this a generic class
//
//	FileVector:
//		- a per process vector that maps OpenFileId's for a single process
// 		  to the global system wide OpenFileTable 
//

#include "syscall.h"
#include "openfile.h"
#include <new>
#define MAX_OPEN_FILE_ID 100 // defines the max number of OpenFileId's a process can have
#define FREE_SPACE -1


// stores an OpenFile object and a reference count of all
// file descriptors to that file
class TableEntry {
	public:
		TableEntry(OpenFile *f);
		~TableEntry();

		OpenFile *GetFile();
		// void SetFile(OpenFile *f);

		int GetCount();
		void IncrementCount();
		void DecrementCount();

	private:
		OpenFile *file;
		int refCount;
};

// List of OpenFiles
class Table {
	public:
		Table();
		~Table();

		void ClearTable();				// Clear list of all open files

		TableEntry *ReadEntry(OpenFileId id);
		void WriteEntry(OpenFileId id, OpenFile *f);
		void ClearEntry(OpenFileId id);

		int Length();
	private:
		TableEntry **table;
		int length;
};

// System-wide table of all open files. 
class OpenFileTable {
	public:
		OpenFileTable();
		~OpenFileTable();

		OpenFileId Insert(OpenFile *f);		// insert the OpenFile f into the OpenFile Table as a new entry and return
											// the corresponding OpenFileId

		OpenFile *Resolve(OpenFileId id);	// resolve the passed in OpenFileId to the OpenFile object it points to
		void Remove(OpenFileId id);				// frees the OpenFileId number from the list

		void AddReference(OpenFileId id);		// increment reference of the filedescriptor


	private:
		Table *fileTable;	// array of OpenFile objects

};


// a per process vector that maps OpenFileId's for a single process
// to the global system wide OpenFileTable
class FileVector {
	public:

		FileVector();
		FileVector(FileVector *parentVector);		// constructor used when sharing file descriptors when Exec()-ing
		~FileVector();

		OpenFileId Insert(OpenFile *f);		// creates new file descriptor to an OpenFile
		OpenFile *Resolve(OpenFileId id);	// resolve the passed in OpenFileId to the OpenFile object it points to
		void Remove(OpenFileId id);			// decrement reference to Openfile

	private:
		OpenFileId *idVector;			// list of OpenfileId's that correspond to Openfiles in the system-wide file table
		int length;						// length of per process file vector
};

#endif // FILETABLE_H
#endif // CHANGED