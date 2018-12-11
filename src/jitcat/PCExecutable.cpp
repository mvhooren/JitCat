#include "PCExecutable.h"
#include "Tools.h"

#include <iostream>
#include <fstream>

//QQQ delete
PCExecutable::PCExecutable():
	isMemoryResident(true)
{
	HMODULE moduleHandle = GetModuleHandle(nullptr);
	baseAddress = reinterpret_cast<void*>(moduleHandle);
	read();
}


PCExecutable::~PCExecutable()
{
	if (!isMemoryResident)
	{
		delete baseAddress;
	}
	Tools::deleteElements(directories);
	for (unsigned int i = 0; i < imports.size(); i++)
	{
		Tools::deleteElements(imports[i]->importLookupTable);
	}
	Tools::deleteElements(imports);
}


void PCExecutable::printExecutableInfoDump()
{
	std::cout << "\n";
	std::cout << "Module handle / base address:" << baseAddress << "\n";
	std::cout << "\n";
	std::cout << "DOS header...\n";
	std::cout << "Magic characters:" << getStringFromFixedSizeCharArray(reinterpret_cast<char*>(&dosHeader->e_magic), 2) << "\n"; //MZ
	std::cout << "Relocations:"<< dosHeader->e_crlc << "\n"; 
	std::cout << "Relocations table address:"<< dosHeader->e_lfarlc << "\n"; 
	std::cout << "Exe file header address:"<< dosHeader->e_lfanew << "\n"; 
	std::cout << "\n";
	std::cout << "Image header..." << "\n"; 
	std::cout << "Image magic characters:"<< getStringFromFixedSizeCharArray(reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(imageHeader) - 4), 4) << "\n";  //PE00
	std::cout << "Machine:"<< getMachineName(imageHeader->Machine) << "\n"; 
	std::cout << "Number of sections:"<< imageHeader->NumberOfSections << "\n"; 
	std::cout << "Number of symbols:"<< imageHeader->NumberOfSymbols << "\n"; 
	std::cout << "Pointer to symbol table:"<< imageHeader->PointerToSymbolTable << "\n"; 
	std::cout << "Characteristics:" << "\n"; 
	printImageCharacteristics();
	std::cout << "Size of optional header:"<< imageHeader->SizeOfOptionalHeader << "\n"; 
	std::cout << "" << "\n"; 
	std::cout << "PE header.." << "\n"; 
	std::cout << "PE magic characters:"<< getStringFromFixedSizeCharArray(reinterpret_cast<char*>(&optionalHeader->Magic), 2) << "\n"; 
	std::cout << "Major linker version:"<< (unsigned int)optionalHeader->MajorLinkerVersion << "\n"; 
	std::cout << "Minor linker version:"<< (unsigned int)optionalHeader->MinorLinkerVersion << "\n"; 
	std::cout << "Major image version:"<< optionalHeader->MajorImageVersion << "\n"; 
	std::cout << "Minor image version:"<< optionalHeader->MinorImageVersion << "\n"; 
	std::cout << "Size of headers:"<< optionalHeader->SizeOfHeaders << "\n"; 
	std::cout << "Size of executable code:"<< optionalHeader->SizeOfCode << "\n"; 
	std::cout << "Size of initialized data:"<< optionalHeader->SizeOfInitializedData << "\n"; 
	std::cout << "Size of uninitialized data:"<< optionalHeader->SizeOfUninitializedData << "\n"; 
	std::cout << "Address of entry point:"<< static_cast<unsigned long>(optionalHeader->AddressOfEntryPoint) << "\n"; 
	std::cout << "Offset to code (.text) section:"<< static_cast<unsigned long>(optionalHeader->BaseOfCode) << "\n"; 
	//std::cout << "Offset to unitialized (.bss) section:"<< reinterpret_cast<uint32_t>(optionalHeader->BaseOfData) << "\n"; 
	std::cout << "Preferred base address:"<< static_cast<unsigned long long>(optionalHeader->ImageBase)<< "Actual base address:"<< baseAddress << "\n"; 
	std::cout << "Number of RVA:"<< optionalHeader->NumberOfRvaAndSizes << "\n"; 

	for (unsigned int i = 0; i < directories.size(); ++i)
	{
		std::cout << "Directory:\t\t"<< getDataDirectoryName(directories[i]->directoryIndex) << "\n"; 
		std::cout << "\tVirtual address:"<< static_cast<unsigned long>(directories[i]->dataDirectory->VirtualAddress) << "\n"; 
		std::cout << "\tActual address:\t"<< directories[i]->addressOfData << "\n"; 
		std::cout << "\tLocated in section:"<< getStringFromFixedSizeCharArray(reinterpret_cast<char*>(directories[i]->section->Name), IMAGE_SIZEOF_SHORT_NAME) << "\n"; 
		std::cout << "\tSize:\t\t"<< optionalHeader->DataDirectory[i].Size << "\n"; 
	}
	std::cout << "\n"; 

	std::cout << "Section headers..." << "\n"; 
	for (unsigned int i = 0; i < sections.size(); i++)
	{
		std::cout <<  getStringFromFixedSizeCharArray(reinterpret_cast<char*>(sections[i]->Name), IMAGE_SIZEOF_SHORT_NAME) << "\n"; 
		std::cout << "\tVirtual address:"<< static_cast<unsigned long>(sections[i]->VirtualAddress) << "\n"; 
		std::cout << "\tSize of raw data:"<< static_cast<unsigned long>(sections[i]->SizeOfRawData) << "\n"; 
		std::cout << "\tPointer to raw data:"<< static_cast<unsigned long>(sections[i]->PointerToRawData) << "\n"; 
		std::cout << "\tCharacteristics:" << "\n"; 
		printSectionCharacteristics(sections[i]);
	}
	std::cout << "" << "\n"; 

	std::cout << "Imports..." << "\n"; 
	printImports();
	std::cout << "" << "\n"; 
}


