

#ifdef CHANGED

#include "filetable.h"
#include "system.h"

//-----------------------------------------------------------
// TableEntry::TableEntry()
//	create a new TableEntry object. Stores a reference to the
//	OpenFile and a reference count to that file
//-----------------------------------------------------------
TableEntry::TableEntry(OpenFile *f) {
	file = f;
	refCount = 1;
}

//-----------------------------------------------------------
// TableEntry::~TableEntry()
//	destructor
//-----------------------------------------------------------
TableEntry::~TableEntry() {
	delete file;
}

//-----------------------------------------------------------
// TableEntry::GetFile()
//	accessor to get at OpenFile
//-----------------------------------------------------------
OpenFile *
TableEntry::GetFile() {
	return file;
}

//-----------------------------------------------------------
// TableEntry::GetCount()
//	return reference count
//-----------------------------------------------------------
int
TableEntry::GetCount() {
	return refCount;
}

//-----------------------------------------------------------
// TableEntry::IncrementCount()
//	increment reference count
//-----------------------------------------------------------
void
TableEntry::IncrementCount() {
	++refCount;
	return ;
}

//-----------------------------------------------------------
// TableEntry::DecrementCount()
//	decrement reference count
//-----------------------------------------------------------
void
TableEntry::DecrementCount() {
	--refCount;
	return ;
}




//-----------------------------------------------------------
// Table::Table()
//	create a new Table object. Initialize array of OpenFile objects
//-----------------------------------------------------------
Table::Table() {
	length = MAX_OPEN_FILE_ID;
	table = new(std::nothrow) TableEntry*[length];			// can only have up to MAX_OPEN_FILE_ID OpenFiles
	// ClearTable();
}

//-----------------------------------------------------------
// Table::~Table()
//	destructor
//-----------------------------------------------------------
Table::~Table() {
	delete [] table;
}

//-----------------------------------------------------------
// Table::ReadEntry()
//	Return the Openfile object that maps to the passed in OpenFileId
//	NOTE: id must be in the allowed range
//-----------------------------------------------------------
TableEntry *
Table::ReadEntry(OpenFileId id) {
	ASSERT(id >= 0 && id < MAX_OPEN_FILE_ID);
	return table[id]; 
}

//-----------------------------------------------------------
// Table::WriteEntry()
//	Write the Openfile object to the passed in OpenFileId.
//	NOTE: id must be in the allowed range
//-----------------------------------------------------------
void
Table::WriteEntry(OpenFileId id, OpenFile *f) {
	ASSERT(id >= 0 && id < MAX_OPEN_FILE_ID);
	table[id] = new(std::nothrow) TableEntry(f);
	return ;
}

//-----------------------------------------------------------
// Table::ClearEntry()
//	deletes an entry in the table
//-----------------------------------------------------------
void
Table::ClearEntry(OpenFileId id) {
	ASSERT(id >= 0 && id < MAX_OPEN_FILE_ID);
	TableEntry *entry = table[id];
	table[id] = NULL;
	delete entry;
	return ;
}

//-----------------------------------------------------------
// Table::ClearTable()
//	iterate over Table and clear each entry
//-----------------------------------------------------------
void
Table::ClearTable() {
	for(int i = 0; i < Length(); ++i)
		ClearEntry(i);
}

//-----------------------------------------------------------
// Table::Length()
//	Return the length of the table
//-----------------------------------------------------------
int 
Table::Length() {
	return length;
}



//-----------------------------------------------------------
// OpenFileTable::OpenFileTable()
//	create a new OpenFileTable object
//-----------------------------------------------------------
OpenFileTable::OpenFileTable() {
	fileTable = new(std::nothrow) Table();		// create a new Table to keep track of OpenFiles
	fileTable->WriteEntry(ConsoleInput, NULL);	// mark "taken" consoleinput/output
	fileTable->WriteEntry(ConsoleOutput, NULL);
}

//-----------------------------------------------------------
// OpenFileTable::~OpenFileTable()
//	destructor
//-----------------------------------------------------------
OpenFileTable::~OpenFileTable() {
	delete fileTable;
}

//-----------------------------------------------------------
// OpenFileTable::Insert()
//	Iterate over the Table and find the first open slot.
//	Insert the Openfile into that slot and return the OpenFileId
//	for that spot. If there are no free spots, return -1
//-----------------------------------------------------------
OpenFileId
OpenFileTable::Insert(OpenFile *f) {
	for(int i = 0; i < fileTable->Length(); ++i) {		// iterate over fileTable, looking for a free spot
		if(fileTable->ReadEntry(i) == NULL) {			// found free spot, add OpenFile as new Entry there
			fileTable->WriteEntry(i, f);				// insert the Openfile into the free spot
			return i;
		}
	}

	return -1;
}

