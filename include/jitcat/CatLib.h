/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Document.h"

#include <memory>
#include <string>
#include <vector>


namespace jitcat
{
	class CatGenericType;
	class ExpressionErrorManager;

	namespace AST
	{
		class CatSourceFile;
	}
	namespace Reflection
	{
		class TypeInfo;
	}

	//A CatLib is used to compile JitCat code from source through calls to addSource.
	//Source code added in this way will be able to use functions, variables, types and constatns defined in source 
	//files that have previously been added.
	//Through calls to addStaticScope, it is possible to also provide types, functions, variables and constants from reflected 
	//C++ classes and/or run-time created custom types.
	//In addition, any type registered with the TypeRegistry is available for use in JitCat source.
	//If added source code does not contain any errors, it is possible to query the CatLib for the types, functions and
	//variables it defines.
	class CatLib
	{
	public:
		CatLib(const std::string& libName);
		~CatLib();

		//addStaticScope adds a scope containing types, variables and/or functions that can be used by source files compiled by this CatLib. 
		//ScopeObject must not be null when compiling a CatLib and is assumed to be "static", that is, 
		//the address of the scopeObject must not change between compile time /and run time of functions 
		//within this CatLib.
		//If a scope object is deleted, any CatLibs that were compiled using it should either also be deleted, or recompiled using a new static scope.
		//Variables and functions are looked up in the reverse order in which the scopes were added. The most recently added scope is searched first.
		//The scopeObject must either implement the static functions required for reflection (see TypeInfo.h), define an ExternalReflector (
		//or provide a TypeInfo object containing typeInfo about the scopeObject combined with a unsigned char* pointing to an instance of the scopeObject.
		//ScopeObjects are not owned/deleted by the CatRuntimeContext.
		//Adding a scope returns a ScopeID that can later be used to remove or change the scope object.
		template<typename ReflectableType>
		CatScopeID addStaticScope(ReflectableType* scopeObject);
		CatScopeID addStaticScope(Reflection::TypeInfo* typeInfo, unsigned char* scopeObject);

		//When a static scope is removed, all source code that has already been compiled should be recompiled.
		void removeStaticScope(CatScopeID id);
		//When a static scope is replaced, all source code that has already been compiled should be recompiled.
		//The scopeObject should point to an object of the same type as the object it replaces.
		void replaceStaticScopeObject(CatScopeID id, unsigned char* scopeObject);

		//Add source code to the CatLib. 
		//The translationUnitName is used for printing errors and generating debug information.
		//If the source comes from an external text file, translationUnitName should refer to the file name.
		//Returns true if the source was compiled without errors.
		bool addSource(const std::string& translationUnitName, Tokenizer::Document& translationUnitCode);

		//Gets the name of the library.
		const std::string& getName() const;

		//Finds a type that should be defined in one of the added source files. 
		//Returns nullptr if the type was not found.
		Reflection::TypeInfo* getTypeInfo(const std::string& typeName) const;

		//Returns the error manager that contains a list of all the errors generated so far by calls to addSource.
		ExpressionErrorManager& getErrorManager() const;


		CatRuntimeContext* getRuntimeContext() const;

	private:
		std::string name;
		std::unique_ptr<ExpressionErrorManager> errorManager;
		std::unique_ptr<CatRuntimeContext> context;

		std::vector<std::unique_ptr<AST::CatSourceFile>> sourceFiles;

	};


	template<typename ReflectableType>
	inline CatScopeID CatLib::addStaticScope(ReflectableType* scopeObject)
	{
		return context->addScope(scopeObject, true);
	}

}