/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "MemberReferencePtr.h"
#include "MemberReference.h"

#include <stddef.h>


MemberReferencePtr::MemberReferencePtr(MemberReference* instance):
	instance(instance),
	original(nullptr)
{
	if (instance != nullptr)
	{
		instance->incrementReferenceCounter();
	}
}


MemberReferencePtr::MemberReferencePtr(const MemberReferencePtr& other):
	instance(other.instance),
	original(other.original)
{
	if (instance != nullptr)
	{
		instance->incrementReferenceCounter();
	}
}


MemberReferencePtr::MemberReferencePtr(const MemberReferencePtr& other, MemberReferencePtr* orignal_):
	instance(other.instance),
	original(orignal_)
{
	if (instance != nullptr)
	{
		instance->incrementReferenceCounter();
	}
}


MemberReferencePtr::~MemberReferencePtr()
{
	release();
}


MemberReferencePtr& MemberReferencePtr::operator=(const MemberReferencePtr& other)
{
	if (other.instance != instance)
	{
		release();
		instance = other.instance;
		if (instance != nullptr)
		{
			instance->incrementReferenceCounter();
		}
	}
	return *this;
}


MemberReferencePtr& MemberReferencePtr::operator=(MemberReference* otherInstance)
{
	if (instance != otherInstance)
	{
		release();
		instance = otherInstance;
		if (instance != nullptr)
		{
			instance->incrementReferenceCounter();
		}
	}
	return *this;
}


MemberReference* MemberReferencePtr::getPointer() const
{
	return instance;
}


MemberReference* MemberReferencePtr::operator->()
{
	return instance;
}


const MemberReference* MemberReferencePtr::operator->() const
{
	return instance;
}


bool MemberReferencePtr::operator==(const MemberReferencePtr& other) const
{
	return instance == other.instance;
}


bool MemberReferencePtr::operator==(const MemberReference* otherInstance) const
{
	return instance == otherInstance;
}


bool MemberReferencePtr::operator!=(const MemberReferencePtr & other) const
{
	return instance != other.instance;
}


bool MemberReferencePtr::operator!=(const MemberReference* otherInstance) const
{
	return instance != otherInstance;
}


bool MemberReferencePtr::isNull() const
{
	return instance == nullptr;
}


void MemberReferencePtr::setOriginalReference(MemberReferencePtr* original_)
{
	original = original_;
}


MemberReferencePtr* MemberReferencePtr::getOriginalReference() const
{
	return original;
}


void MemberReferencePtr::release()
{
	if (instance != nullptr)
	{
		if (!instance->decrementReferenceCounter())
		{
			delete instance;
			instance = nullptr;
		}
	}
}