//-----------------------------------------------------------
// OpenFileTable::Resolve()
//	check if OpenFileId is valid and return the Openfile object
//	at that index
//-----------------------------------------------------------
OpenFile *
OpenFileTable::Resolve(OpenFileId id) {
	if(id < 0 || id >= fileTable->Length())			// safety checks. invalid file id
		return NULL;

	TableEntry *entry = fileTable->ReadEntry(id);					// get the TableEntry that corresponds to the OpenfileId
	if(entry == NULL)		// safety check, Entry does not exist
		return NULL;
	
	return entry->GetFile();		// return the Openfile object at the Entry
}

//-----------------------------------------------------------
// OpenFileTable::Remove()
//	Decrement reference count to the Openfile object that
// corresponds to the passed in OpenfileId. If no more references,
// remove the file
//-----------------------------------------------------------
void
OpenFileTable::Remove(OpenFileId id) {
	if(id < 0 || id >= fileTable->Length())			// safety check, invalid file id
		return ;

	TableEntry *entry = fileTable->ReadEntry(id);		// get the TableEntry
	if(entry == NULL)	// safety check, entry does not exist
		return;

	entry->DecrementCount();			// decrement reference count of tableentry
	if(entry->GetCount() <= 0)			// if no more references to OpenFile, remove file from table
		fileTable->ClearEntry(id);		// clears entry in fileTable

	return ;
}

//-----------------------------------------------------------
// OpenFileTable::AddReference()
//	Increment reference count of the passed in OpenFileId
//-----------------------------------------------------------
void
OpenFileTable::AddReference(OpenFileId id) {
	if(id < 0 || id >= fileTable->Length())			// invalid file id
		return ;

	TableEntry *entry = fileTable->ReadEntry(id);			// get TableEntry
	if(entry == NULL)		// entry does not exist
		return;

	entry->IncrementCount();			// increment reference count of OpenFile object
}


//######################################################################################


//-----------------------------------------------------------
// FileVector::FileVector()
//	constructor for a per process FileVector with no sharing
//-----------------------------------------------------------
FileVector::FileVector() {
	length = MAX_OPEN_FILE_ID;
	idVector = new(std::nothrow) OpenFileId[length];

	idVector[0] = ConsoleInput;						// set descriptor 0 to be ConsoleInput
	idVector[1] = ConsoleOutput;					// set descriptor 1 to be ConsoleOutput
	globalFileTable->AddReference(ConsoleInput);		// Add reference to ConsoleInput
	globalFileTable->AddReference(ConsoleOutput);		// add reference to ConsoleOutput

	for(int i = 2; i < length; ++i)					// iterate over vector, settting each value to FREE
		idVector[i] = FREE_SPACE;
}

//-----------------------------------------------------------
// FileVector::FileVector(FileVector *parentVector)
//	constructor for a per process FileVector with sharing.
//	Takes in the FileVector of the parent process and copies
// everything.
//-----------------------------------------------------------
FileVector::FileVector(FileVector *parentVector) {
	length = MAX_OPEN_FILE_ID;
	idVector = new(std::nothrow) OpenFileId[length];
	for(int i = 0; i < parentVector->length; ++i) {		// iterate over parent vector and copy all references
		idVector[i] = parentVector->idVector[i];
		globalFileTable->AddReference(idVector[i]);		// increment reference count of each openfileid
	}
}

//-----------------------------------------------------------
// FileVector::~FileVector()
//	close all references in the vector
//-----------------------------------------------------------
FileVector::~FileVector() {
	for(int i = 0; i < length; ++i)
		globalFileTable->Remove(idVector[i]);

	delete[] idVector;
}

//-----------------------------------------------------------
// FileVector::Insert()
//	Creates new file descriptor to the passed in OpenFile
//-----------------------------------------------------------
OpenFileId
FileVector::Insert(OpenFile *f) {
	OpenFileId globalId = globalFileTable->Insert(f);		// insert the OpenFile into the system-wide OpenFileTable and save the corresponding id
	
	OpenFileId localId = -1;				// set as -1 initially in case it is not found
	for(int i = 2; i < length; ++i) {			// iterate over file vector, looking for a free spot
		if(idVector[i] == FREE_SPACE) {			// found free slot
			idVector[i] = globalId;				// save global file descriptor
			localId = i;						// save local file descriptor
			break;
		}
	}

	return localId;
}

//-----------------------------------------------------------
// FileVector::Resolve()
//	Returns the Openfile object associated with the passed in
// OpenFileId
//-----------------------------------------------------------
OpenFile *
FileVector::Resolve(OpenFileId id) {
	if(id < 0 || id >= length)			// safety check, invalid id
		return NULL;

	return globalFileTable->Resolve(idVector[id]);		// resolve the id
}

//-----------------------------------------------------------
// FileVector::Remove()
//	Remove reference to the file pointed to by the passed in id
//-----------------------------------------------------------
void
FileVector::Remove(OpenFileId id) {
	if(id < 0 || id >= length)			// safety check, invalid file
		return ;

	globalFileTable->Remove(idVector[id]);		// remove reference from system-wide file table
	idVector[id] = FREE_SPACE;					// clear spot in local file vector
}

#endif // CHANGED