/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatGenericType.h"
class MemberReference;

//A MemberReferencePtr uses a reference counter to store the number of times the MemberReference is referenced.
//Once the reference counter reaches zero, the MemberReference is automatically deleted.
class MemberReferencePtr
{
public:
	MemberReferencePtr(MemberReference* instance = 0);
	MemberReferencePtr(const MemberReferencePtr& other);
	MemberReferencePtr(const MemberReferencePtr& other, MemberReferencePtr* orignal_);
	~MemberReferencePtr();
	MemberReferencePtr& operator=(const MemberReferencePtr& other);
	MemberReferencePtr& operator=(MemberReference* otherInstance);
	MemberReference* getPointer() const;
	MemberReference* operator->();
	const MemberReference* operator->() const;
	bool operator==(const MemberReferencePtr& other) const;
	bool operator==(const MemberReference* otherInstance) const;
	bool operator!=(const MemberReferencePtr& other) const;
	bool operator!=(const MemberReference* otherInstance) const;
	bool isNull() const;

	void setOriginalReference(MemberReferencePtr* original_);
	MemberReferencePtr* getOriginalReference() const;

private:
	void release();

private:
	MemberReference* instance;
	//This is a pointer to the original member reference that this is a copy of.
	//This exists ONLY for enabling assigning of the orignal reference instead of to the instance directly.
	MemberReferencePtr* original;
};

