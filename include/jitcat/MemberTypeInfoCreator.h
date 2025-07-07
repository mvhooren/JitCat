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

	template <typename ObjectT>
	class MemberTypeInfoCreator
	{
	public:
		template<typename ClassType>
		static inline TypeMemberInfo* getMemberInfo(const std::string& memberName, ObjectT ClassType::* member, bool isConst, bool isWritable) 
		{
			if constexpr (std::is_enum_v<ObjectT>)
			{
				return new BasicTypeMemberInfo<ClassType, ObjectT>(memberName, member, TypeTraits<ObjectT>::toGenericType());
			}
			else
			{
				TypeInfo* nestedType = TypeRegistry::get()->registerType<ObjectT>();
				return new ClassObjectMemberInfo<ClassType, ObjectT>(memberName, member, CatGenericType(nestedType, isWritable, isConst).toPointer(TypeOwnershipSemantics::Value, false, isConst));
			}
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
	class MemberTypeInfoCreator<double>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, double ClassType::* member, bool isConst, bool isWritable) 
		{
			return new BasicTypeMemberInfo<ClassType, double>(memberName, member, CatGenericType::createDoubleType(isWritable, isConst));
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
	class MemberTypeInfoCreator<unsigned int>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, unsigned int ClassType::* member, bool isConst, bool isWritable) 
		{
			return new BasicTypeMemberInfo<ClassType, unsigned int>(memberName, member, CatGenericType::createUIntType(isWritable, isConst));
		}
	};


	template <>
	class MemberTypeInfoCreator<uint64_t>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, uint64_t ClassType::* member, bool isConst, bool isWritable) 
		{
			return new BasicTypeMemberInfo<ClassType, uint64_t>(memberName, member, CatGenericType::createUInt64Type(isWritable, isConst));
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


	template <typename PointerT>
	class MemberTypeInfoCreator<std::unique_ptr<PointerT>>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, std::unique_ptr<PointerT> ClassType::* member, bool isConst, bool isWritable) 
		{
			TypeInfo* nestedType = TypeRegistry::get()->registerType<PointerT>();
			return new ClassUniquePtrMemberInfo<ClassType, PointerT>(memberName, member, CatGenericType(nestedType, isWritable, isConst).toPointer(TypeOwnershipSemantics::Weak, false, isConst));
		}
	};


	template <typename PointerT>
	class MemberTypeInfoCreator<PointerT*>
	{
	public:
		template<typename ClassType>
		static TypeMemberInfo* getMemberInfo(const std::string& memberName, PointerT* ClassType::* member, bool isConst, bool isWritable) 
		{
			TypeInfo* nestedType = TypeRegistry::get()->registerType<PointerT>();
			return new ClassPointerMemberInfo<ClassType, PointerT>(memberName, member, CatGenericType(nestedType, isWritable, isConst).toPointer(TypeOwnershipSemantics::Weak, isWritable, isConst));
		}
	};


} //End namespace jitcat::Reflection