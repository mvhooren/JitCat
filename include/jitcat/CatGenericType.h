/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once
#include "jitcat/CatInfixOperatorType.h"
#include "jitcat/ContainerType.h"
#include "jitcat/InfixOperatorResultInfo.h"
#include "jitcat/TypeInfoDeleter.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <any>
#include <fstream>
#include <map>
#include <memory>
#include <string>

namespace jitcat
{
	namespace Reflection
	{
		class TypeInfo;
		class TypeCaster;
		class ContainerManipulator;
	}

	class CatGenericType
	{
	private:
		enum class SpecificType
		{
			None,
			Basic,
			Pointer,
			ReflectableHandle,
			ReflectableObject,
			Container,
			Count
		};
	
		enum class BasicType
		{
			None,
			Int,
			Float,
			String,
			Bool,
			Void,
			Count
		};

		CatGenericType(SpecificType specificType, BasicType basicType, Reflection::TypeInfo* nestedType, Reflection::TypeOwnershipSemantics ownershipSemantics, Reflection::ContainerType containerType, Reflection::ContainerManipulator* containerManipulator, CatGenericType* pointeeType, bool writable, bool constant);
		CatGenericType(BasicType catType, bool writable = false, bool constant = false);

	public:
		CatGenericType();
		CatGenericType(Reflection::TypeInfo* reflectableType, bool writable = false, bool constant = false);
		CatGenericType(Reflection::ContainerType containerType, Reflection::ContainerManipulator* containerManipulator, bool writable = false, bool constant = false);
		CatGenericType(const CatGenericType& pointee, Reflection::TypeOwnershipSemantics ownershipSemantics, bool isHandle, bool writable = false, bool constant = false);
		CatGenericType(const CatGenericType& other);

		CatGenericType& operator=(const CatGenericType& other);
		bool operator== (const CatGenericType& other) const;
		bool operator!= (const CatGenericType& other) const;
		bool compare(const CatGenericType& other, bool includeOwnershipSemantics, bool includeIndirection) const;

		bool isUnknown() const;
		bool isValidType() const;
		bool isBasicType() const;
		bool isBoolType() const;
		bool isIntType() const;
		bool isFloatType() const;
		bool isStringType() const;
		bool isScalarType() const;
		bool isVoidType() const;
		bool isReflectableObjectType() const;
		bool isReflectableHandleType() const;
		bool isPointerToReflectableObjectType() const;
		bool isReflectablePointerOrHandle() const;
		bool isPointerType() const;
		bool isPointerToPointerType() const;
		bool isPointerToHandleType() const;
		bool isAssignableType() const;
		bool isContainerType() const;
		bool isArrayType() const;
		bool isPointerToArrayType() const;
		bool isVectorType() const;
		bool isMapType() const;
		bool isNullptrType() const;
		
		bool isTriviallyCopyable() const;
		bool isWritable() const;
		bool isConst() const;

		void addDependentType(Reflection::TypeInfo* objectType);
		bool isDependentOn(Reflection::TypeInfo* objectType) const;

		//Copies the type but sets all modifiers (const, writable) to false.
		CatGenericType toUnmodified() const;
		//Copies the type but sets the writable flag to false.
		CatGenericType toUnwritable() const;
		//Copies the type but sets the writable flag to true.
		CatGenericType toWritable() const;
		//Copies the type but sets the ownership to Value
		CatGenericType toValueOwnership() const;
		//Gets a pointer type to this type
		CatGenericType toPointer(Reflection::TypeOwnershipSemantics ownershipSemantics = Reflection::TypeOwnershipSemantics::Weak, bool writable = false, bool constant = false) const;
		CatGenericType toHandle(Reflection::TypeOwnershipSemantics ownershipSemantics = Reflection::TypeOwnershipSemantics::Weak, bool writable = false, bool constant = false) const;
		CatGenericType convertPointerToHandle() const;
		//Removes pointers and handles
		const CatGenericType& removeIndirection() const;
		const CatGenericType& removeIndirection(int& levelsOfIndirectionRemoved) const;

		Reflection::ContainerManipulator* getContainerManipulator() const;
		const CatGenericType& getContainerItemType() const;
		const char* getObjectTypeName() const;

		AST::InfixOperatorResultInfo getInfixOperatorResultInfo(AST::CatInfixOperatorType oper, const CatGenericType& rightType);

		std::string toString() const;
		
		CatGenericType* getPointeeType() const;
		void setPointeeType(std::unique_ptr<CatGenericType> pointee);