void PCExecutable::read()
{
	dosHeader = reinterpret_cast<_IMAGE_DOS_HEADER*>(baseAddress);
	uintptr_t imageHeaderAddress = reinterpret_cast<uintptr_t>(baseAddress) + dosHeader->e_lfanew;
	imageHeader = reinterpret_cast<IMAGE_FILE_HEADER*>(imageHeaderAddress + 4);
	uintptr_t optionalHeaderAddress = imageHeaderAddress + IMAGE_SIZEOF_FILE_HEADER + 4;
	optionalHeader = reinterpret_cast<_IMAGE_OPTIONAL_HEADER64*>(optionalHeaderAddress);
	uintptr_t sectionArrayStartAddress = optionalHeaderAddress + sizeof(_IMAGE_OPTIONAL_HEADER);
	_IMAGE_SECTION_HEADER* headerArray = reinterpret_cast<_IMAGE_SECTION_HEADER*>(sectionArrayStartAddress);
	for (unsigned int i = 0; i < imageHeader->NumberOfSections; i++)
	{
		sections.push_back(&headerArray[i]);
	}
	for (unsigned int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; ++i)
	{
		if (optionalHeader->DataDirectory[i].VirtualAddress != 0)
		{
			PEDataDirectory* dataDirectory = new PEDataDirectory();
			dataDirectory->dataDirectory = &optionalHeader->DataDirectory[i];
			dataDirectory->directoryIndex = i;
			unsigned int sectionIndex = 0;
			dataDirectory->addressOfData = getAddressOfDataDirectory(dataDirectory->dataDirectory, sectionIndex);
			dataDirectory->section = sections[sectionIndex];
			directories.push_back(dataDirectory);
			readDirectoryInfo(dataDirectory);
		}
	}
}


void PCExecutable::readDirectoryInfo(PEDataDirectory* directory)
{
	switch (directory->directoryIndex)
	{
		case IMAGE_DIRECTORY_ENTRY_EXPORT:
			readExportDirectory(directory);
			break;
		case IMAGE_DIRECTORY_ENTRY_IMPORT:
			readImportDirectory(directory);
			break;
	}
}


void PCExecutable::readExportDirectory(PEDataDirectory* exportDirectory)
{
	_IMAGE_EXPORT_DIRECTORY* exportsInfo = reinterpret_cast<_IMAGE_EXPORT_DIRECTORY*>(exportDirectory->addressOfData);
	std::cout << "Exports, nr of functions: " << exportsInfo->NumberOfFunctions << " nr of names: " << exportsInfo->NumberOfNames << "\n";
}


void PCExecutable::readImportDirectory(PEDataDirectory* importDirectory)
{
	_IMAGE_IMPORT_DESCRIPTOR* importsArray = reinterpret_cast<_IMAGE_IMPORT_DESCRIPTOR*>(importDirectory->addressOfData);
	unsigned int importIndex = 0;
	while (true)
	{
		_IMAGE_IMPORT_DESCRIPTOR* import = &importsArray[importIndex];
		if (import->OriginalFirstThunk == 0)
		{
			//This is an empty import to signify the end of the list
			break;
		}
		ImportedDLL* importedDLL = new ImportedDLL();
		importedDLL->descriptor = import;
		importedDLL->dllName = std::string(reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(baseAddress) + import->Name));
		uintptr_t* importLookupArray = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(baseAddress) + import->OriginalFirstThunk);
		uintptr_t* importAddressArray = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(baseAddress) + import->FirstThunk);
		unsigned int functionIndex = 0;
		while (true)
		{
			if (importLookupArray[functionIndex] == 0)
			{
				//A value of 0 signifies the end of the lookup table
				break;
			}
			ImportedDLLFunction* function = new ImportedDLLFunction();
			function->bitField = 0;//&importLookupArray[functionIndex];
			function->importByName = (importLookupArray[functionIndex] & IMAGE_ORDINAL_FLAG64) == 0;
			if (function->importByName)
			{
				function->hint = *reinterpret_cast<unsigned short*>(reinterpret_cast<uintptr_t>(baseAddress) + importLookupArray[functionIndex]);
				function->name = std::string(reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(baseAddress) + importLookupArray[functionIndex] + 2));
				function->ordinal = 0;
			}
			else
			{
				function->hint = 0;
				function->name = "";
				function->ordinal = importLookupArray[functionIndex] ^ IMAGE_ORDINAL_FLAG64;
			}
			if (isMemoryResident)
			{
				function->functionAddress = reinterpret_cast<void*>(importAddressArray[functionIndex]);
			}
			else
			{
				function->functionAddress = 0;
			}
			importedDLL->importLookupTable.push_back(function);
			functionIndex++;
		}

		imports.push_back(importedDLL);
		importIndex++;
	}
}


