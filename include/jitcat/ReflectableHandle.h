/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ExternalReflector.h"

namespace jitcat::Reflection
{
	class Reflectable;


	class ReflectableHandle
	{
	public:
		ReflectableHandle();
		ReflectableHandle(Reflectable* reflectable);
		ReflectableHandle(const ReflectableHandle& other);
		ReflectableHandle(const ReflectableHandle&& other) = delete;
		~ReflectableHandle();
		//Returns nullptr if reflectable is deleted
		Reflectable* get() const;
		bool getIsValid() const;
		ReflectableHandle& operator=(const ReflectableHandle& other);
		ReflectableHandle& operator=(Reflectable* other);

		void notifyDeletion();

		static Reflectable* staticGet(const ReflectableHandle& handle);
		static void staticAssign(ReflectableHandle& handle, Reflectable* reflectable);

	private:
		Reflectable* reflectable;
	};

	template<>
	class ExternalReflector<ReflectableHandle>
	{
	public:
		static const char* getTypeName() {return "ReflectableHandle";}
		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo) {}	
		static constexpr bool exists = true;
	};

} //End namespace jitcat::Reflection