/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/HandleTrackingMethod.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <set>

namespace jitcat::AST
{
	class CatClassDefinition;
	class CatFunctionDefinition;
}

namespace jitcat::Reflection
{
	struct CustomTypeMemberFunctionInfo;
	struct StaticMemberInfo;
	class StaticConstMemberInfo;

	//Represents a compound type that can be defined at runtime.

	//Members can be added to the type using the addMember functions.

	//An instance of the struct can be created using construction functions.

	//Instances are tracked and when a member is added after instances have been created, all instances will be updated.
	//Instances should be held in a ReflectableHandle in order to receive updates caused by the addition of members. 
	//Naked pointers to an instance will become invalid after members are added.

	class CustomTypeInfo: public TypeInfo
	{
	public:
		CustomTypeInfo(const char* typeName, HandleTrackingMethod trackingMethod = HandleTrackingMethod::InternalHandlePointer);
		CustomTypeInfo(AST::CatClassDefinition* classDefinition, HandleTrackingMethod trackingMethod );
	protected:
		virtual ~CustomTypeInfo();
	public:

		//Instances that have been created before members are added will be updated.
		TypeMemberInfo* addDoubleMember(const std::string& memberName, float defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addFloatMember(const std::string& memberName, float defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addIntMember(const std::string& memberName, int defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addBoolMember(const std::string& memberName, bool defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addVector4fMember(const std::string& memberName, const std::array<float, 4> defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addStringMember(const std::string& memberName, const Configuration::CatString& defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addObjectMember(const std::string& memberName, unsigned char* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics = TypeOwnershipSemantics::Weak, bool isWritable = true, bool isConst = false);
		template <typename ObjectT>
		inline TypeMemberInfo* addObjectMember(const std::string& memberName, ObjectT* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics = TypeOwnershipSemantics::Weak, bool isWritable = true, bool isConst = false)
		{
			return addObjectMember(memberName, reinterpret_cast<unsigned char*>(defaultValue), objectTypeInfo, ownershipSemantics, isWritable, isConst);
		}
		template <typename EnumT>
		inline TypeMemberInfo* addEnumMember(const std::string& memberName, EnumT defaultValue, bool isWritable = true, bool isConst = false)
		{
			unsigned char* data = increaseDataSize(sizeof(EnumT));
			memcpy(data, &defaultValue, sizeof(EnumT));
			unsigned int offset = (unsigned int)(data - defaultData);
			if (defaultData == nullptr)
			{
				offset = 0;
			}

			std::set<unsigned char*>::iterator end = instances.end();
			for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
			{
				memcpy((unsigned char*)(*iter) + offset, &defaultValue, sizeof(EnumT));
			}

			CatGenericType enumType = TypeTraits<EnumT>::toGenericType().copyWithFlags(isWritable, isConst);
			TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<EnumT>(memberName, offset, enumType, getTypeName());
			std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
			TypeInfo::addMember(lowerCaseMemberName, memberInfo);
			return memberInfo;			
		}

		template <typename ConstantT>
		inline StaticConstMemberInfo* addConstant(const std::string& identifier, ConstantT value)
		{
			CatGenericType type = TypeTraits<typename RemoveConst<ConstantT>::type>::toGenericType();
			std::any anyValue = TypeTraits<ConstantT>::getCatValue(value);
			return TypeInfo::addConstant(identifier, type, anyValue);
		}
		TypeMemberInfo* addDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo);

		TypeMemberInfo* addMember(const std::string& memberName, const CatGenericType& type);

		//Add a static member variable
		StaticMemberInfo* addStaticDoubleMember(const std::string& memberName, double defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticFloatMember(const std::string& memberName, float defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticIntMember(const std::string& memberName, int defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticBoolMember(const std::string& memberName, bool defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticStringMember(const std::string& memberName, const Configuration::CatString& defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticObjectMember(const std::string& memberName, unsigned char* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics = TypeOwnershipSemantics::Weak, bool isWritable = true, bool isConst = false);
		template<typename ObjectT>
		StaticMemberInfo* addStaticObjectMember(const std::string& memberName, ObjectT* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics = TypeOwnershipSemantics::Weak, bool isWritable = true, bool isConst = false)
		{
			return addStaticObjectMember(memberName, reinterpret_cast<unsigned char*>(defaultValue), objectTypeInfo, ownershipSemantics, isWritable, isConst);
		}
		StaticMemberInfo* addStaticDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo);

		StaticMemberInfo* addStaticMember(const std::string& memberName, const CatGenericType& type);

		CustomTypeMemberFunctionInfo* addMemberFunction(const std::string& memberFunctionName, const CatGenericType& thisType, AST::CatFunctionDefinition* functionDefinition);

		//Set a function that was previously added with addMemberFunction to be the default constructor.
		//Returns true if the function name was found and if it is a valid default constructor (it has no arguments).
		bool setDefaultConstructorFunction(const std::string& constructorFunctionName);

		//Set a function that was previously added with addMemberFunction to be the destructor.
		//Returns true if the function name was found and if it is a valid destructor (it has no arguments).
		bool setDestructorFunction(const std::string& constructorFunctionName);

		//This will not shrink the typeSize, only remove the member from the list.
		//The data will only shrink after a restart of the program.
		//Because of this, the CustomTypeInfo remains compatible with existing instances.
		//It is assumed that this does not happen very often.
		void removeMember(const std::string& memberName);
		
		//For creating a "static" data type, this instance points directly to the default data.
		unsigned char* getDefaultInstance();

		virtual bool isCustomType() const override final;

		AST::CatClassDefinition* getClassDefinition();

		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const override final;
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize) override final;
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;

		virtual bool isTriviallyCopyable() const override final;
		virtual bool isTriviallyConstructable() const override final;

		virtual bool canBeDeleted() const override final;

		llvm::orc::JITDylib* getDylib() const;
		void setDylib(llvm::orc::JITDylib* generatedDylib);

		HandleTrackingMethod getHandleTrackingMethod() const;

	private:
		void instanceDestructor(unsigned char* data);
		void instanceDestructorInPlace(unsigned char* data);

		std::size_t addReflectableHandle(unsigned char* defaultValue, TypeInfo* objectType);
		//Returns a pointer to the start of the newly added size
		unsigned char* increaseDataSize(std::size_t amount);
		void increaseDataSize(unsigned char*& data, std::size_t amount, std::size_t currentSize);
		void createDataCopy(const unsigned char* sourceData, std::size_t sourceSize, unsigned char* copyData, std::size_t copySize) const;

		void removeInstance(unsigned char* instance);

	private:
		mutable std::set<unsigned char*> instances;

		HandleTrackingMethod trackingMethod;

		//If this custom type belongs to a CatClassDefinition, classDefinition is non-null.
		AST::CatClassDefinition* classDefinition;

		unsigned char* defaultData;

		std::vector<std::unique_ptr<unsigned char>> staticData;

		bool triviallyCopyable;
		bool triviallyConstructable;

		MemberFunctionInfo* defaultConstructorFunction;
		MemberFunctionInfo* destructorFunction;

		std::vector<std::unique_ptr<TypeMemberInfo>> removedMembers;

		llvm::orc::JITDylib* dylib;
	};


} //End namespace jitcat::Reflection