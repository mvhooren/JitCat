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
	class CustomTypeInstance;
	struct CustomTypeMemberFunctionInfo;

	//Represents the type of a struct that can be defined at runtime.
	//Fields can be added to the struct using the addMember functions.
	//An instance of the struct can be created using createInstance();
	//Instances are tracked and when a member is added after instances have been created, all instances will be updated.
	//The CustomTypeInstance can then be used to provide variables for an expression by adding it to a CatRuntimeContext.
	class CustomTypeInfo: public TypeInfo
	{
	public:
		CustomTypeInfo(const char* typeName, bool isConstType = false);
		virtual ~CustomTypeInfo();

		CustomTypeInstance* createInstance();
		CustomTypeInstance* createInstanceCopy(CustomTypeInstance* source);
		unsigned char* instanceConstructor();
		void instanceDestructor(CustomTypeInstance* instance);
		void instanceDestructor(unsigned char* data);

		//Instances that have been created before members are added will be updated.
		TypeMemberInfo* addFloatMember(const std::string& memberName, float defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addIntMember(const std::string& memberName, int defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addBoolMember(const std::string& memberName, bool defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addStringMember(const std::string& memberName, const std::string& defaultValue, bool isWritable = true, bool isConst = false);
		TypeMemberInfo* addObjectMember(const std::string& memberName, Reflectable* defaulValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics = TypeOwnershipSemantics::Weak, bool isWritable = true, bool isConst = false);

		TypeMemberInfo* addMember(const std::string& memberName, const CatGenericType& type);

		CustomTypeMemberFunctionInfo* addMemberFunction(const std::string& memberFunctionName, const CatGenericType& thisType, AST::CatFunctionDefinition* functionDefinition);

		//This will not shrink the typeSize, only remove the member from the list.
		//The data will only shrink after a restart of the program.
		//Because of this, the CustomTypeInfo remains compatible with existing instances.
		//It is assumed that this does not happen very often.
		void removeMember(const std::string& memberName);
		void renameMember(const std::string& oldMemberName, const std::string& newMemberName);

		//For creating a "static" data type, this instance points directly to the default data.
		CustomTypeInstance* getDefaultInstance();

		virtual bool isCustomType() const;

	private:
		//Returns a pointer to the start of the newly added size
		unsigned char* increaseDataSize(unsigned int amount);
		void increaseDataSize(unsigned char*& data, unsigned int amount, unsigned int currentSize);
		unsigned char* createDataCopy(unsigned char* otherData, unsigned int sizeOfCopy, unsigned int sizeOfSource);

	private:
		CustomTypeInstance* defaultInstance;

		std::set<CustomTypeInstance*> instances;

		bool isConstType;
		unsigned char* defaultData;

		unsigned int typeSize;

		bool isTriviallyCopyable;

		std::vector<std::unique_ptr<TypeMemberInfo>> removedMembers;
	};


} //End namespace jitcat::Reflection