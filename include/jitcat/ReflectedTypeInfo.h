/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include "jitcat/TypeInfo.h"

#include <cstddef>
#include <functional>


namespace jitcat::Reflection
{
	class Reflectable;
	class TypeCaster;

	class ReflectedTypeInfo: public TypeInfo
	{
	public:
		ReflectedTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* typeCaster, std::function<Reflectable*()>& constructor,
						  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
						  std::function<void (Reflectable*)>& destructor): 
			TypeInfo(typeName, typeSize, typeCaster),
			constructor(constructor),
			placementConstructor(placementConstructor),
			destructor(destructor)
		{}

		virtual ~ReflectedTypeInfo() {};

		//Adds information of a member of type U inside struct/class T
		//A second function exists to differentiate between U and U*
		template <typename ReflectedT, typename MemberT>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberT ReflectedT::* member, MemberFlags flags = MF::none);

		template <typename ReflectedT, typename MemberT, typename ... Args>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberT (ReflectedT::*function)(Args...));

		template <typename ReflectedT, typename MemberT, typename ... Args>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberT (ReflectedT::*function)(Args...) const);

		inline virtual void construct(unsigned char* buffer, std::size_t bufferSize) const override final;
		inline virtual Reflectable* construct() const override final;
		inline virtual void destruct(Reflectable* object) override final;

	private:
		std::function<Reflectable*()> constructor;
		std::function<void(unsigned char* buffer, std::size_t bufferSize)> placementConstructor;
		std::function<void (Reflectable*)> destructor;
	};

}

#include "jitcat/ReflectedTypeInfoHeaderImplementation.h"