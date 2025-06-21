/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TypeInfo.h"
#include "jitcat/Configuration.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/JitCat.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/StaticConstMemberInfo.h"
#include "jitcat/StaticMemberInfo.h"
#include "jitcat/StaticMemberFunctionInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/VariableEnumerator.h"

#include <iostream>
#include <sstream>

using namespace jitcat;
using namespace jitcat::Reflection;


TypeInfo::TypeInfo(const char* typeName, std::size_t typeSize, std::unique_ptr<TypeCaster> caster):
	typeName(typeName),
	caster(std::move(caster)),
	parentType(nullptr),
	typeSize(typeSize)
{
	if (JitCat::get()->getHasPrecompiledExpression())
	{
		std::string typeNameGlobal = Tools::append("_TypeInfo:", getTypeName());
		JitCat::get()->setPrecompiledGlobalVariable(typeNameGlobal, reinterpret_cast<uintptr_t>(this));
		std::string typeSizeGLobal = Tools::append("__sizeOf:", getTypeName());
		JitCat::get()->setPrecompiledGlobalVariable(typeSizeGLobal, typeSize);
	}
}


TypeInfo::~TypeInfo()
{
	for (auto& iter : members)
	{
		if ((iter.second->getType().isPointerToReflectableObjectType()
			|| iter.second->getType().isReflectableHandleType())
			&& (Tools::startsWith(iter.first, "$") || iter.second->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value))
		{
			TypeInfo* typeInfo = iter.second->getType().getPointeeType()->getObjectType();
			typeInfo->removeDependentType(this);
		}
	}
	if (parentType != nullptr)
	{
		parentType->removeType(getTypeName());
	}
	for (auto& iter : types)
	{
		iter.second->setParentType(nullptr);
	}
}


void TypeInfo::destroy(TypeInfo* type)
{
	if (type->canBeDeleted())
	{
		delete type;
	}
	else
	{
		typeDeletionList.push_back(type);
	}
	updateTypeDestruction();
}


void TypeInfo::updateTypeDestruction()
{
	bool foundDeletion = false;
	do
	{
		foundDeletion = false;
		int numTypes = (int)typeDeletionList.size();

		//Reverse iterate because that is typically the order in which dependent types are deleted.
		for (int i = numTypes - 1; i >= 0; i--)
		{
			if (typeDeletionList[i]->canBeDeleted())
			{
				delete typeDeletionList[i];
				typeDeletionList[i] = typeDeletionList.back();
				typeDeletionList.pop_back();
				foundDeletion = true;
			}
		}
	} while (foundDeletion);
}


bool TypeInfo::addType(TypeInfo* type)
{
	std::string lowercaseTypeName = Tools::toLowerCase(type->getTypeName());
	if (types.find(lowercaseTypeName) == types.end())
	{
		types[lowercaseTypeName] = type;
		type->setParentType(this);
		return true;
	}
	return false;
}


StaticConstMemberInfo* TypeInfo::addConstant(const std::string& name, const CatGenericType& type, const std::any& value)
{
	std::string lowercaseTypeName = Tools::toLowerCase(name);
	if (staticConstMembers.find(lowercaseTypeName) == staticConstMembers.end())
	{
		staticConstMembers.emplace(std::make_pair(lowercaseTypeName, std::make_unique<StaticConstMemberInfo>(name, type, value)));
		return staticConstMembers[lowercaseTypeName].get();
	}
	return nullptr;
}


void TypeInfo::setParentType(TypeInfo* type)
{
	parentType = type;
}


bool TypeInfo::removeType(const std::string& typeName)
{
	auto iter = types.find(Tools::toLowerCase(typeName));
	if (iter != types.end())
	{
		iter->second->setParentType(nullptr);
		types.erase(iter);
		return true;
	}
	return false;
}


void TypeInfo::addDeserializedMember(TypeMemberInfo* memberInfo)
{
	std::string lowerCaseMemberName = Tools::toLowerCase(memberInfo->getMemberName());
	members.emplace(lowerCaseMemberName,  memberInfo);
}


