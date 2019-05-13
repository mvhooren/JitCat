/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <functional>
#include <map>
#include <string>

#include "jitcat/CatHostClass.h"

namespace jitcat
{
	//Manages a set of Reflectable C++ host class types that can be instantiated and/or inherited by classes in the JitCat language.
	class CatHostClasses
	{
	public:
		CatHostClasses();
		~CatHostClasses();

		void addHostClass(std::unique_ptr<CatHostClass>& hostClass, const std::string& className);

		template<typename ReflectableT>
		inline void addHostClass(bool constructible, bool inheritable);

		template<typename ReflectableT>
		inline void addHostClass(bool constructible, bool inheritable,
								std::function<ReflectableT* ()> constructor,
								std::function<void(ReflectableT*)> destructor = [](ReflectableT * object) {delete object; });

		CatHostClass* getHostClass(const std::string& hostClassName) const;

	private:
		std::map<std::string, std::unique_ptr<CatHostClass>> hostClasses;
	};
}

#include "CatHostClassesHeaderImplementation.h"