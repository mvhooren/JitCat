/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ReflectableHandle;
class TypeInfo;

#include <vector>
#include <string>


class Reflectable
{
private:
	//disable copying, this is usually a bad idea because of the observers list
	Reflectable(const Reflectable& );
public:
	Reflectable();
	virtual ~Reflectable();

	void addObserver(ReflectableHandle* observer);
	void removeObserver(ReflectableHandle* observer);

	virtual void copyFrom(const Reflectable* other) {}

private:
	std::vector<ReflectableHandle*> observers;
};