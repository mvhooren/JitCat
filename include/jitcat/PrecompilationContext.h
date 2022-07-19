/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <string>

namespace jitcat
{
	class CatGenericType;
	class CatLib;
	class CatRuntimeContext;

	namespace AST
	{
		class CatAssignableExpression;
		class CatSourceFile;
		class CatTypedExpression;
	}

	//JitCat expressions can be precompiled to an object file.
	//To do this, create a PrecompilationContext and set it on a CatRuntimeContext before calling compile on an expression.
	//Every expression that gets compiled in this way will also be precompiled.
	//When all expressions have been compiled, call finishPrecompilation, which will emit the precompiled files.
	class PrecompilationContext
	{
	private:
		PrecompilationContext(const PrecompilationContext&) = delete;
		PrecompilationContext& operator=(const PrecompilationContext&) = delete;
	public:
		PrecompilationContext() {};
		virtual ~PrecompilationContext() {};

		virtual void precompileSourceFile(const jitcat::AST::CatSourceFile* sourceFile, jitcat::CatLib* catLib, CatRuntimeContext* context) = 0;
		virtual void precompileExpression(const AST::CatTypedExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context) = 0;
		virtual void precompileAssignmentExpression(const AST::CatAssignableExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context) = 0;

		//Call this when all expressions have been compiled that should be precompiled.
		//This will emit the precompiled files.
		virtual void finishPrecompilation() = 0;
	};

}