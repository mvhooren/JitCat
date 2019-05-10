/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once


namespace jitcat
{
	namespace Reflection
	{
		class Reflectable;
	}
	namespace AST
	{
		class CatClassDefinition;
	}
	class CatRuntimeContext;
	class ExpressionErrorManager;

	class CatHostClass
	{
	public:
		CatHostClass(bool constructible, bool inheritable) : constructible(constructible), inheritable(inheritable) {};
		virtual ~CatHostClass() {}
		virtual Reflection::Reflectable* construct() = 0;
		virtual void destruct(Reflection::Reflectable* object) = 0;
		virtual void inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext) {};
		bool isConstructible() { return constructible; }
		bool isInheritable() { return inheritable; }
	private:
		bool constructible;
		bool inheritable;
	};
}