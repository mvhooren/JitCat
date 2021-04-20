/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ExternalReflector.h"
#include "jitcat/HandleTrackingMethod.h"

#include <unordered_map>


namespace jitcat::Reflection
{
	class CustomTypeInfo;
	class TypeInfo;


	class ReflectableHandle
	{
	public:
		ReflectableHandle();
		ReflectableHandle(unsigned char* reflectable, TypeInfo* reflectableType);
		ReflectableHandle(const ReflectableHandle& other);
		ReflectableHandle(const ReflectableHandle&& other) = delete;
		~ReflectableHandle();
		//Returns nullptr if reflectable is deleted
		unsigned char* get() const;
		bool getIsValid() const;
		ReflectableHandle& operator=(const ReflectableHandle& other);
		void setReflectable(unsigned char* reflectable, TypeInfo* reflectableType);
		ReflectableHandle& operator=(std::nullptr_t other);

		//Searches through the handles to oldObject and updates them to point to newObject.
		static void replaceCustomObjects(unsigned char* oldObject, CustomTypeInfo* oldType, unsigned char* newObject, CustomTypeInfo* newType);
		//Will set all the handles that point to object to nullptr, and will unlinks them.
		static void nullifyObjectHandles(unsigned char* object, CustomTypeInfo* objectType);

		TypeInfo* getObjectType() const;
		ReflectableHandle* getNextHandle() const;
		void setNextHandle(ReflectableHandle* next);
		ReflectableHandle* getPreviousHandle() const;
		void setPreviousHandle(ReflectableHandle* previous);

		void setReflectableReplacement(unsigned char* reflectable, TypeInfo* reflectableType);

		bool validateHandles() const;

	private:
		void registerObserver();
		void deregisterObserver();

		static void replaceFirstHandle(ReflectableHandle** handle, ReflectableHandle* replacement);
		
		static ReflectableHandle* getFirstHandle(unsigned char* object, HandleTrackingMethod trackingMethod);
		static void setFirstHandle(unsigned char* object, ReflectableHandle* handle, HandleTrackingMethod trackingMethod);

	private:
		unsigned char* reflectable;
		TypeInfo* reflectableType;
		ReflectableHandle* nextHandle;
		ReflectableHandle* previousHandle;

		//This is a pointer because otherwise it would be subject to static member destruction order, causing
		//static reflectable objects that are destructed to sometimes crash because customObjectObservers was already destroyed.
		//Because of this, customObjectObservers is also intentionally leaked.
		static std::unordered_map<const unsigned char*, ReflectableHandle*>* customObjectObservers;
	};

	template<>
	class ExternalReflector<ReflectableHandle>
	{
	public:
		static const char* getTypeName() {return "ReflectableHandle";}
		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo) {}	
		static constexpr bool exists = true;
	};

} //End namespace jitcat::Reflection