		Reflection::TypeInfo* getObjectType() const;
		Reflection::TypeOwnershipSemantics getOwnershipSemantics() const;
		void setOwnershipSemantics(Reflection::TypeOwnershipSemantics semantics);

		//This will cast the pointer to the C++ type associated with this CatGenericType and returns it as a std::any
		std::any createAnyOfType(uintptr_t pointer);
		//This will cast and dereference the pointer to the C++ type associate with this CatGenericType and returns it as a std::any.
		std::any createAnyOfTypeAt(uintptr_t pointer);

		std::any createDefault() const;

		std::size_t getTypeSize() const;

		//Converts value of valueType to this type if possible, otherwise returns default value
		std::any convertToType(std::any value, const CatGenericType& valueType) const;

		void printValue(std::any& value);

		static float convertToFloat(std::any value, const CatGenericType& valueType);
		static int convertToInt(std::any value, const CatGenericType& valueType);
		static bool convertToBoolean(std::any value, const CatGenericType& valueType);
		static std::string convertToString(std::any value, const CatGenericType& valueType);
		static CatGenericType readFromXML(std::ifstream& xmlFile, const std::string& closingTag, std::map<std::string, Reflection::TypeInfo*>& typeInfos);
		void writeToXML(std::ofstream& xmlFile, const char* linePrefixCharacters);

		bool isConstructible() const;
		bool isCopyConstructible() const;
		bool isMoveConstructible() const;
		bool isDestructible() const;

		//Construct and default-initialize this type.
		std::any construct();
		//Construct and default-initialize this type into the supplied buffer. 
		//Buffer size must be greater or equal to getTypeSize(). Returns true if succesful.
		bool placementConstruct(unsigned char* buffer, std::size_t bufferSize);
		//Copy construct this type from the sourceBuffer to the target buffer. Returns true if succesful.
		bool copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize);
		//Move construct this type form the sourceBuffer to the target buffer. Returns true if succesful.
		bool moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize);
		//Destruct this type located in the supplied buffer. Returns true if succesful.
		bool placementDestruct(unsigned char* buffer, std::size_t bufferSize);
		//Casts the contents of value to a unsigned char* and gets the size of the value
		void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const;

		//If this is a non-pointer type, gets a typecaster for the type
		const Reflection::TypeCaster* getTypeCaster() const;
		//If this is a pointer type, gets the raw pointer value/address
		uintptr_t getRawPointer(const std::any& value) const;
		//Returns a std::any containing a pointer to the object stored in value.
		std::any getAddressOf(std::any& value) const;
		//If this is a pointer type, creates a pointer value of this type from the raw pointer.
		std::any createFromRawPointer(const uintptr_t pointer) const;
		std::any createNullPtr() const;

		static CatGenericType createIntType(bool isWritable, bool isConst);
		static CatGenericType createFloatType(bool isWritable, bool isConst);
		static CatGenericType createBoolType(bool isWritable, bool isConst);
		static CatGenericType createStringType(bool isWritable, bool isConst);

	private:
		static const char* toString(BasicType type);
		static BasicType toBasicType(const char* value);
		static const char* toString(SpecificType type);
		static SpecificType toSpecificType(const char* value);

	public:
		static const CatGenericType intType;
		static const CatGenericType floatType;
		static const CatGenericType boolType;
		static const CatGenericType stringType;
		static const CatGenericType voidType;
		static const std::unique_ptr<Reflection::TypeInfo, Reflection::TypeInfoDeleter> nullptrTypeInfo;
		static const CatGenericType nullptrType;
		static const CatGenericType unknownType;

		static const std::unique_ptr<Reflection::TypeCaster> intTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> floatTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> boolTypeCaster; 
		static const std::unique_ptr<Reflection::TypeCaster> stringTypeCaster;

	private:
		SpecificType specificType;
		BasicType basicType;

		//not owned
		Reflection::TypeInfo* nestedType;

		//When the member is a container, catType or nestedType will be set to the item type of the container
		Reflection::ContainerType containerType;
		//not owned, non null when the type is a container.
		Reflection::ContainerManipulator* containerManipulator;

		Reflection::TypeOwnershipSemantics ownershipSemantics;
		std::unique_ptr<CatGenericType> pointeeType;

		//Type modifiers/flags. These are not taken into account when comparing CatGenericType objects using operator ==.
		bool writable;
		bool constant;
	};


} // End namespace jitcat