void* PCExecutable::getAddressOfDataDirectory(_IMAGE_DATA_DIRECTORY* dataDirectory, unsigned int& sectionIndex) const
{
	_IMAGE_SECTION_HEADER* sectionContainingDirectory = nullptr;
	sectionIndex = sections.size() + 1;
	for (unsigned int i = 0; i < sections.size(); i++)
	{
		if (dataDirectory->VirtualAddress >= sections[i]->VirtualAddress
			&& dataDirectory->VirtualAddress < sections[i]->VirtualAddress + sections[i]->SizeOfRawData)
		{
			sectionContainingDirectory = sections[i];
			sectionIndex = i;
			break;
		}
	}
	if (isMemoryResident)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(baseAddress) + dataDirectory->VirtualAddress);
	}
	else
	{
		if (sectionContainingDirectory != nullptr)
		{
			uintptr_t directoryAddress = reinterpret_cast<uintptr_t>(baseAddress) + 
											(dataDirectory->VirtualAddress - sectionContainingDirectory->VirtualAddress)
											+ sectionContainingDirectory->PointerToRawData;
			return reinterpret_cast<void*>(directoryAddress);
		}
		else
		{
			return nullptr;
		}
	}
}


std::string PCExecutable::getStringFromFixedSizeCharArray(const char* charArray, unsigned int length) const
{
	char* chars = new char[length + 1];
	//make sure the string is 0 terminated
	chars[length] = 0;
	memcpy(chars, charArray, length);
	std::string convertedString(chars);
	delete[] chars;
	return convertedString;
}


bool PCExecutable::containsFlag(unsigned int bits, unsigned int flag) const
{
	return (bits & flag) != 0;
}


std::string PCExecutable::getMachineName(unsigned short identifier) const
{
	switch (identifier)
	{
		case IMAGE_FILE_MACHINE_UNKNOWN:
			return "Unknown";
		default:
			return "Other";
		case IMAGE_FILE_MACHINE_I386:
			return "Intel x86";
		case IMAGE_FILE_MACHINE_POWERPC:
			return "IBM PowerPC Little-endian";
		case IMAGE_FILE_MACHINE_IA64:
			return "Intel 64";
		case IMAGE_FILE_MACHINE_AMD64:
			return "AMD64 (K8)";
	}
}


