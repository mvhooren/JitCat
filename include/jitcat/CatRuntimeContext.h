/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::LLVM
{
	class LLVMCodeGenerator;
}
namespace jitcat::Reflection
{
	class CustomTypeInfo;
	class CustomTypeInstance;
	struct MemberFunctionInfo;
	class Reflectable;
	class TypeInfo;
	struct TypeMemberInfo;
}
#include "jitcat/CatScopeID.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/RuntimeContext.h"
#include "jitcat/TypeRegistry.h"

#include <any>
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace jitcat
{
	class CatRuntimeContext;
	class CatScope;
	class ErrorContext;
	class ExpressionErrorManager;
	namespace AST
	{
		class CatFunctionDefinition;
		class CatScopeBlock;
	}
	//A CatRuntimeContext provides variables and functions for use in expressions. (See Expression.h, ExpressionAny.h)
	//It can contain multiple "scopes" of variables.
	//Variables can come from classes that inherit from Reflectable (and implement the static functions required for reflection, see TypeInfo.h) or
	//from a CustomTypeInfo / CustomTypeInstance, which represent a struct that is defined at runtime.
	//It also provides a context for errors that are generated when expressions are compiled using a CatRuntimeContext.
	//It does this by providing a context name as well as a stack of error contexts that provide better descriptions for any errors that are generated.
	//Errors are managed by the ExpressionErrorManager. An ExpressionErrorManager can be passed to the constructor to use one ExpressionErrorManager for multiple CatRuntimeContexts.
	//If no errorManager is passed, a new one will be created.
	class CatRuntimeContext: public RuntimeContext
	{
		struct Scope
		{
			//The TypeInfo and Reflectable are not owned here.
			Reflection::TypeInfo* scopeType;
			Reflection::ReflectableHandle scopeObject;
			bool isStatic;
		};
	public:
		//contextName is used to provide better descriptions for errors that are generated by expressions compiled using this context.
		//errorManager manages the list of errors generated by expressions. If errorManager is null, a new errorManager will be created just for this context.
		CatRuntimeContext(const std::string& contextName, ExpressionErrorManager* errorManager = nullptr);
		virtual ~CatRuntimeContext();

		//Returns the name of the context for the purpose of error messages.
		virtual std::string getContextName() override final;

		//addScope adds a scope containing variables and/or functions that can be used by an expression. 
		//If isStatic is true, scopeObject must not be null when compiling an expression and is assumed to be "static", that is, 
		//the address of the scopeObject must not change between compile time and run time of an expression.
		//If a static scope object is deleted, any expressions that were compiled using it should either also be deleted, or recompiled using a new static scope.
		//A dynamic scope object is assumed to potentially change after each execution of an expression and may be null at compile time.
		//Variables and functions are looked up in the reverse order in which the scopes were added. The most recently added scope is searched first.
		//The scopeObject must inherit from Reflectable and it must implement the static functions required for reflection (see TypeInfo.h).
		//scopeObjects are not owned/deleted by the CatRuntimeContext.
		//Adding a scope returns a ScopeID that can later be used to remove or change the scope object.
		template<typename ReflectableType>
		CatScopeID addScope(ReflectableType* scopeObject = nullptr, bool isStatic = false);
		//Same as above addObject, but explicitly specify type and scopeObject instead of deriving typeInfo from the scopeObject;
		CatScopeID addScope(Reflection::TypeInfo* typeInfo, Reflection::Reflectable* scopeObject = nullptr, bool isStatic = false);
		//Same as addScope but uses a custom type (a struct defined at runtime) instead of a reflected C++ class.
		CatScopeID addCustomTypeScope(Reflection::CustomTypeInfo* typeInfo, Reflection::CustomTypeInstance* scopeObject = nullptr, bool isStatic = false);

		int getNumScopes() const;
		//When a scope is removed, any expressions that were compiled using this context should be recompiled.
		void removeScope(CatScopeID id);
		//If the ScopeID refers to a static scope, any expressions that were compiled using this context should be recompiled.
		void setScopeObject(CatScopeID id, Reflection::Reflectable* scopeObject);
		//Returns weither or not the provided ScopeID is a static scope.
		bool isStaticScope(CatScopeID id) const;
	
		Reflection::Reflectable* getScopeObject(CatScopeID id) const;
		Reflection::TypeInfo* getScopeType(CatScopeID id) const;

		ExpressionErrorManager* getErrorManager() const;
		void pushErrorContext(ErrorContext* context);
		void popErrorContext(ErrorContext* context);

		Reflection::TypeMemberInfo* findVariable(const std::string& lowercaseName, CatScopeID& scopeId);
		Reflection::MemberFunctionInfo* findFunction(const std::string& lowercaseName, CatScopeID& scopeId);

		std::shared_ptr<LLVM::LLVMCodeGenerator> getCodeGenerator();
		int getNextFunctionIndex();

		void setCurrentFunction(AST::CatFunctionDefinition* function);
		AST::CatFunctionDefinition* getCurrentFunction() const;

		void setCurrentScope(CatScope* scope);
		CatScope* getCurrentScope() const;
		Reflection::Reflectable* getCurrentScopeObject() const;

		bool getIsReturning() const;
		void setReturning(bool isReturning);

	private:
		CatScopeID createScope(Reflection::Reflectable* scopeObject, Reflection::TypeInfo* type, bool isStatic);

	public:
		static CatRuntimeContext defaultContext;
	private:
		int nextFunctionIndex;
		AST::CatFunctionDefinition* currentFunctionDefinition;
		CatScope* currentScope;

		bool returning;

		bool ownsErrorManager;
		ExpressionErrorManager* errorManager;

		std::string contextName;
		//Scopes should not actually be removed from this list (but they can be set to nullptr if removeScope is called).
		//This allows the ScopeID to be equal to the index of the scope in this vector.
		std::vector<std::unique_ptr<Scope>> scopes;

	#ifdef ENABLE_LLVM
		std::shared_ptr<LLVM::LLVMCodeGenerator> codeGenerator;
	#endif
		std::vector<ErrorContext*> errorContextStack;
	};


	template<typename ReflectableType>
	inline CatScopeID CatRuntimeContext::addScope(ReflectableType* scopeObject, bool isStatic)
	{
		static_assert(std::is_base_of<Reflection::Reflectable, ReflectableType>::value, "scopeObject must inherit from Reflectable");
		//scopeObject must not be nullptr if it is static
		assert(!isStatic || scopeObject != nullptr);
		Reflection::TypeInfo* typeInfo = Reflection::TypeRegistry::get()->registerType<ReflectableType>();
		return createScope(static_cast<Reflection::Reflectable*>(scopeObject), typeInfo, isStatic);
	}

} //End namespace jitcat