void TypeInfo::addDeserializedStaticMember(StaticMemberInfo* staticMemberInfo)
{
	std::string lowerCaseMemberName = Tools::toLowerCase(staticMemberInfo->memberName);
	staticMembers.emplace(lowerCaseMemberName,  staticMemberInfo);
}


void TypeInfo::addDeserializedMemberFunction(MemberFunctionInfo* memberFunction)
{
	std::string lowerCaseMemberFunctionName = Tools::toLowerCase(memberFunction->getMemberFunctionName());
	memberFunctions.emplace(lowerCaseMemberFunctionName, memberFunction);
}


void TypeInfo::addDeserializedStaticMemberFunction(StaticFunctionInfo* staticFunction)
{
	std::string lowerCaseMemberFunctionName = Tools::toLowerCase(staticFunction->getNormalFunctionName());
	staticFunctions.emplace(lowerCaseMemberFunctionName, staticFunction);
}


std::size_t TypeInfo::getTypeSize() const
{
	return typeSize;
}


const CatGenericType& TypeInfo::getType(const std::string& dotNotation) const
{
	std::vector<std::string> indirectionList;
	Tools::split(dotNotation, ".", indirectionList, false);
	return getType(indirectionList, 0);
}


const CatGenericType& TypeInfo::getType(const std::vector<std::string>& indirectionList, int offset) const
{
	int indirectionListSize = (int)indirectionList.size();
	if (indirectionListSize > 0)
	{
		std::map<std::string, std::unique_ptr<TypeMemberInfo>>::const_iterator iter = members.find(indirectionList[offset]);
		if (iter != members.end())
		{
			TypeMemberInfo* memberInfo = iter->second.get();
			if (memberInfo->getType().isBasicType())
			{
				if (offset == indirectionListSize - 1)
				{
					return memberInfo->getType();
				}
			}
			else if (memberInfo->getType().isPointerToReflectableObjectType())
			{
				if (indirectionListSize > offset + 1)
				{
					return memberInfo->getType().getPointeeType()->getObjectType()->getType(indirectionList, offset + 1);
				}
			}
		}
	}
	return CatGenericType::unknownType;
}


