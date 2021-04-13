/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ExternalReflector.h"

#include <unordered_map>
#include <vector>
#include <string>


namespace jitcat::Reflection
{
	class CustomTypeInfo;
	class ReflectableHandle;
	class ReflectedTypeInfo;
	class TypeInfo;

	//Reflectable can optionally be inherited by any class that wants to reflect its members.
	//Its main task is that it will make sure that any references created by the scripting 
	//system will be set to null when the Reflectable is destroyed.
	//It does this without any data members.
	//If an object is being reflected, but it did not inherit from Reflectable, then 
	//the destruct/placementDestruct should be called manually when the object is destroyed if 
	//any handles might possibly have been created for it.
	class Reflectable
	{
	public:
		Reflectable();
		Reflectable(const Reflectable& other);
		//Allow move construction, all the observers will be updated to the new Reflectable
		Reflectable(Reflectable&& other) noexcept;

		Reflectable& operator=(const Reflectable& other);
	protected:
		~Reflectable();

	public:
		void addObserver(ReflectableHandle* observer);
		void removeObserver(ReflectableHandle* observer);
		std::size_t getNumReflectableHandles() const;

		bool validateHandles(TypeInfo* reflectableType);
	private:
		bool hasHandle(ReflectableHandle* handle) const;

	private:
		ReflectableHandle* firstHandle;
	};

	template<>
	class ExternalReflector<Reflectable>
	{
	public:
		static const char* getTypeName() {return "Reflectable";}
		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo) {}	
		static constexpr bool exists = true;
	};
} //End namespace jitcat::Reflection