/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/LLVMTarget.h"

#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/SubtargetFeature.h>


#include <memory>
#include <optional>
#include <string>


namespace jitcat::LLVM
{
	class LLVMTypes;
	//This class contains all the target-specific information that is needed by LLVM for code generation,
	//both for JIT compilation and pre-compilation.
	class LLVMTargetConfig
	{
	public:
		LLVMTargetConfig(bool isJITTarget, bool sretBeforeThis, bool useThisCall, bool callerDestroysTemporaryArguments, 
						 bool enableSymbolSearchWorkaround, bool is64BitTarget, unsigned int sizeOfBoolInBits, 
						 unsigned int defaultLLVMCallingConvention, const std::string& targetTripple, const std::string& cpuName,
						 std::string objectFileExtension, const llvm::TargetOptions& targetOptions, const llvm::SubtargetFeatures& subtargetFeatures, 
						 llvm::CodeGenOpt::Level optimizationLevel,
						 llvm::Optional<llvm::Reloc::Model> relocationModel = llvm::Optional<llvm::Reloc::Model>(),
						 llvm::Optional<llvm::CodeModel::Model> codeModel = llvm::Optional<llvm::CodeModel::Model>());

		LLVMTargetConfig(const LLVMTargetConfig&) = delete;
		LLVMTargetConfig& operator=(const LLVMTargetConfig&) = delete;

		~LLVMTargetConfig();
		
		static std::unique_ptr<LLVMTargetConfig> createJITTargetConfig();
		static std::unique_ptr<LLVMTargetConfig> createConfigForPreconfiguredTarget(LLVMTarget target);

		llvm::TargetMachine& getTargetMachine() const;
		const llvm::DataLayout& getDataLayout() const;

		llvm::Expected<const llvm::orc::JITTargetMachineBuilder&> getTargetMachineBuilder() const;

		const LLVMTypes& getLLVMTypes() const;

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
		
		//The default calling convention to use when generating code for a function call.
		//May be overridden by the X86_ThisCall calling convention if useThisCall is true.
		//Must be one of the calling conventions defined by llvm in llvm::CallingConv.
		const unsigned int defaultLLVMCallingConvention;

	private:
		std::unique_ptr<LLVMTypes> llvmTypes;
		//The target tripple that identifies CPU architecture, OS and compiler compatibility
		const std::string targetTripple;
		//The name of the target CPU
		const std::string cpuName;
		//The file extension for object files that are generated when pre-compiling
		const std::string objectFileExtension;

		//Contains all the target specific information for the machine that we are compiling for. Among other things, the target CPU type.
		std::unique_ptr<llvm::TargetMachine> targetMachine;
		//Specifies the layout of structs and the type of name mangling used based on the target machine as well as endianness.
		std::unique_ptr<const llvm::DataLayout> dataLayout;

		//Target specific options
		const llvm::TargetOptions targetOptions;
		const llvm::SubtargetFeatures subtargetFeatures;
		const llvm::CodeGenOpt::Level optimizationLevel;
		const llvm::Optional<llvm::Reloc::Model> relocationModel;
		const llvm::Optional<llvm::CodeModel::Model> codeModel;

		llvm::Optional<llvm::orc::JITTargetMachineBuilder> targetMachineBuilder;
	};
};