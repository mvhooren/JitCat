/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ExternalReflector.h"
#include "jitcat/FunctionPresenceTest.h"

#include <memory>
#include <string>
#include <type_traits>

template<class T, class U=
	  typename std::remove_cv<
	  typename std::remove_pointer<
	  typename std::remove_reference<
	  typename std::remove_extent<
	  T
	  >::type
	  >::type
	  >::type
	  >::type
	  > struct remove_all : remove_all<U> {};
	template<class T> struct remove_all<T, T> { typedef T type; };


namespace jitcat::Reflection
{
    template<typename ReflectableT>
    class TypeNameGetter
    {
		using DecayedT = typename remove_all<ReflectableT>::type;
        TypeNameGetter() = delete;
        ~TypeNameGetter() = delete;
        TypeNameGetter(const TypeNameGetter&) = delete;
    public:
        static const char* get()
        {
			if constexpr (GetTypeNameAndReflectExist<DecayedT>::value)
			{
				return DecayedT::getTypeName();
			}
			else if constexpr (std::is_enum_v<DecayedT>)
			{
				return getEnumName<DecayedT>();			
			}
			else if constexpr (ExternalReflector<DecayedT>::exists)
			{
				return ExternalReflector<DecayedT>::getTypeName();
			}
			else if constexpr (std::is_same_v<void, DecayedT>)
			{
				return "void";
			}
			else if constexpr (std::is_same_v<float, DecayedT>)
			{
				return "float";
			}
			else if constexpr (std::is_same_v<int, DecayedT>)
			{
				return "int";
			}
			else if constexpr (std::is_same_v<bool, DecayedT>)
			{
				return "bool";
			}
			else if constexpr (std::is_same_v<std::string, DecayedT>)
			{
				return "string";
			}
			else
			{
				static_assert(false, "Need to implement reflection for ReflectableT");
			}
        }
    };

	template<typename ReflectableT>
	class TypeNameGetter<std::unique_ptr<ReflectableT>>
	{
		TypeNameGetter() = delete;
		~TypeNameGetter() = delete;
		TypeNameGetter(const TypeNameGetter&) = delete;
	public:
		static const char* get() { return TypeNameGetter<ReflectableT>::get();}
	};

}	



