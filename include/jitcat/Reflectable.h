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
	private:
		//disable copying, this is usually a bad idea because of the observers list
		Reflectable(const Reflectable& );
	public:
		Reflectable();
		//Do allow move construction, all the observers will be updated to the new Reflectable
		Reflectable(Reflectable&&) noexcept;
		virtual ~Reflectable();

		void addObserver(ReflectableHandle* observer);
		void removeObserver(ReflectableHandle* observer);

		virtual void copyFrom(const Reflectable* other) {}

	private:
		std::vector<ReflectableHandle*> observers;
	};

} //End namespace jitcat::Reflection