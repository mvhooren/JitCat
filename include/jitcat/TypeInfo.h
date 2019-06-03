/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/ContainerType.h"
#include "jitcat/MemberFlags.h"
#include "jitcat/Reflectable.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/Tools.h"

#include <any>
#include <map>
#include <memory>
#include <vector>

namespace jitcat::Reflection
{
	struct MemberFunctionInfo;
	class TypeCaster;
	class TypeInfo;
	struct TypeMemberInfo;
	class VariableEnumerator;


	//TypeInfo stores a tree of type member information about a class or struct.
	//This allows members to be accessed by string. (For example Root.Member[10].Member().Member)
	//Each member is described by a class that inherits from TypeMemberInfo. (See TypeMemberInfo.h)
	//Going from a string to a reference is done by calling dereference on a TypeInfo. This will return a std::any object.
	//See TypeDereferencer.h

	//The TypeInfo system can store information about the following types:
	//- Common basic types: int, bool, float and std::string
	//- Common containers: std::vector<T>, std::map<std::string, T> where T is itself a reflectable class/struct
	//- Structs and classes that have been made reflectable

	//Any class or struct can be made reflectable as follows:
	//		Inherit from Reflectable so that references to the object stored when dereferencing can check if the object has been deleted.
	//		static void reflect(TypeInfo& typeInfo); //Add class members inside this function using typeInfo.addMember
	//		static const char* getTypeName(); //Return the name of the class

	//These types should be enough to implement most common data structures. Adding more types is possible, but it would lead to a lot more template-nastyness in TypeInfo.h
	//Also, this system is designed to work with JitCat, which currently only supportes the basic types mentioned above.

	//Creating an instance of TypeInfo and then passing it into the static reflect member of a class or struct will fill the datastructure with type information.
	//Notice that the addMember function does not require an instance of the class to exist. Generating type information is done entirely statically.
	//This class uses various template tricks to derive types from the parameter passed to addMember
	class TypeInfo
	{
	public:
		//The type name must be a static const char* because types are compared based on the pointer value of their type names.
		TypeInfo(const char* typeName, TypeCaster* caster);
		virtual ~TypeInfo();

		//Adds information of a member of type U inside struct/class T
		//A second function exists to differentiate between U and U*
		template <typename T, typename U>
		TypeInfo& addMember(const std::string& identifier, U T::* member, MemberFlags flags = MF::none);

		template <typename T, typename U, typename ... Args>
		TypeInfo& addMember(const std::string& identifier, U (T::*function)(Args...));

		template <typename T, typename U, typename ... Args>
		TypeInfo& addMember(const std::string& identifier, U (T::*function)(Args...) const);

		//Add a nested type to this type. Return true if the type was added, false if a type with this name already exists.
		bool addType(TypeInfo* type);
		//Set the parent of this type if this type is nested in some other type.
		void setParentType(TypeInfo* type);
		//Returns true if the type was deleted, false if the type was not found.
		bool removeType(const std::string& typeName);

		void addDeserializedMember(TypeMemberInfo* memberInfo);
		void addDeserializedMemberFunction(MemberFunctionInfo* memberFunction);

		//Given a dot notated string like "bla.blep.blip", returns the CatGenericType of "blip".
		const CatGenericType& getType(const std::string& dotNotation) const;
		//Similar to above, but instead it takes a vector that contains the strings splitted on "." and an offset from where to start.
		//The offset should point to a variable of the type described by this TypeInfo.
		const CatGenericType& getType(const std::vector<std::string>& indirectionList, int offset) const;
	
		//Gets the type information of a member variable given its name.
		TypeMemberInfo* getMemberInfo(const std::string& identifier) const;

		//Gets the type information of a member function given its name.
		MemberFunctionInfo* getMemberFunctionInfo(const std::string& identifier) const;

		TypeInfo* getTypeInfo(const std::string& typeName) const;

		//Returns the type name of the class/struct
		const char* getTypeName() const;
		void setTypeName(const char* newTypeName);

		//Enumerates all the members of the class described by this TypeInfo by passing them to the VariableEnumerator
		void enumerateVariables(VariableEnumerator* enumerator, bool allowEmptyStructs) const;

		//Returns true if this is a CustomTypeInfo.
		virtual bool isCustomType() const;

		//Beware that these lists are case insensitive because the keys have been converted to lower case
		const std::map<std::string, std::unique_ptr<TypeMemberInfo>>& getMembers() const;
		const std::map<std::string, std::unique_ptr<MemberFunctionInfo>>& getMemberFunctions() const;
		const std::map<std::string, TypeInfo*>& getTypes() const;

		//May be nullptr when type info was read from XML
		const TypeCaster* getTypeCaster() const;

	protected:
		//Adds members from a member object that will automatically be forwarded.
		void addDeferredMembers(TypeMemberInfo* deferredMember);

	protected:
		const char* typeName;
		std::unique_ptr<TypeCaster> caster;
		//Member variables of this type
		std::map<std::string, std::unique_ptr<TypeMemberInfo>> members;
		//Member functions of this type
		std::map<std::string, std::unique_ptr<MemberFunctionInfo>> memberFunctions;
		//Nested type definitions within this type. These are not owned here.
		std::map<std::string, TypeInfo*> types;
		//The parent of this type if this type is nested into another type. nullptr otherwise.
		TypeInfo* parentType;
	};

}
#include "jitcat/TypeInfoHeaderImplementation.h"
