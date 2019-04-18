/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/MemberInfo.h"
#include "jitcat/TypeRegistry.h"

namespace jitcat::Reflection
{

	template <typename T>
	class MemberTypeInfoCreator
	{
	public:
		template<typename ClassType>
		static inline TypeMemberInfo* getMemberInfo(const std::string& memberName, T ClassType::* member, bool isConst, bool isWritable) 
		{
			static_assert(std::is_base_of<Reflectable, T>::value, "Unsupported reflectable type.");
			TypeInfo* nestedType = TypeRegistry::get()->registerType<T>();
			return new ClassObjectMemberInfo<ClassType, T>(memberName, member, CatGenericType(nestedType, false, isConst));
		}
	};


	template <>
	class MemberTypeInfoCreator<void>
	{
	public:
		template<typename ClassType>
		static inline TypeMemberInfo* getMemberInfo(const std::string& memberName, int ClassType::* member, bool isConst, bool isWritable) { return nullptr;}
	};


	template <>
	class MemberTypeInfoCreator<float>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, float ClassType::* member, bool isConst, bool isWritable) 
		{
			return new BasicTypeMemberInfo<ClassType, float>(memberName, member, CatGenericType::createFloatType(isWritable, isConst));
		}
	};


	template <>
	class MemberTypeInfoCreator<int>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, int ClassType::* member, bool isConst, bool isWritable) 
		{
			return new BasicTypeMemberInfo<ClassType, int>(memberName, member, CatGenericType::createIntType(isWritable, isConst));
		}
	};


	template <>
	class MemberTypeInfoCreator<bool>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, bool ClassType::* member, bool isConst, bool isWritable) 
		{
			return new BasicTypeMemberInfo<ClassType, bool>(memberName, member, CatGenericType::createBoolType(isWritable, isConst));
		}
	};


	template <>
	class MemberTypeInfoCreator<std::string>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, std::string ClassType::* member, bool isConst, bool isWritable) 
		{
			return new BasicTypeMemberInfo<ClassType, std::string>(memberName, member, CatGenericType::createStringType(isWritable, isConst));
		}
	};


	template <typename U>
	class MemberTypeInfoCreator<std::unique_ptr<U>>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, std::unique_ptr<U> ClassType::* member, bool isConst, bool isWritable) 
		{
			TypeInfo* nestedType = TypeRegistry::get()->registerType<U>();
			return new ClassUniquePtrMemberInfo<ClassType, U>(memberName, member, CatGenericType(nestedType, false, isConst));
		}
	};


	template <typename U>
	class MemberTypeInfoCreator<U*>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, U* ClassType::* member, bool isConst, bool isWritable) 
		{
			TypeInfo* nestedType = TypeRegistry::get()->registerType<U>();
			return new ClassPointerMemberInfo<ClassType, U>(memberName, member, CatGenericType(nestedType, isWritable, isConst));
		}
	};


	template <typename ItemType>
	class MemberTypeInfoCreator<std::vector<ItemType> >
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, std::vector<ItemType> ClassType::* member, bool isConst, bool isWritable) 
		{
			TypeInfo* nestedType = TypeTraits<ItemType>::getTypeInfo();
			
			return new ContainerMemberInfo<ClassType, std::vector<ItemType> >(memberName, member, TypeTraits<std::vector<ItemType>>::toGenericType());
		}
	};


	template <typename ItemType, typename Compare>
	class MemberTypeInfoCreator<std::map<std::string, ItemType, Compare> >
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, std::map<std::string, ItemType, Compare> ClassType::* member, bool isConst, bool isWritable) 
		{
			TypeInfo* nestedType = TypeTraits<ItemType>::getTypeInfo();
			return new ContainerMemberInfo<ClassType, std::map<std::string, ItemType, Compare> >(memberName, member, TypeTraits<std::map<std::string, ItemType, Compare>>::toGenericType());
		}
	};


} //End namespace jitcat::Reflection