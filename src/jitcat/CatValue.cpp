/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatLog.h"
#include "CatValue.h"
#include "MemberReference.h"
#include <cstdlib>
#include <iostream>
#include <sstream>


CatValue::CatValue(const CatValue& other):
	memberReference(nullptr)
{
	type = other.type;
	switch (type)
	{
		case CatType::Float:	floatValue = other.floatValue;						break;
		case CatType::Int:		intValue = other.intValue;							break;
		case CatType::Bool:	boolValue = other.boolValue;						break;
		case CatType::String:	stringValue = new std::string(*other.stringValue);	break;
		case CatType::Object:	memberReference = other.memberReference; intValue = 0;	break;
		case CatType::Error:	errorValue = new CatError(other.errorValue->message); break;
	}
}

CatValue::~CatValue()
{
	if (type == CatType::String) 
	{
		delete stringValue; 
	} 
	else if (type == CatType::Error) 
	{ 
		delete errorValue; 
	}
}


CatType CatValue::getValueType() const
{
	if (type != CatType::Object)
	{
		return type;
	}
	else if (memberReference.isNull())
	{
		return CatType::Error;
	}
	else
	{
		return memberReference->getCatType();
	}
}

float CatValue::getFloatValue() const
{
	if (type != CatType::Object)
	{
		return floatValue;
	}
	else if (memberReference.isNull())
	{
		return 0.0f;
	}
	else
	{
		return memberReference->getFloat();
	}
}


int CatValue::getIntValue() const
{
	if (type != CatType::Object)
	{
		return intValue;
	}
	else if (memberReference.isNull())
	{
		return 0;
	}
	else
	{
		return memberReference->getInt();
	}
}


bool CatValue::getBoolValue() const
{
	if (type != CatType::Object)
	{
		return boolValue;
	}
	else if (memberReference.isNull())
	{
		return false;
	}
	else
	{
		return memberReference->getBool();
	}
}


const std::string& CatValue::getStringValue() const
{
	if (type != CatType::Object)
	{
		return *stringValue;
	}
	else if (memberReference.isNull())
	{
		return Tools::empty;
	}
	else
	{
		return memberReference->getString();
	}
}

const CatError& CatValue::getErrorValue() const
{
	if (type != CatType::Object)
	{
		return *errorValue;
	}
	else
	{
		return CatError::defaultError;
	}
}

bool CatValue::isReference() const
{
	return type == CatType::Object;
}


int CatValue::toIntValue() const
{
	switch (type)
	{
		case CatType::Float:	return (int)floatValue;
		case CatType::Int:		return intValue;
		case CatType::Bool:	return boolValue ? 1 : 0;
		case CatType::String:	return atoi(stringValue->c_str());
		case CatType::Object:	
		{
			if (memberReference.isNull())
			{
				return 0;
			}
			switch (memberReference->getCatType())
			{
				case CatType::Float:	return (int)memberReference->getFloat();
				case CatType::Int:		return memberReference->getInt();
				case CatType::Bool:	return memberReference->getBool() ? 1 : 0;
				case CatType::String:	return atoi(memberReference->getString().c_str());
				default:		return 0;
			}
		}
		default:		return 0;
	}
}


float CatValue::toFloatValue() const
{
	switch (type)
	{
		case CatType::Float:	return floatValue;
		case CatType::Int:		return (float)intValue;
		case CatType::Bool:	return boolValue ? 1.0f : 0.0f;
		case CatType::String:	return (float)atof(stringValue->c_str());
		case CatType::Object:
		{
			if (memberReference.isNull())
			{
				return 0.0f;
			}
			switch (memberReference->getCatType())
			{
				case CatType::Float:	return memberReference->getFloat();
				case CatType::Int:		return (float)memberReference->getInt();
				case CatType::Bool:	return memberReference->getBool() ? 1.0f : 0.0f;
				case CatType::String:	return (float)atof(memberReference->getString().c_str());
				default:		return 0;
			}
		}
		default:		return 0.0f;
	}
}


