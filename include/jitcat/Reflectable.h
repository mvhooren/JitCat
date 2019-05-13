/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include <vector>
#include <string>

namespace jitcat::Reflection
{
	class ReflectableHandle;
	class TypeInfo;


	class Reflectable
	{
	public:
		Reflectable();
		Reflectable(const Reflectable& other);
		//Allow move construction, all the observers will be updated to the new Reflectable
		Reflectable(Reflectable&& other) noexcept;

		Reflectable& operator=(const Reflectable& other);

		virtual ~Reflectable();

		void addObserver(ReflectableHandle* observer);
		void removeObserver(ReflectableHandle* observer);

	private:
		std::vector<ReflectableHandle*> observers;
	};

} //End namespace jitcat::Reflection