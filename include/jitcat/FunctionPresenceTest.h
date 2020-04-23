/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <type_traits>


namespace jitcat::Reflection
{
    class ReflectedTypeInfo;
		    //static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		    //static const char* getTypeName();
    template<typename BaseT>
    struct GetTypeNameAndReflectExist
    {
        template<typename FunctionPtrT, FunctionPtrT> struct SameType;

        template<typename TestBaseT>
        static constexpr std::true_type testPresence(SameType<const char*(*)(), &TestBaseT::getTypeName>*,SameType<void(*)(jitcat::Reflection::ReflectedTypeInfo& typeInfo), &TestBaseT::reflect>*);
        template<typename TestBaseT>
        static constexpr std::false_type testPresence(...);

        static constexpr bool value = decltype(testPresence<BaseT>(0, 0))::value;
    };

    template<typename BaseT>
    struct ReflectExists
    {
        template<typename FunctionPtrT, FunctionPtrT> struct SameType;

        template<typename TestBaseT>
        static constexpr std::true_type testPresence(SameType<void(*)(jitcat::Reflection::ReflectedTypeInfo& typeInfo), &TestBaseT::reflect>*);
        template<typename TestBaseT>
        static constexpr std::false_type testPresence(...);

        static constexpr bool value = decltype(testPresence<BaseT>(0))::value;
    };
}

/*template<typename, typename>
struct GetTypeNameExists;

template<typename T, typename Ret, typename... Args>
struct GetTypeNameExists<T, Ret(Args...)> {
    template<typename U, U> struct Check;

    template<typename U>
    static std::true_type Test(Check<Ret(*)(Args...), &U::fun>*);

    template<typename U>
    static std::false_type Test(...);

    static const bool value = decltype(Test<T>(0))::value;
};*/