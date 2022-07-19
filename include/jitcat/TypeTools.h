/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ExternalReflector.h"
#include "jitcat/FunctionPresenceTest.h"
#include "jitcat/Tools.h"

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

namespace jitcat
{
	template <typename AnyType>
	struct RemoveConst
	{
		typedef AnyType type;
	};
	template<typename ConstType>
	struct RemoveConst<ConstType&>
	{
		typedef typename RemoveConst<ConstType>::type& type;
	};
	template<typename ConstType>
	struct RemoveConst<ConstType*>
	{
		typedef typename RemoveConst<ConstType>::type* type;
	};
	template<typename ConstType>
	struct RemoveConst<ConstType* const>
	{
		typedef typename RemoveConst<ConstType>::type* type;
	};
	template<typename ConstType>
	struct RemoveConst<const ConstType>
	{
		typedef typename RemoveConst<ConstType>::type type;
	};
	template<typename ConstType>
	struct RemoveConst<volatile ConstType>
	{
		typedef typename RemoveConst<ConstType>::type type;
	};


	//UnderlyingType exists because std::underlying_type causes a compile error on VS2017
	//when a std::underlying_type is used withing a if constexpr (std::is_enum_v<T>) and T is not an enum.
	template <typename AnyType, typename EnabledT = void>
	struct UnderlyingType
	{
		typedef AnyType type;
	};
	
	template <typename EnumT>
	struct UnderlyingType<EnumT, std::enable_if_t<std::is_enum_v<EnumT>>>
	{
		typedef typename std::underlying_type<EnumT>::type type;
	};
}

namespace jitcat::Reflection
{
    template<typename ReflectableT>
    class TypeNameGetter
    {
        TypeNameGetter() = delete;
        ~TypeNameGetter() = delete;
        TypeNameGetter(const TypeNameGetter&) = delete;
    public:
		using DecayedT = typename remove_all<ReflectableT>::type;

        static const char* get()
        {
			if constexpr (GetTypeNameAndReflectExist<DecayedT>::value)			return DecayedT::getTypeName();
			else if constexpr (std::is_enum_v<DecayedT>)						return getEnumName<DecayedT>();
			else if constexpr (ExternalReflector<DecayedT>::exists)				return ExternalReflector<DecayedT>::getTypeName();
			else if constexpr (std::is_same_v<void, DecayedT>)					return "void";
			else if constexpr (std::is_same_v<bool, DecayedT>)					return "bool";
			else if constexpr (std::is_same_v<char, DecayedT>)					return "char";
			else if constexpr (std::is_same_v<unsigned char, DecayedT>)			return "uchar";
			else if constexpr (std::is_same_v<wchar_t, DecayedT>)				return "wchar";
			else if constexpr (std::is_same_v<char16_t, DecayedT>)				return "char16";
			else if constexpr (std::is_same_v<char32_t, DecayedT>)				return "char32";
			else if constexpr (std::is_same_v<short, DecayedT>)					return "short";
			else if constexpr (std::is_same_v<unsigned short, DecayedT>)		return "ushort";
			else if constexpr (std::is_same_v<int, DecayedT>)					return "int";
			else if constexpr (std::is_same_v<unsigned int, DecayedT>)			return "uint";
			else if constexpr (std::is_same_v<long, DecayedT>)					return "long";
			else if constexpr (std::is_same_v<unsigned long, DecayedT>)			return "ulong";
			else if constexpr (std::is_same_v<long long, DecayedT>)				return "longlong";
			else if constexpr (std::is_same_v<unsigned long long, DecayedT>)	return "ulonglong";
			else if constexpr (std::is_same_v<float, DecayedT>)					return "float";
			else if constexpr (std::is_same_v<std::array<float,4>, DecayedT>)	return "vector4f";
			else if constexpr (std::is_same_v<double, DecayedT>)				return "double";
			else if constexpr (std::is_same_v<long double, DecayedT>)			return "longdouble";
			else																static_assert(std::is_same_v<void, DecayedT>, "Need to implement reflection for ReflectableT");
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

	template<typename ReflectableT>
	class QualifiedTypeNameGetter
	{
        QualifiedTypeNameGetter() = delete;
        ~QualifiedTypeNameGetter() = delete;
        QualifiedTypeNameGetter(const QualifiedTypeNameGetter&) = delete;
		public:
		static const char* get()
		{
			static_assert(std::is_same_v<ReflectableT, typename remove_all<ReflectableT>::type>, "Type should be fully unqualified at this point. There must be a missing specialization of QualifiedTypeNameGetter if this fails.");
			return TypeNameGetter<ReflectableT>::get();
		}
	};


	template<typename ReflectableT>
	class QualifiedTypeNameGetter<std::unique_ptr<ReflectableT>>
	{
		QualifiedTypeNameGetter() = delete;
		~QualifiedTypeNameGetter() = delete;
		QualifiedTypeNameGetter(const QualifiedTypeNameGetter&) = delete;
	public:
		static const char* get() 
		{
			static std::string typeName = Tools::append("unique_ptr<", QualifiedTypeNameGetter<ReflectableT>::get(), ">");
			return typeName.c_str();
		}
	};

	template<typename ReflectableT>
	class QualifiedTypeNameGetter<std::shared_ptr<ReflectableT>>
	{
		QualifiedTypeNameGetter() = delete;
		~QualifiedTypeNameGetter() = delete;
		QualifiedTypeNameGetter(const QualifiedTypeNameGetter&) = delete;
	public:
		static const char* get() 
		{
			static std::string typeName = Tools::append("shared_ptr<", QualifiedTypeNameGetter<ReflectableT>::get(), ">");
			return typeName.c_str();
		}
	};

	template<typename ReflectableT>
	class QualifiedTypeNameGetter<ReflectableT*>
	{
		QualifiedTypeNameGetter() = delete;
		~QualifiedTypeNameGetter() = delete;
		QualifiedTypeNameGetter(const QualifiedTypeNameGetter&) = delete;
	public:
		static const char* get() 
		{
			static const std::string typeName = Tools::append(QualifiedTypeNameGetter<ReflectableT>::get(), "*");
			return typeName.c_str();
		}
	};

	template<typename ReflectableT>
	class QualifiedTypeNameGetter<ReflectableT&>
	{
		QualifiedTypeNameGetter() = delete;
		~QualifiedTypeNameGetter() = delete;
		QualifiedTypeNameGetter(const QualifiedTypeNameGetter&) = delete;
	public:
		static const char* get() 
		{
			static const std::string typeName = Tools::append(QualifiedTypeNameGetter<ReflectableT>::get(), "&");
			return typeName.c_str();
		}
	};

	template<typename ReflectableT>
	class QualifiedTypeNameGetter<const ReflectableT>
	{
		QualifiedTypeNameGetter() = delete;
		~QualifiedTypeNameGetter() = delete;
		QualifiedTypeNameGetter(const QualifiedTypeNameGetter&) = delete;
	public:
		static const char* get() 
		{
			static const std::string typeName = Tools::append("const ", QualifiedTypeNameGetter<ReflectableT>::get());
			return typeName.c_str();
		}
	};
}