TypeMemberInfo* TypeInfo::getMemberInfo(const std::string& identifier) const
{
	auto iter = members.find(Tools::toLowerCase(identifier));
	if (iter != members.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}


StaticMemberInfo* TypeInfo::getStaticMemberInfo(const std::string& identifier) const
{
	auto iter = staticMembers.find(Tools::toLowerCase(identifier));
	if (iter != staticMembers.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}


StaticConstMemberInfo* TypeInfo::getStaticConstMemberInfo(const std::string& identifier) const
{
	auto iter = staticConstMembers.find(Tools::toLowerCase(identifier));
	if (iter != staticConstMembers.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}


MemberFunctionInfo* TypeInfo::getFirstMemberFunctionInfo(const std::string& identifier) const
{
	auto iter = memberFunctions.find(Tools::toLowerCase(identifier));
	if (iter != memberFunctions.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}


std::vector<MemberFunctionInfo*> jitcat::Reflection::TypeInfo::getMemberFunctionsByName(const std::string& functionName) const
{
	std::vector<MemberFunctionInfo*> foundFunctions;
	std::string lowerCaseFunctionName = Tools::toLowerCase(functionName);
	auto lowerBound = memberFunctions.lower_bound(lowerCaseFunctionName);
	auto upperBound = memberFunctions.upper_bound(lowerCaseFunctionName);
	for (auto& iter = lowerBound; iter != upperBound; ++iter)
	{
		foundFunctions.push_back(iter->second.get());
	}
	return foundFunctions;
}


MemberFunctionInfo* TypeInfo::getMemberFunctionInfo(const FunctionSignature* functionSignature) const
{
	return getMemberFunctionInfo(*functionSignature);
}


MemberFunctionInfo* TypeInfo::getMemberFunctionInfo(const FunctionSignature& functionSignature) const
{
	std::string lowerCaseFunctionName = functionSignature.getLowerCaseFunctionName();
	auto lowerBound = memberFunctions.lower_bound(lowerCaseFunctionName);
	auto upperBound = memberFunctions.upper_bound(lowerCaseFunctionName);
	for (auto& iter = lowerBound; iter != upperBound; ++iter)
	{
		if (iter->second->compare(functionSignature))
		{
			return iter->second.get();
		}
	}
	return nullptr;
}


StaticFunctionInfo* TypeInfo::getFirstStaticMemberFunctionInfo(const std::string& identifier) const
{
	auto iter = staticFunctions.find(Tools::toLowerCase(identifier));
	if (iter != staticFunctions.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}


StaticFunctionInfo* TypeInfo::getStaticMemberFunctionInfo(const FunctionSignature* functionSignature) const
{
	return getStaticMemberFunctionInfo(*functionSignature);
}


StaticFunctionInfo* TypeInfo::getStaticMemberFunctionInfo(const FunctionSignature& functionSignature) const
{
	std::string lowerCaseFunctionName = functionSignature.getLowerCaseFunctionName();
	auto lowerBound = staticFunctions.lower_bound(lowerCaseFunctionName);
	auto upperBound = staticFunctions.upper_bound(lowerCaseFunctionName);
	for (auto& iter = lowerBound; iter != upperBound; ++iter)
	{
		if (iter->second->compare(functionSignature))
		{
			return iter->second.get();
		}
	}
	return nullptr;
}


TypeInfo* TypeInfo::getTypeInfo(const std::string& typeName) const
{
	auto iter = types.find(Tools::toLowerCase(typeName));
	if (iter != types.end())
	{
		return iter->second;
	}
	else
	{
		return nullptr;
	}
}


const char* TypeInfo::getTypeName() const
{
	return typeName;
}


std::string jitcat::Reflection::TypeInfo::getQualifiedTypeName() const
{
	if (parentType != nullptr)
	{
		return Tools::append(parentType->getQualifiedTypeName(), "::", typeName);
	}
	else
	{
		return typeName;
	}
}


void TypeInfo::setTypeName(const char* newTypeName)
{
	typeName = newTypeName;
}


void TypeInfo::enumerateVariables(VariableEnumerator* enumerator, bool allowEmptyStructs) const
{
	for (auto& iter : memberFunctions)
	{
		std::stringstream result;
		result << iter.second->getMemberFunctionName();
		result << "(";
		std::size_t numArguments = iter.second->getNumberOfArguments();
		for (std::size_t i = 0; i < numArguments; i++)
		{
			if (i > 0)
			{
				result << ", ";
			}
			result << iter.second->getArgumentType(i).toString();
		}
		result << ")";
		enumerator->addFunction(iter.second->getMemberFunctionName(), result.str());
	}

	auto last = members.end();
	for (auto iter = members.begin(); iter != last; ++iter)
	{
		const CatGenericType& memberType = iter->second->getType();
		if (memberType.isBasicType() || memberType.isStringType() || memberType.isEnumType())
		{
			std::string catTypeName = memberType.toString();
			enumerator->addVariable(iter->second->getMemberName(), catTypeName, iter->second->getType().isWritable(), iter->second->getType().isConst());
		}
		else if (memberType.isPointerToReflectableObjectType() || memberType.isReflectableHandleType())
		{
			std::string nestedTypeName = memberType.toString();
			if (allowEmptyStructs || memberType.getPointeeType()->getObjectType()->getMembers().size() > 0)
			{
				enumerator->enterNameSpace(iter->second->getMemberName(), nestedTypeName, NamespaceType::Object);
				if (!Tools::isInList(enumerator->loopDetectionTypeStack, nestedTypeName))
				{
					enumerator->loopDetectionTypeStack.push_back(nestedTypeName);
					memberType.getPointeeType()->getObjectType()->enumerateVariables(enumerator, allowEmptyStructs);
					enumerator->loopDetectionTypeStack.pop_back();
				}
				enumerator->exitNameSpace();
					
			}
		}
	}

	auto constLast = staticConstMembers.end();
	for (auto iter = staticConstMembers.begin(); iter != constLast; ++iter)
	{
		enumerator->addConstant(iter->second->getName(), getTypeName(), iter->second->getType().toString());
	}

	auto staticLast = staticMembers.end();
	for (auto iter = staticMembers.begin(); iter != staticLast; ++iter)
	{
		const CatGenericType& memberType = iter->second->catType;
		if (memberType.isBasicType() || memberType.isStringType() || memberType.isEnumType())
		{
			std::string catTypeName = memberType.toString();
			enumerator->addStaticVariable(iter->second->memberName, catTypeName, iter->second->catType.isWritable(), iter->second->catType.isConst());
		}
		else if (memberType.isPointerToReflectableObjectType() || memberType.isReflectableHandleType())
		{
			std::string nestedTypeName = memberType.toString();
			if (allowEmptyStructs || memberType.getPointeeType()->getObjectType()->getMembers().size() > 0)
			{
				enumerator->enterNameSpace(iter->second->memberName, nestedTypeName, NamespaceType::Object);
				if (!Tools::isInList(enumerator->loopDetectionTypeStack, nestedTypeName))
				{
					enumerator->loopDetectionTypeStack.push_back(nestedTypeName);
					memberType.getPointeeType()->getObjectType()->enumerateVariables(enumerator, allowEmptyStructs);
					enumerator->loopDetectionTypeStack.pop_back();
				}
				enumerator->exitNameSpace();
					
			}
		}
	}
}


void TypeInfo::enumerateMemberVariables(std::function<void(const CatGenericType&, const std::string&)>& enumerator) const
{
	for (auto& iter : membersByOrdinal)
	{
		enumerator(iter.second->getType(), iter.second->getMemberName());
	}
}


bool TypeInfo::isCustomType() const
{
	return false;
}


bool TypeInfo::isReflectedType() const
{
	return false;
}


bool TypeInfo::isArrayType() const
{
	return false;
}


bool TypeInfo::isTriviallyCopyable() const
{
	return false;
}


bool Reflection::TypeInfo::isTriviallyConstructable() const
{
	return false;
}


const std::map<std::string, std::unique_ptr<TypeMemberInfo>>& TypeInfo::getMembers() const
{
	return members;
}


const std::map<std::string, std::unique_ptr<StaticMemberInfo>>& jitcat::Reflection::TypeInfo::getStaticMembers() const
{
	return staticMembers;
}


const std::map<unsigned long long, TypeMemberInfo*>& jitcat::Reflection::TypeInfo::getMembersByOrdinal() const
{
	return membersByOrdinal;
}


const std::multimap<std::string, std::unique_ptr<MemberFunctionInfo>>& TypeInfo::getMemberFunctions() const
{
	return memberFunctions;
}


const std::multimap<std::string, std::unique_ptr<StaticFunctionInfo>>& TypeInfo::getStaticMemberFunctions() const
{
	return staticFunctions;
}


const std::map<std::string, TypeInfo*>& TypeInfo::getTypes() const
{
	return types;
}


const TypeCaster* TypeInfo::getTypeCaster() const
{
	return caster.get();
}


void TypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	assert(false);
}


unsigned char* TypeInfo::construct() const
{
	std::size_t typeSize = getTypeSize();
	unsigned char* buffer = new unsigned char[typeSize];
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		std::cout << "(TypeInfo::construct) Allocated buffer of size " << std::dec << typeSize << ": " << std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
	}
	placementConstruct(buffer, typeSize);
	return buffer;
}


void TypeInfo::destruct(unsigned char* object)
{
	placementDestruct(object, getTypeSize());
	delete[] object;
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		std::cout << "(TypeInfo::destruct) Deallocated buffer of size " << std::dec << typeSize << ": " << std::hex << reinterpret_cast<uintptr_t>(object) << "\n";
	}
}


void TypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
	assert(false);
}


void TypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(false);
}


void TypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(false);
}


void TypeInfo::toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const
{
	if (caster != nullptr)
	{
		caster->toBuffer(value, buffer, bufferSize);
	}
	else
	{
		assert(false);
	}
}


bool TypeInfo::getAllowInheritance() const
{
	return true;
}


bool TypeInfo::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
	return true;
}


bool TypeInfo::getAllowConstruction() const
{
	return true;
}


bool TypeInfo::getAllowCopyConstruction() const
{
	return true;
}


bool TypeInfo::getAllowMoveConstruction() const
{
	return true;
}


bool TypeInfo::canBeDeleted() const
{
	return dependentTypes.size() == 0;
}


bool jitcat::Reflection::TypeInfo::canBeAssignedBy(const CatGenericType& type) const
{
	SearchFunctionSignature searchSignature("=", {type.removeIndirection().toPointer()});
	return getMemberFunctionInfo(searchSignature) != nullptr;
}


void TypeInfo::addDependentType(TypeInfo* otherType)
{
	assert(otherType != this);
	if (dependentTypes.find(otherType) == dependentTypes.end())
	{
		dependentTypes.insert(otherType);
	}
}


void TypeInfo::renameMember(const std::string& oldMemberName, const std::string& newMemberName)
{
	auto iter = members.find(Tools::toLowerCase(oldMemberName));
	if (iter != members.end() && members.find(Tools::toLowerCase(newMemberName)) == members.end() && !iter->second->isDeferred())
	{
		std::unique_ptr<TypeMemberInfo> memberInfo = std::move(iter->second);
		memberInfo->setMemberName(newMemberName);
		members.erase(iter);
		std::string lowerCaseMemberName = Tools::toLowerCase(newMemberName);
		members.emplace(lowerCaseMemberName, std::move(memberInfo));
	}
}


