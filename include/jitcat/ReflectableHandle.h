/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


class Reflectable;


class ReflectableHandle
{
public:
	ReflectableHandle(Reflectable* reflectable);
	virtual ~ReflectableHandle();
	//Returns nullptr if reflectable is deleted
	Reflectable* get() const;
	bool getIsValid() const;

	void notifyDeletion();

private:
	Reflectable* reflectable;
};