/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/TypeInfo.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <set>

namespace jitcat::AST
{
	class CatFunctionDefinition;
}

namespace jitcat::Reflection
{
	struct CustomTypeMemberFunctionInfo;
	struct StaticMemberInfo;

	//Represents a compound type that can be defined at runtime.

	//Members can be added to the type using the addMember functions.

	//An instance of the struct can be created using construction functions.

	//Instances are tracked and when a member is added after instances have been created, all instances will be updated.
	//Instances should be held in a ReflectableHandle in order to receive updates caused by the addition of members. 
	//Naked pointers to an instance will become invalid after members are added.

	class CustomTypeInfo: public TypeInfo
	{
	public:
		CustomTypeInfo(const char* typeName, bool isConstType = false);
	protected:
		virtual ~CustomTypeInfo();
	public:

		//Instances that have been created before members are added will be updated.
		TypeMemberInfo* addFloatMember(const std::string& memberName, float defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addIntMember(const std::string& memberName, int defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addBoolMember(const std::string& memberName, bool defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addStringMember(const std::string& memberName, const std::string& defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addObjectMember(const std::string& memberName, Reflectable* defaulValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics = TypeOwnershipSemantics::Weak, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo);

		TypeMemberInfo* addMember(const std::string& memberName, const CatGenericType& type);

		//Add a static member variable
		StaticMemberInfo* addStaticFloatMember(const std::string& memberName, float defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticIntMember(const std::string& memberName, int defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticBoolMember(const std::string& memberName, bool defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticStringMember(const std::string& memberName, const std::string& defaultValue, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticObjectMember(const std::string& memberName, Reflectable* defaulValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics = TypeOwnershipSemantics::Weak, bool isWritable = true, bool isConst = false);
		StaticMemberInfo* addStaticDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo);

		StaticMemberInfo* addStaticMember(const std::string& memberName, const CatGenericType& type);

		CustomTypeMemberFunctionInfo* addMemberFunction(const std::string& memberFunctionName, const CatGenericType& thisType, AST::CatFunctionDefinition* functionDefinition);

		//This will not shrink the typeSize, only remove the member from the list.
		//The data will only shrink after a restart of the program.
		//Because of this, the CustomTypeInfo remains compatible with existing instances.
		//It is assumed that this does not happen very often.
		void removeMember(const std::string& memberName);
		
		//For creating a "static" data type, this instance points directly to the default data.
		Reflectable* getDefaultInstance();

		virtual bool isCustomType() const;

		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const override final;
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize) override final;
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;

		virtual bool isTriviallyCopyable() const;

		virtual bool canBeDeleted() const override final;

	private:
		void instanceDestructor(unsigned char* data);
		void instanceDestructorInPlace(unsigned char* data);

		std::size_t addReflectableHandle(Reflectable* defaultValue);
		//Returns a pointer to the start of the newly added size
		unsigned char* increaseDataSize(std::size_t amount);
		void increaseDataSize(unsigned char*& data, std::size_t amount, std::size_t currentSize);
		void createDataCopy(const unsigned char* sourceData, std::size_t sourceSize, unsigned char* copyData, std::size_t copySize) const;

		void removeInstance(Reflectable* instance);

	private:
		mutable std::set<Reflectable*> instances;

		bool isConstType;
		unsigned char* defaultData;

		std::vector<std::unique_ptr<unsigned char>> staticData;

		bool triviallyCopyable;

		std::vector<std::unique_ptr<TypeMemberInfo>> removedMembers;
	};


} //End namespace jitcat::Reflection