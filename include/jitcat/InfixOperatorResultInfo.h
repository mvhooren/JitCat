/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <memory>

namespace jitcat
{
	class CatGenericType;
}
namespace jitcat::Reflection
{
	class TypeInfo;
}


namespace jitcat::AST
{
	class InfixOperatorResultInfo
	{
	public:
		InfixOperatorResultInfo();
		void setResultType(const CatGenericType& result);
		const CatGenericType& getResultType() const;

		void setIsOverloaded(bool overloaded);
		bool getIsOverloaded() const;
		bool getIsStaticOverloaded() const;

		void setStaticOverloadType(Reflection::TypeInfo* overloadType);
		Reflection::TypeInfo* getStaticOverloadedType() const;

	private:
		std::unique_ptr<CatGenericType> resultType;
		bool isOverloaded;
		Reflection::TypeInfo* staticOverloadedType;
	};
}