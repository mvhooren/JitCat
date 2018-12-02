#include "TypeTraits.h"
#include "MemberReference.h"


float TypeTraits<float>::getValueFromMemberReference(MemberReference* value)
{ 
	if (value != nullptr
		&& value->getCatType() == CatType::Float)
	{
		return value->getFloat();
	}
	return 0.0f;
}


int TypeTraits<int>::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::Int)
	{
		return value->getInt();
	}
	return 0;
}


bool TypeTraits<bool>::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::Bool)
	{
		return value->getBool();
	}
	return false;
}


std::string TypeTraits<std::string>::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::String)
	{
		return value->getString();
	}
	return "";
}