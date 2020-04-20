/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include "jitcat/CatGenericType.h"

#include <any>
#include <string>


namespace jitcat::Reflection
{
	class StaticConstMemberInfo
	{
	public:
		StaticConstMemberInfo(const std::string& name, const CatGenericType& type, const std::any& value);

		const std::string& getName() const;
		const std::string& getLowerCaseName() const;
		const CatGenericType& getType() const;
		const std::any& getValue() const;

	private:
		std::string name;
		std::string lowerCaseName;
		CatGenericType type;
		std::any value;
	};
}