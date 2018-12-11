#pragma once 

#include <string>
#include <vector>
#include <Windows.h>

//This class parses the windows executable format.
//It can parse an executable already resident in memory (loaded) or it can load it from a file.
class PCExecutable
{
private:
	struct PEDataDirectory
	{
		unsigned int directoryIndex;
		_IMAGE_SECTION_HEADER* section;
		void* addressOfData;
		IMAGE_DATA_DIRECTORY* dataDirectory;
	};
	struct ImportedDLLFunction
	{
		bool importByName;		//if false: import by ordinal
		std::string name;			//invalid if import by ordinal
		unsigned short hint;	//hint if import by name
		unsigned short ordinal;	//Ordinal if import by ordinal
		unsigned int* bitField;	//Pointer to the entry in the actual lookup table
		void* functionAddress;	//If the executable is resident in memory, this points to the actual function
	};
	struct ImportedDLL
	{
		std::string dllName;
		_IMAGE_IMPORT_DESCRIPTOR* descriptor;
		std::vector<ImportedDLLFunction*> importLookupTable;
	};
	
public:
	//Read from the current memory resident process
	PCExecutable();

	~PCExecutable();

	void printExecutableInfoDump();

private:
	void read();
	void readDirectoryInfo(PEDataDirectory* directory);
	void readExportDirectory(PEDataDirectory* importDirectory);
	void readImportDirectory(PEDataDirectory* importDirectory);
	void* getAddressOfDataDirectory(_IMAGE_DATA_DIRECTORY* dataDirectory, unsigned int& sectionIndex) const;
	std::string getStringFromFixedSizeCharArray(const char* charArray, unsigned int length) const;
	bool containsFlag(unsigned int bits, unsigned int flag) const;
	std::string getMachineName(unsigned short identifier) const;
	void printImageCharacteristics() const;
	void printSectionCharacteristics(_IMAGE_SECTION_HEADER* section) const;
	std::string getDataDirectoryName(unsigned int identifier) const;
	void printIfContainsFlag(unsigned int bits, unsigned int flag, 
							 const char* messageIfContainsFlag, 
							 const char* messageIfNotContainsFlag) const;
	void printImports() const;
private:
	//Either the start of executable file data, or the base address of a process loaded in virtual memory
	void* baseAddress;
	//True if the executable is read from memory, false if read from file
	bool isMemoryResident;
	//The dos header
	_IMAGE_DOS_HEADER* dosHeader;
	//The windows image file header
	IMAGE_FILE_HEADER* imageHeader;
	//The (not so) optional header of the PE file format
	_IMAGE_OPTIONAL_HEADER64* optionalHeader;
	//Array of sections (.text, .rdata, etc.)
	std::vector<_IMAGE_SECTION_HEADER*> sections;
	//Array of data directories
	std::vector<PEDataDirectory*> directories;
	//Header for the import section
	std::vector<ImportedDLL*> imports;
};