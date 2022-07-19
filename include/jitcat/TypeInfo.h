/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/ExternalReflector.h"
#include "jitcat/MemberFlags.h"
#include "jitcat/Reflectable.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/Tools.h"

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>


namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionErrorManager;
	namespace AST
	{
		class CatClassDefinition;
	}
}

namespace jitcat::Reflection
{
	class FunctionSignature;
	struct MemberFunctionInfo;
	class ReflectedTypeInfo;
	class StaticFunctionInfo;
	class StaticConstMemberInfo;
	struct StaticMemberInfo;
	class TypeCaster;
	struct TypeMemberInfo;
	class VariableEnumerator;


	//TypeInfo stores a tree of type member information about a class or struct.
		
	//There are two main sources of type information, both are represented by a class that derives from TypeInfo.
	//The first source is through reflection of c++ classes and structs. This is done through the ReflectedTypeInfo class. 
	//(See ReflectedTypeInfo.h)
	//The second source is through custom type creation. In this case a type is defined at runtime through the CustomTypeInfo class.
	//(See CustomTypeInfo.h)
	
	//The TypeInfo system can store information about members of the following types:
	//- Common basic types: void, int, bool, float, double
	//- std::string
	//- pointers and std::unique_ptr that point to a supported type.
	//- Common stl containers: std::vector<T>, std::deque<T>, std::array<T, int>, std::unordered_map<U, T> and std::map<U, T> 
	//  where T and U are themselves supported types.
	//- Structs and classes that have been made reflectable.
	//- Member functions, including static member functions, that use supported types as parameters and return value (including void).
	//- Overloaded operators (both the static and the member variants.)
	//- Named static constants of a supported data type.

	//Member variables are described by classes that inherit from TypeMemberInfo. (See MemberInfo.h)
	//Member functions are described by classes that inherit from MemberFunctionInfo. (See MemberFunctionInfo.h)
	//Static member functions are described by classes that inherit from StaticMemberfunctionInfo. (See StaticMemberFunctionInfo.h)
	//Finally, static members are described by classes that inherit from StaticMemberInfo. (See StaticMemberInfo.h)

	//TypeInfo can also construct and delete instances of the type it represents, if construction/deletion is allowed/possible. 
	class TypeInfo
	{
	public:
		//The type name must be a static const char* because types are compared based on the pointer value of their type names.
		TypeInfo(const char* typeName, std::size_t typeSize, std::unique_ptr<TypeCaster> caster);
	protected:
		virtual ~TypeInfo();
		//Destroy should be called instead of deleting a type. 
		//It will ensure that the type is only deleted when there are no more dependencies.
		static void destroy(TypeInfo* type);
		friend struct TypeInfoDeleter;

	public:

		static void updateTypeDestruction();
		//Add a nested type to this type. Return true if the type was added, false if a type with this name already exists.
		bool addType(TypeInfo* type);
		//Add a named constant value to this type. Returns nullptr if a constant if this name already exists.
		StaticConstMemberInfo* addConstant(const std::string& name, const CatGenericType& type, const std::any& value);
		//Set the parent of this type if this type is nested in some other type.
		void setParentType(TypeInfo* type);
		//Returns true if the type was deleted, false if the type was not found.
		bool removeType(const std::string& typeName);

		void addDeserializedMember(TypeMemberInfo* memberInfo);
		void addDeserializedStaticMember(StaticMemberInfo* staticMemberInfo);
		void addDeserializedMemberFunction(MemberFunctionInfo* memberFunction);
		void addDeserializedStaticMemberFunction(StaticFunctionInfo* staticFunction);

		//Returns the size of the type in bytes
		std::size_t getTypeSize() const;

		//Given a dot notated string like "bla.blep.blip", returns the CatGenericType of "blip".
		const CatGenericType& getType(const std::string& dotNotation) const;
		//Similar to above, but instead it takes a vector that contains the strings splitted on "." and an offset from where to start.
		//The offset should point to a variable of the type described by this TypeInfo.
		const CatGenericType& getType(const std::vector<std::string>& indirectionList, int offset) const;
	
		//Gets the type information of a member variable given its name.
		TypeMemberInfo* getMemberInfo(const std::string& identifier) const;

		//Gets the type information of a static member variable given its name.
		StaticMemberInfo* getStaticMemberInfo(const std::string& identifier) const;

		//Gets the type and value information of a static const member given its name.
		StaticConstMemberInfo* getStaticConstMemberInfo(const std::string& identifier) const;

		//Gets the type information of a member function given its name.
		MemberFunctionInfo* getFirstMemberFunctionInfo(const std::string& identifier) const;
		std::vector<MemberFunctionInfo*> getMemberFunctionsByName(const std::string& functionName) const;

		//Gets the type information of a member function given its name.
		MemberFunctionInfo* getMemberFunctionInfo(const FunctionSignature* functionSignature) const;
		MemberFunctionInfo* getMemberFunctionInfo(const FunctionSignature& functionSignature) const;

		//Gets the type information of a static function given its name.
		StaticFunctionInfo* getFirstStaticMemberFunctionInfo(const std::string& identifier) const;

		//Gets the type information of a static function given its name.
		StaticFunctionInfo* getStaticMemberFunctionInfo(const FunctionSignature* functionSignature) const;
		StaticFunctionInfo* getStaticMemberFunctionInfo(const FunctionSignature& functionSignature) const;

