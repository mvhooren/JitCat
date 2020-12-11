#include "jitcat/ArrayMemberFunctionInfo.h"
#include "jitcat/ArrayTypeInfo.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::Reflection;


const char* ArrayMemberFunctionInfo::toString(Operation operation)
{
	switch (operation)
	{
		default: assert(false); return "none";
		case Operation::Add:	return "add";
		case Operation::Index:	return "[]";
		case Operation::Remove:	return "remove";
		case Operation::Size:	return "size";
	}
}


ArrayMemberFunctionInfo::ArrayMemberFunctionInfo(Operation operation, ArrayTypeInfo* arrayTypeInfo):
	MemberFunctionInfo(toString(operation), CatGenericType::voidType),
	operation(operation),
	arrayTypeInfo(arrayTypeInfo)
{
	
	switch (operation)
	{
		case Operation::Add:
		{
			returnType = CatGenericType::intType;
			if (!arrayTypeInfo->getArrayItemType().isReflectableObjectType())
			{
				argumentTypes.push_back(arrayTypeInfo->getArrayItemType());
			}
			else
			{
				argumentTypes.push_back(arrayTypeInfo->getArrayItemType().toPointer(TypeOwnershipSemantics::Weak, false, false));
			}
			
		} break;
		case Operation::Index:
		{
			returnType = arrayTypeInfo->getArrayItemType().toPointer(TypeOwnershipSemantics::Weak, true, false);
			argumentTypes.push_back(CatGenericType::intType);
		} break;
		case Operation::Remove:
		{
			returnType = CatGenericType::voidType;
			argumentTypes.push_back(CatGenericType::intType);
		} break;
		case Operation::Size:
		{
			returnType = CatGenericType::intType;
		} break;
	}
}


std::any ArrayMemberFunctionInfo::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
{ 
	if (base.has_value())
	{
		ArrayTypeInfo::Array* baseArray = std::any_cast<ArrayTypeInfo::Array*>(base);
		switch (operation)
		{
			default:				assert(false); return std::any();
			case Operation::Remove:
			{
				assert(parameters.size() == 1);
				arrayTypeInfo->remove(baseArray, std::any_cast<int>(parameters[0]));
				return std::any();
			}
			case Operation::Add:	
			{
				assert(parameters.size() == 1);
				return arrayTypeInfo->add(baseArray, parameters[0]);
			}
			case Operation::Index:
			{
				assert(parameters.size() == 1);
				return arrayTypeInfo->index(baseArray, std::any_cast<int>(parameters[0]));
			}
			case Operation::Size:
			{
				assert(parameters.size() == 0);
				if (baseArray != nullptr)
				{
					return baseArray->size;
				}
				else
				{
					return 0;
				}
			}
		}			
		
	}
	switch (operation)
	{
		default: assert(false);	return std::any();
		case Operation::Add:	return std::any();
		case Operation::Index:	return arrayTypeInfo->getArrayItemType().createDefault();
		case Operation::Remove: return std::any();
		case Operation::Size:	return std::any((int)0);
	}
}


MemberFunctionCallData ArrayMemberFunctionInfo::getFunctionAddress() const
{
	//QQQ add an inline member function call data type and generate array functions
	return MemberFunctionCallData();
}