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
		CatHostClass(bool constructible, bool placementConstructible, bool inheritable) : constructible(constructible), placementConstructible(placementConstructible), inheritable(inheritable) {};
		virtual ~CatHostClass() {}

		virtual Reflection::Reflectable* construct() = 0;
		virtual void destruct(Reflection::Reflectable* object) = 0;

		virtual void placementConstruct(unsigned char* data, std::size_t dataSize) = 0;
		virtual void placementDestruct(unsigned char* data, std::size_t dataSize) = 0;

		virtual bool inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext) { return true; };
		bool isConstructible() const { return constructible; }
		bool isPlacementConstructible() const {	return placementConstructible; }
		bool isInheritable() const { return inheritable; }

	private:
		bool constructible;
		bool placementConstructible;
		bool inheritable;
	};
}