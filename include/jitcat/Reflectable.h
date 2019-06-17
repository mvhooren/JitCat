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
	class ReflectableHandle;
	class ReflectedTypeInfo;

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

	private:
		static std::unordered_multimap<Reflectable*, ReflectableHandle*> observers;
	};

} //End namespace jitcat::Reflection