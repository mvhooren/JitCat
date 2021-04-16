/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatLib;
	class CatRuntimeContext;
	namespace AST
	{
		class CatClassDefinition;
		class CatFunctionDefinition;
		class CatScopeBlock;
	}
}
#include "jitcat/CatScopeID.h"
#include "jitcat/LLVMCompileOptions.h"
#include "jitcat/LLVMForwardDeclares.h"

#include <functional>
#include <map>
#include <vector>


namespace jitcat::LLVM
{
	class LLVMCodeGeneratorHelper;


	struct LLVMCompileTimeContext
	{
		LLVMCompileTimeContext(CatRuntimeContext* catContext, bool isPrecompilationContext);

		CatRuntimeContext* catContext;
		bool isPrecompilationContext;

		void clearState();

		CatLib* currentLib;
		const AST::CatClassDefinition* currentClass;
		const AST::CatScopeBlock* currentScope;
		const AST::CatFunctionDefinition* currentFunctionDefinition;
		llvm::Function* currentFunction;

		LLVMCodeGeneratorHelper* helper;
		std::vector<std::function<llvm::Value*()>> blockDestructorGenerators;
	
		LLVMCompileOptions options;

		std::map<CatScopeID, llvm::Value*> scopeValues;
	};

} //End namespace jitcat::LLVM