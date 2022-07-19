/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


namespace jitcat::Reflection
{
	//Instances of a CustomTypeInfo are stored in a ReflectableHandle.
	//These ReflectableHandles form a linked list per object instance. 
	//This way, an object instance can know about all handles that point to it.
	//This enum defines where the pointer to the first ReflectableHandle is stored.
	enum class HandleTrackingMethod
	{
		//The pointer to the first ReflectableHandle is stored inside each object instance.
		//This uses some extra memory per object, but is the fastest method.
		InternalHandlePointer,
		//The pointer to the first ReflectableHandle is stored inside an external unordered_map.
		//The object will be smaller, but adding and removing handles is slower.
		ExternallyTracked,
		//Instances of this CustomTypeInfo are not tracked at all.
		//Adding or removing members after instances have already been constructed will not update existing instances and will likely cause issues.
		None
	};
}