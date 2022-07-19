/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/LLVMTarget.h"

#include <memory>
#include <optional>
#include <string>


namespace jitcat::LLVM
{
	struct LLVMTargetConfigOptions;
	class LLVMTypes;
	//This class contains all the target-specific information that is needed by LLVM for code generation,
	//both for JIT compilation and pre-compilation.
	class LLVMTargetConfig
	{
	public:
		LLVMTargetConfig(bool isJITTarget, bool sretBeforeThis, bool useThisCall, bool callerDestroysTemporaryArguments, 
						 bool enableSymbolSearchWorkaround, bool is64BitTarget, unsigned int sizeOfBoolInBits, 
						 std::string objectFileExtension, std::unique_ptr<LLVMTargetConfigOptions> llvmOptions);

		LLVMTargetConfig(const LLVMTargetConfig&) = delete;
		LLVMTargetConfig& operator=(const LLVMTargetConfig&) = delete;

		~LLVMTargetConfig();
		
		static std::unique_ptr<LLVMTargetConfig> createJITTargetConfig();
		static std::unique_ptr<LLVMTargetConfig> createConfigForPreconfiguredTarget(LLVMTarget target);

		llvm::TargetMachine& getTargetMachine() const;
		const llvm::DataLayout& getDataLayout() const;

		llvm::orc::JITTargetMachineBuilder* getTargetMachineBuilder() const;

		const LLVMTypes& getLLVMTypes() const;

		const LLVMTargetConfigOptions& getOptions() const;

	private:
		static std::unique_ptr<LLVMTargetConfig> createTargetConfigForCurrentMachine(bool isJITTarget);
		static std::unique_ptr<LLVMTargetConfig> createGenericWindowsx64Target();
		static std::unique_ptr<LLVMTargetConfig> createXboxOneTarget();
		static std::unique_ptr<LLVMTargetConfig> createPS4Target();

	public:
		//Wether or not JIT compilation is supported on the target
		const bool isJITTarget;
		
		//64 bit target if true, 32 bit target if false.
		const bool is64BitTarget;
		
		//Determines the ordering of the 'this' argument and the 'sret' argument in a member function.
		//Sret is used when a function returns a structure by value. Om windows/msvc a class member funtion's
		//first argument will be the 'this' pointer and the second argument will be the sret pointer if applicable.
		//On linux/clang/gcc this is the other way around.
		const bool sretBeforeThis;

		//Determines who is responsible for destroying temporary argument values that are passed to a function.
		//In the Windows/MSVC ABI, the callee is responsible for destroying temporaries.
		//On Linux/GCC/Clang, which use the Itanium C++ ABI, the caller is responsible.
		const bool callerDestroysTemporaryArguments;

		//Sets the calling convention for member function calls. Similar to the sretBeforeThis option.
		const bool useThisCall;

		//Enable some workarounds on Windows/MSVC required for finding function symbols after their code has been generated.
		const bool enableSymbolSearchWorkaround;
		
		//Size, int bits, of a boolean.
		const unsigned int sizeOfBoolInBits;
		
		//The file extension for object files that are generated when pre-compiling
		const std::string objectFileExtension;

	private:
		std::unique_ptr<LLVMTargetConfigOptions> llvmOptions;
	};
};