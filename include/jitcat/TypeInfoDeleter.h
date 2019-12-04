/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <memory>


namespace jitcat::Reflection
{

	class TypeInfo;
	struct TypeInfoDeleter
	{
		void operator()(TypeInfo* typeInfo) const;
	};

	template <class TypeInfoT, class... ConstructorArgsT>
	inline std::unique_ptr<TypeInfoT, TypeInfoDeleter> makeTypeInfo(ConstructorArgsT&&... args) 
	{ 
		return std::unique_ptr<TypeInfoT, TypeInfoDeleter>(new TypeInfoT(std::forward<ConstructorArgsT>(args)...), TypeInfoDeleter());
	}
}
