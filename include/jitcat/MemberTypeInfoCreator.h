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
			return new ClassObjectMemberInfo<ClassType, T>(memberName, member, CatGenericType(nestedType, isWritable, isConst).toPointer(TypeOwnershipSemantics::Value, false, isConst));
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
			return new ClassUniquePtrMemberInfo<ClassType, U>(memberName, member, CatGenericType(nestedType, isWritable, isConst).toPointer(TypeOwnershipSemantics::Weak, false, isConst));
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
			return new ClassPointerMemberInfo<ClassType, U>(memberName, member, CatGenericType(nestedType, isWritable, isConst).toPointer(TypeOwnershipSemantics::Weak, isWritable, isConst));
		}
	};


	template <typename ItemType, typename AllocatorT>
	class MemberTypeInfoCreator<std::vector<ItemType, AllocatorT> >
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, std::vector<ItemType, AllocatorT> ClassType::* member, bool isConst, bool isWritable)
		{
			return new ContainerMemberInfo<ClassType, std::vector<ItemType, AllocatorT> >(memberName, member, TypeTraits<std::vector<ItemType, AllocatorT>>::toGenericType());
		}
	};


	template <typename KeyType, typename ItemType, typename CompareT, typename AllocatorT>
	class MemberTypeInfoCreator<std::map<KeyType, ItemType, CompareT, AllocatorT> >
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, std::map<KeyType, ItemType, CompareT, AllocatorT> ClassType::* member, bool isConst, bool isWritable)
		{
			return new ContainerMemberInfo<ClassType, std::map<KeyType, ItemType, CompareT, AllocatorT> >(memberName, member, TypeTraits<std::map<KeyType, ItemType, CompareT, AllocatorT>>::toGenericType());
		}
	};


} //End namespace jitcat::Reflection