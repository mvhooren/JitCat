#pragma once


struct LLVMCompileOptions
{
	LLVMCompileOptions(): enableDereferenceNullChecks(false) 
	{}
	bool enableDereferenceNullChecks;
};