void PCExecutable::printImageCharacteristics() const
{
	unsigned int characteristics = imageHeader->Characteristics;
	printIfContainsFlag(characteristics, IMAGE_FILE_RELOCS_STRIPPED, "\tRelocation info stripped from file.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_EXECUTABLE_IMAGE, "\tFile is executable  (i.e. no unresolved externel references).", "\tFile is NOT executable.");
	printIfContainsFlag(characteristics, IMAGE_FILE_LINE_NUMS_STRIPPED, "\tLine nunbers stripped from file.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_LOCAL_SYMS_STRIPPED, "\tLocal symbols stripped from file.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_AGGRESIVE_WS_TRIM, "\tAgressively trim working set.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_LARGE_ADDRESS_AWARE, "\tApplication can handle >2gb addresses.", "\tApplicatio CANNOT handle >2gb addresses.");
	printIfContainsFlag(characteristics, IMAGE_FILE_BYTES_REVERSED_LO, "\tBytes of machine word are reversed.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_32BIT_MACHINE, "\t32 bit word machine.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_DEBUG_STRIPPED, "\tDebugging info stripped from file in .DBG file", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP, "\tIf Image is on removable media, copy and run from the swap file.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_SYSTEM, "\tSystem File.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_DLL, "\tFile is a DLL.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_UP_SYSTEM_ONLY, "\tFile should only be run on a UP machine.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_FILE_BYTES_REVERSED_HI, "\tBytes of machine word are reversed.", nullptr);	
}


void PCExecutable::printSectionCharacteristics(_IMAGE_SECTION_HEADER* section) const
{
	unsigned int characteristics = section->Characteristics;
	printIfContainsFlag(characteristics, IMAGE_SCN_CNT_CODE, "\t\tSection contains code.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_CNT_INITIALIZED_DATA, "\t\tSection contains initialized data.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_CNT_UNINITIALIZED_DATA, "\t\tSection contains uninitialized data.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_LNK_OTHER, "\t\tOther (reserved).", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_LNK_INFO, "\t\tSection contains comments or some other type of information.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_LNK_REMOVE, "\t\tSection contents will not become part of image.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_LNK_COMDAT, "\t\tSection contents comdat.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_NO_DEFER_SPEC_EXC, "\t\tReset speculative exceptions handling bits in the TLB entries for this section.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_GPREL, "\t\tSection content can be accessed relative to GP.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_LNK_NRELOC_OVFL, "\t\tSection contains extended relocations.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_MEM_DISCARDABLE, "\t\tSection can be discarded.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_MEM_NOT_CACHED, "\t\tSection is not cachable.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_MEM_NOT_PAGED, "\t\tSection is not pageable.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_MEM_SHARED, "\t\tSection is shareable.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_MEM_EXECUTE, "\t\tSection is executable.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_MEM_READ, "\t\tSection is readable.", nullptr);
	printIfContainsFlag(characteristics, IMAGE_SCN_MEM_WRITE, "\t\tSection is writeable.", nullptr);
}


std::string PCExecutable::getDataDirectoryName(unsigned int identifier) const
{
	switch (identifier)
	{
		case IMAGE_DIRECTORY_ENTRY_EXPORT:
			return "Export Directory";
		case IMAGE_DIRECTORY_ENTRY_IMPORT:
			return "Import Directory";
		case IMAGE_DIRECTORY_ENTRY_RESOURCE:
			return "Resource Directory";
		case IMAGE_DIRECTORY_ENTRY_EXCEPTION:
			return "Exception Directory";
		case IMAGE_DIRECTORY_ENTRY_SECURITY:
			return "Security Directory";
		case IMAGE_DIRECTORY_ENTRY_BASERELOC:
			return "Base Relocation Table";
		case IMAGE_DIRECTORY_ENTRY_DEBUG:
			return "Debug Directory";
		case IMAGE_DIRECTORY_ENTRY_ARCHITECTURE:
			return "Architecture Specific Data";
		case IMAGE_DIRECTORY_ENTRY_GLOBALPTR:
			return "RVA of global pointer";
		case IMAGE_DIRECTORY_ENTRY_TLS:
			return "Thread Local Storage directory";
		case IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:
			return "Load Configuration Directory";
		case IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT:
			return "Bound Import Directory in headers";
		case IMAGE_DIRECTORY_ENTRY_IAT:
			return "Import Address Table";
		case IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:
			return "Delay Load Import Descriptors";
		case IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR:
			return "COM Runtime descriptor";
		default:
			return "Not used";
	}
}


void PCExecutable::printIfContainsFlag(unsigned int bits, unsigned int flag, 
										const char* messageIfContainsFlag, 
										const char* messageIfNotContainsFlag) const
{
	bool flagFound = containsFlag(bits, flag);
	if (flagFound && messageIfContainsFlag != nullptr)
	{
		std::cout << messageIfContainsFlag  << "\n";
	}
	else if (!flagFound && messageIfNotContainsFlag != nullptr)
	{
		std::cout << messageIfNotContainsFlag  << "\n";
	}
}


void PCExecutable::printImports() const
{
	for (unsigned int i = 0; i < imports.size(); i++)
	{
		std::cout << imports[i]->dllName  << "\n";
		for (unsigned int j = 0; j < imports[i]->importLookupTable.size(); j++)
		{
			if (imports[i]->importLookupTable[j]->importByName)
			{
				if (!isMemoryResident)
				{
					std::cout << "\tBy name, hint: " << imports[i]->importLookupTable[j]->hint << " name: " << imports[i]->importLookupTable[j]->name << "\n";
				}
				else
				{
					std::cout << "\tBy name, address: " << imports[i]->importLookupTable[j]->functionAddress << " hint: " << imports[i]->importLookupTable[j]->hint << " name: " << imports[i]->importLookupTable[j]->name << "\n";
				}
			}
			else
			{
				if (!isMemoryResident)
				{
					std::cout << "\tBy ordinal: " << imports[i]->importLookupTable[j]->ordinal << "\n";
				}
				else
				{
					std::cout << "\tBy ordinal, address: " << imports[i]->importLookupTable[j]->functionAddress << " ordinal: " << imports[i]->importLookupTable[j]->ordinal << "\n";
				}
			}
		}
	}
}