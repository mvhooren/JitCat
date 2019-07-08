#include "jitcat/ArrayMemberFunctionInfo.h"
#include "jitcat/ContainerManipulator.h"

#include <cassert>

using namespace jitcat::Reflection;


const char* jitcat::Reflection::ArrayTypeMemberFunctionInfo::toString(Operation operation)
{
	switch (operation)
	{
		default: assert(false); return "none";
		case Operation::Add:	return "add";
		case Operation::Remove:	return "remove";
		case Operation::Size:	return "size";
	}
}


jitcat::Reflection::ArrayTypeMemberFunctionInfo::ArrayTypeMemberFunctionInfo(Operation operation, const CatGenericType& arrayType):
	MemberFunctionInfo(toString(operation), CatGenericType::voidType),
	operation(operation),
	arrayType(arrayType),
	arrayManipulator(static_cast<ArrayManipulator*>(arrayType.getObjectType()))
{
	
	switch (operation)
	{
		case Operation::Add:
		{
			returnType = CatGenericType::intType;
			if (!arrayManipulator->getValueType().isReflectableObjectType())
			{
				argumentTypes.push_back(arrayManipulator->getValueType());
			}
			else
			{
				argumentTypes.push_back(arrayManipulator->getValueType().toPointer(TypeOwnershipSemantics::Owned, false, false));
			}
			
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


std::any jitcat::Reflection::ArrayTypeMemberFunctionInfo::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters)
{ 
	if (base.has_value())
	{
		ArrayManipulator::Array* baseArray = static_cast<ArrayManipulator::Array*>(std::any_cast<Reflectable*>(base));
		switch (operation)
		{
			default:				assert(false); return std::any();
			case Operation::Remove:
			{
				assert(parameters.size() == 1);
				arrayManipulator->remove(baseArray, std::any_cast<int>(parameters[0]));
				return std::any();
			} break;
			case Operation::Add:	
			{
				assert(parameters.size() == 1);
				return arrayManipulator->add(baseArray, parameters[0]);
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
		default:				assert(false);
		case Operation::Remove:
		case Operation::Add:	return std::any();
		case Operation::Size:	return std::any((int)0);
	}
}


MemberFunctionCallData jitcat::Reflection::ArrayTypeMemberFunctionInfo::getFunctionAddress() const
{
	return MemberFunctionCallData();
}
;
