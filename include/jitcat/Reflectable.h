/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <unordered_map>
#include <vector>
#include <string>


namespace jitcat::Reflection
{
	class CustomTypeInfo;
	class ReflectableHandle;
	class ReflectedTypeInfo;

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

		static void destruct(Reflectable* reflectable);
		static void placementDestruct(Reflectable* reflectable);
		//Will replace the reflectable in all ReflectableHandle observers with the newReflectable.
		static void replaceReflectable(Reflectable* oldReflectable, Reflectable* newReflectable);

	private:
		//This is a pointer because otherwise it would be subject to static member destruction order, causing
		//static reflectable objects that are destructed to sometimes crash because observers was already destroyed.
		//Because of this, observers is also intentionally leaked.
		static std::unordered_multimap<Reflectable*, ReflectableHandle*>* observers;
	};
} //End namespace jitcat::Reflection