void TypeInfo::removeDependentType(TypeInfo* otherType)
{
	auto iter = dependentTypes.find(otherType);
	if (iter != dependentTypes.end())
	{
		dependentTypes.erase(iter);
	}
}


void TypeInfo::addDeferredMembers(TypeMemberInfo* deferredMember)
{
	auto& deferredMembers = deferredMember->getType().getPointeeType()->getObjectType()->getMembers();
	auto& deferredMemberFunctions = deferredMember->getType().getPointeeType()->getObjectType()->getMemberFunctions();

	for (auto& member : deferredMembers)
	{
		if (member.second->getMemberVisibility() == MemberVisibility::Public
			|| member.second->getMemberVisibility() == MemberVisibility::Protected)
		{
			members.emplace(member.first, member.second->toDeferredTypeMemberInfo(deferredMember));
		}
	}
	for (auto& memberFunction : deferredMemberFunctions)
	{
		if (memberFunction.second->getVisibility() == MemberVisibility::Public
			|| memberFunction.second->getVisibility() == MemberVisibility::Protected)
		{
			memberFunctions.emplace(memberFunction.first, memberFunction.second->toDeferredMemberFunction(deferredMember, this));
		}
	}
}


void TypeInfo::addMember(const std::string& memberName, TypeMemberInfo* memberInfo)
{
	membersByOrdinal[memberInfo->getOrdinal()] = memberInfo;
	members.emplace(memberName, memberInfo);
}


TypeMemberInfo* TypeInfo::releaseMember(const std::string& memberName)
{
	auto iter = members.find(memberName);
	if (iter != members.end())
	{
		TypeMemberInfo* memberInfo = iter->second.release();

		members.erase(iter);

		auto oridinalIter = membersByOrdinal.find(memberInfo->getOrdinal());
		if (oridinalIter != membersByOrdinal.end())
		{
			membersByOrdinal.erase(oridinalIter);
		}

		return memberInfo;
	}
	return nullptr;
}


std::vector<TypeInfo*> TypeInfo::typeDeletionList = std::vector<TypeInfo*>();