		//Gets a nested type given its name
		TypeInfo* getTypeInfo(const std::string& typeName) const;

		//Returns the type name of the class/struct
		const char* getTypeName() const;
		//Includes parent type names with :: delimiter.
		std::string getQualifiedTypeName() const;

		void setTypeName(const char* newTypeName);

		//Enumerates all the members of the class described by this TypeInfo by passing them to the VariableEnumerator in alphabetical order
		void enumerateVariables(VariableEnumerator* enumerator, bool allowEmptyStructs) const;
		//Enumerats all the member variables of the class described by this TypeInfo by passing them to the enumerator function in ordinal order
		void enumerateMemberVariables(std::function<void(const CatGenericType&, const std::string&)>& enumerator) const;

		//Returns true if this is a CustomTypeInfo.
		virtual bool isCustomType() const;
		//Returns true if this is a ReflectedTypeInfo.
		virtual bool isReflectedType() const;
		//Returns true if this is an ArrayTypeInfo.
		virtual bool isArrayType() const;
		//Returns true if this is a VectorTypeInfo
		virtual bool isVectorType() const;

		//Returns true if the type can be copied using memcpy without adverse side effects.
		virtual bool isTriviallyCopyable() const;
		//Returns true if the type can be constructed through zero-initialisation.
		virtual bool isTriviallyConstructable() const;

		//Beware that these lists are case insensitive because the keys have been converted to lower case
		const std::map<std::string, std::unique_ptr<TypeMemberInfo>>& getMembers() const;
		const std::map<std::string, std::unique_ptr<StaticMemberInfo>>& getStaticMembers() const;
		const std::map<unsigned long long, TypeMemberInfo*>& getMembersByOrdinal() const;
		const std::multimap<std::string, std::unique_ptr<MemberFunctionInfo>>& getMemberFunctions() const;
		const std::multimap<std::string, std::unique_ptr<StaticFunctionInfo>>& getStaticMemberFunctions() const;
		const std::map<std::string, TypeInfo*>& getTypes() const;

		//May be nullptr when type info was read from XML
		const TypeCaster* getTypeCaster() const;

		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const;

		//Allocates memory and default-constructs an instance of this type and returns a pointer to the object.
		unsigned char* construct() const;
		//Runs the destructor of this type on the object at objectPointer and frees its memory.
		void destruct(unsigned char* objectPointer);
		//Runs the destructor of this type on the buffer, does not free any memory.
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize);
		//If supported, runs the copy constructor of this type on the object contained in sourceBuffer and places the copy in targetBuffer.
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize);
		//If supported, runs the move constructor of this type on the object contained in sourceBuffer and places the copy in targetBuffer.
		//The object in sourceBuffer should be destructed afterwards.
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize);
		//Given a pointer contained in value, cast it to a raw buffer.
		void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const;

		virtual bool getAllowInheritance() const;
		virtual bool inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext);
		virtual bool getAllowConstruction() const;
		virtual bool getAllowCopyConstruction() const;
		virtual bool getAllowMoveConstruction() const;

		//Returns true if the type has no dependencies and can be deleted.
		virtual bool canBeDeleted() const;

		bool canBeAssignedBy(const CatGenericType& type) const;

		void addDependentType(TypeInfo* otherType);
		void removeDependentType(TypeInfo* otherType);

		void renameMember(const std::string& oldMemberName, const std::string& newMemberName);

	protected:
		//Adds members from a member object that will automatically be forwarded.
		void addDeferredMembers(TypeMemberInfo* deferredMember);
		void addMember(const std::string& memberName, TypeMemberInfo* memberInfo);
		TypeMemberInfo* releaseMember(const std::string& memberName);

	protected:
		const char* typeName;
		std::unique_ptr<TypeCaster> caster;
	protected:
		
		//Member variables of this type
		std::map<std::string, std::unique_ptr<TypeMemberInfo>> members;
		//Member variables sorted by their offset / ordinal
		std::map<unsigned long long, TypeMemberInfo*> membersByOrdinal;

		//Member functions of this type, member function overloading is allowed
		std::multimap<std::string, std::unique_ptr<MemberFunctionInfo>> memberFunctions;
		//Static member variables of this type
		std::map<std::string, std::unique_ptr<StaticMemberInfo>> staticMembers;
		//Static const members of this type
		std::map<std::string, std::unique_ptr<StaticConstMemberInfo>> staticConstMembers;
		//Static functions of this type, static function overloading is allowed
		std::multimap<std::string, std::unique_ptr<StaticFunctionInfo>> staticFunctions;

		//Nested type definitions within this type. These are not owned here.
		std::map<std::string, TypeInfo*> types;

		//The parent of this type if this type is nested into another type. nullptr otherwise.
		TypeInfo* parentType;
		//Size of the type in bytes
		std::size_t typeSize;

		//A set of types that use this type as an object data member or inherit from this type
		std::set<TypeInfo*> dependentTypes;

		//Keep a list of types that are to be deleted.
		//Types are only deleted if there are no more dependencies on that type.
		static std::vector<TypeInfo*> typeDeletionList;
	};

	template<>
	class ExternalReflector<TypeInfo>
	{
	public:
		static const char* getTypeName() {return "TypeInfo";}
		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo) {}
		static constexpr bool exists = true;
	};
}