bool CatValue::toBoolValue() const
{
	switch (type)
	{
		case CatType::Float:	return floatValue > 0.0f;
		case CatType::Int:		return intValue > 0;
		case CatType::Bool:	return boolValue;
		case CatType::String:	return *stringValue == "true" || atoi(stringValue->c_str()) > 0;
		case CatType::Object:
		{
			if (memberReference.isNull())
			{
				return 0.0f;
			}
			switch (memberReference->getCatType())
			{
				case CatType::Float:	return memberReference->getFloat() > 0.0f;
				case CatType::Int:		return memberReference->getInt() > 0;
				case CatType::Bool:	return memberReference->getBool();
				case CatType::String:
				{
					const std::string& value = memberReference->getString();
					return value == "true" || atoi(value.c_str()) > 0;
				}
				default:		return 0;
			}
		}
		default:		return false;
	}
}


std::string CatValue::toStringValue() const
{
	switch (type)
	{
		case CatType::Float: 
		{
			std::ostringstream ss;
			ss << floatValue;
			return std::string(ss.str());
		}
		case CatType::Int:
		{
			std::ostringstream ss;
			ss << intValue;
			return std::string(ss.str());
		}
		case CatType::Bool:	return boolValue ? "1" : "0";
		case CatType::String:	return *stringValue;
		case CatType::Object:
		{
			if (memberReference.isNull())
			{
				return "";
			}
			switch (memberReference->getCatType())
			{
				case CatType::Float:	
				{
					std::ostringstream ss;
					ss << memberReference->getFloat();
					return std::string(ss.str());
				}
				case CatType::Int:
				{
					std::ostringstream ss;
					ss << memberReference->getInt();
					return std::string(ss.str());
				}
				case CatType::Bool:	return memberReference->getBool() ? "1" : "0";
				case CatType::String:	return memberReference->getString();
				default:		return "";
			}
		}
		default:		return "";
	}
}


CatGenericType CatValue::getGenericType() const
{
	switch (type)
	{
		case CatType::Int:
		case CatType::Float:
		case CatType::String:
		case CatType::Bool:
		case CatType::Void:
			return CatGenericType(type);
		case CatType::Object:
			if (!memberReference.isNull())
			{
				return memberReference->getGenericType();
			}
			else
			{
				return  CatGenericType("Error");
			}
		case CatType::Error:
			return CatGenericType(errorValue->message);
		case CatType::Unknown:
		default:
			return  CatGenericType("Error");
	}
}


CatValue& CatValue::operator=(const CatValue& other)
{
	if (type == CatType::String)
	{
		delete stringValue;
	}
	else if (type == CatType::Error)
	{
		delete errorValue;
	}
	type = other.type;
	switch (type)
	{
		case CatType::Float:	floatValue = other.floatValue;	break;
		case CatType::Int:		intValue = other.intValue;		break;
		case CatType::Bool:	boolValue = other.boolValue;	break;
		case CatType::String:	stringValue = new std::string(*other.stringValue);	  break;
		case CatType::Error:	errorValue = new CatError(other.errorValue->message); break;
		case CatType::Object: memberReference = other.memberReference; intValue = 0; break;
	}
	return *this;
}


void CatValue::printValue() const
{
	switch (type)
	{
		default:
		case CatType::Void:
			CatLog::log("void");
			break;
		case CatType::Float:
			CatLog::log(floatValue);
			break;
		case CatType::Int:
			CatLog::log(intValue);
			break;
		case CatType::Bool:
			if (boolValue)
			{
				CatLog::log("true");
			}
			else
			{
				CatLog::log("false");
			}
			break;
		case CatType::String:
			CatLog::log("\"");
			CatLog::log((*stringValue).c_str());
			CatLog::log("\"");
			break;
		case CatType::Object:
			CatLog::log("object: ");
			if (!memberReference.isNull())
			{
				switch (memberReference->getCatType())
				{
					case CatType::Float:	CatLog::log(memberReference->getFloat()); break;
					case CatType::Int:		CatLog::log(memberReference->getInt());	break;
					case CatType::Bool:	CatLog::log(memberReference->getBool() ? "true" : "false"); break;
					case CatType::String:	CatLog::log("\""); CatLog::log(memberReference->getString()); CatLog::log("\""); break;
					case CatType::Object:
					{
						CatLog::log(memberReference->getCustomTypeName()); 
						CatLog::log(":");
						std::stringstream stream;
						stream << std::hex << memberReference->getParentObject();
						CatLog::log(stream.str());
						break;
					}
					default:		CatLog::log("?"); break;
				}
			}
			else
			{
				CatLog::log("null");
			}

			break;
		case CatType::Error:
			CatLog::log("ERROR: ");
			if (errorValue != nullptr)
			{
				CatLog::log(errorValue->message.c_str());
			}
			break;
	}
}
