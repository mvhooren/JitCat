/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatInfixOperatorType.h"
#include "jitcat/Configuration.h"
#include "jitcat/IndirectionConversionMode.h"
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
	}

	class CatGenericType
	{
	private:
		enum class SpecificType: unsigned char
		{
			None,
			Basic,
			Enum,
			Pointer,
			ReflectableHandle,
			ReflectableObject,
			Count
		};
	
		enum class BasicType: unsigned char
		{
			None,
			Char,
			UChar,
			Int,
			UInt,
			Int64,
			UInt64,
			Float,
			Double,
			Bool,
			Void,
			Count
		};

		CatGenericType(SpecificType specificType, BasicType basicType, Reflection::TypeInfo* nestedType, 
					   Reflection::TypeOwnershipSemantics ownershipSemantics, CatGenericType* pointeeType, bool writable, bool constant);
		CatGenericType(BasicType catType, bool writable = false, bool constant = false);

	public:
		CatGenericType();
		CatGenericType(const CatGenericType& enumUnderlyingType, Reflection::TypeInfo* enumValuesType, bool writable = false, bool constant = false);
		CatGenericType(Reflection::TypeInfo* reflectableType, bool writable = false, bool constant = false);
		CatGenericType(const CatGenericType& pointee, Reflection::TypeOwnershipSemantics ownershipSemantics, 
					   bool isHandle, bool writable = false, bool constant = false);
		CatGenericType(const CatGenericType& other);

		CatGenericType& operator=(const CatGenericType& other);

		//Comparison operators do not compare the writable and constant flags
		bool operator== (const CatGenericType& other) const;
		bool operator!= (const CatGenericType& other) const;
		bool compare(const CatGenericType& other, bool includeOwnershipSemantics, bool includeIndirection) const;

		bool isUnknown() const;
		bool isValidType() const;
		bool isBasicType() const;
		bool isBoolType() const;
		bool isCharType() const;
		bool isUCharType() const;
		bool isIntType() const;
		bool isUIntType() const;
		bool isInt64Type() const;
		bool isUInt64Type() const;
		bool isIntegralType() const;
		bool isFloatType() const;
		bool isDoubleType() const;

		bool isStringType() const;
		bool isStringPtrType() const;
		bool isStringValueType() const;

		bool isScalarType() const;
		bool isSignedType() const;
		bool isUnsignedType() const;
		bool isVoidType() const;
		bool isEnumType() const;
		bool isReflectableObjectType() const;
		bool isReflectableHandleType() const;
		bool isPointerToReflectableObjectType() const;
		bool isReflectablePointerOrHandle() const;
		bool isPointerType() const;
		bool isPointerToPointerType() const;
		bool isPointerToHandleType() const;
		bool isAssignableType() const;
		bool isNullptrType() const;
		
		bool isTriviallyCopyable() const;
		bool isWritable() const;
		bool isConst() const;

		void addDependentType(Reflection::TypeInfo* objectType);
		bool isDependentOn(Reflection::TypeInfo* objectType) const;

		//If this is an enum type. Gets the underlying type of the enum (for example, int).
		const CatGenericType& getUnderlyingEnumType() const;

		CatGenericType copyWithFlags(bool writable, bool constant) const;
		//Copies the type but sets all modifiers (const, writable) to false.
		CatGenericType toUnmodified() const;
		//Copies the type but sets the writable flag to false.
		CatGenericType toUnwritable() const;
		//Copies the type but sets the writable flag to true.
		CatGenericType toWritable() const;
		//Copies the type but sets the ownership to Value
		CatGenericType toValueOwnership() const;
		//Copies the type but sets the ownership to Value
		CatGenericType toChangedOwnership(Reflection::TypeOwnershipSemantics ownershipSemantics) const;
		//Gets a pointer type to this type
		CatGenericType toPointer(Reflection::TypeOwnershipSemantics ownershipSemantics = Reflection::TypeOwnershipSemantics::Weak, 
								 bool writable = false, bool constant = false) const;
		CatGenericType toHandle(Reflection::TypeOwnershipSemantics ownershipSemantics = Reflection::TypeOwnershipSemantics::Weak, 
								bool writable = false, bool constant = false) const;
		CatGenericType convertPointerToHandle() const;
		CatGenericType toArray() const;
		//Removes pointers and handles
		const CatGenericType& removeIndirection() const;
		const CatGenericType& removeIndirection(int& levelsOfIndirectionRemoved) const;
		IndirectionConversionMode getIndirectionConversion(const CatGenericType& other) const;
		std::any doIndirectionConversion(std::any& value, IndirectionConversionMode mode) const;

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
		//Creates a default value of this type and returns is in a std::any.
		std::any createDefault() const;
		//Returns the size of the type in bytes.
		std::size_t getTypeSize() const;

		//Converts value of valueType to this type if possible, otherwise returns default value
		std::any convertToType(std::any& value, const CatGenericType& valueType) const;

		//If this is an enum type. Converts the std::any containing an enum value to a std::any containing its underlying type.
		std::any toUnderlyingType(std::any enumValue) const;

		void printValue(std::any& value);

		static float convertToFloat(std::any value, const CatGenericType& valueType);
		static double convertToDouble(std::any value, const CatGenericType& valueType);
		static char convertToChar(std::any value, const CatGenericType& valueType);
		static unsigned char convertToUChar(std::any value, const CatGenericType& valueType);
		static int convertToInt(std::any value, const CatGenericType& valueType);
		static unsigned int convertToUInt(std::any value, const CatGenericType& valueType);
		static int64_t convertToInt64(std::any value, const CatGenericType& valueType);
		static uint64_t convertToUInt64(std::any value, const CatGenericType& valueType);
		static bool convertToBoolean(std::any value, const CatGenericType& valueType);
		static Configuration::CatString convertToString(std::any value, const CatGenericType& valueType);
		static CatGenericType readFromXML(std::ifstream& xmlFile, const std::string& closingTag, std::map<std::string, Reflection::TypeInfo*>& typeInfos);
		void writeToXML(std::ofstream& xmlFile, const char* linePrefixCharacters) const;

		bool isConstructible() const;
		bool isCopyConstructible() const;
		bool isMoveConstructible() const;
		bool isDestructible() const;

		//Construct and default-initialize this type.
		std::any construct() const;
		//Construct and default-initialize this type into the supplied buffer. 
		//Buffer size must be greater or equal to getTypeSize(). Returns true if succesful.
		bool placementConstruct(unsigned char* buffer, std::size_t bufferSize) const;
		//Copy construct this type from the sourceBuffer to the target buffer. Returns true if succesful.
		bool copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) const;
		//Move construct this type form the sourceBuffer to the target buffer. Returns true if succesful.
		bool moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) const;
		//Destruct this type located in the supplied buffer. Returns true if succesful.
		bool placementDestruct(unsigned char* buffer, std::size_t bufferSize) const;
		//Casts the contents of value to a unsigned char* and gets the size of the value
		void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const;

		//If this is a non-pointer type, gets a typecaster for the type
		const Reflection::TypeCaster* getTypeCaster() const;
		//If this is a pointer type, gets the raw pointer value/address
		uintptr_t getRawPointer(const std::any& value) const;
		//Returns a std::any containing a pointer to the object stored in value.
		std::any getAddressOf(std::any& value) const;
		//If value contains a pointer, returns a std::any containing the dereferenced value of that pointer.
		std::any getDereferencedOf(std::any& value) const;
		//If this is a pointer type, creates a pointer value of this type from the raw pointer.
		std::any createFromRawPointer(const uintptr_t pointer) const;
		std::any createNullPtr() const;

		jitcat::Reflection::TypeInfo** getTypeInfoToSet();

		static const CatGenericType& getWidestBasicType(const CatGenericType& left, const CatGenericType& right);
		static CatGenericType createCharType(bool isWritable, bool isConst);
		static CatGenericType createUCharType(bool isWritable, bool isConst);
		static CatGenericType createIntType(bool isWritable, bool isConst);
		static CatGenericType createUIntType(bool isWritable, bool isConst);
		static CatGenericType createInt64Type(bool isWritable, bool isConst);
		static CatGenericType createUInt64Type(bool isWritable, bool isConst);
		static CatGenericType createFloatType(bool isWritable, bool isConst);
		static CatGenericType createDoubleType(bool isWritable, bool isConst);
		static CatGenericType createBoolType(bool isWritable, bool isConst);
		static CatGenericType createStringType(bool isWritable, bool isConst);
		static CatGenericType createArrayType(const CatGenericType& arrayItemType, bool isWritable, bool isConst);
		static CatGenericType createBasicTypeFromName(const std::string& name, bool isWritable, bool isConst);

	private:
		static bool isValidSpecificType(SpecificType type);
		static bool isValidBasicType(BasicType type);
		static const char* toString(BasicType type);
		static BasicType toBasicType(const char* value);
		static BasicType getWidestType(BasicType lType, BasicType rType);
		static const char* toString(SpecificType type);
		static SpecificType toSpecificType(const char* value);
		static const CatGenericType& getBasicType(BasicType type);

	public:
		static const CatGenericType voidType;
		static const CatGenericType charType;
		static const CatGenericType uCharType;
		static const CatGenericType intType;
		static const CatGenericType uIntType;
		static const CatGenericType int64Type;
		static const CatGenericType uInt64Type;
		static const CatGenericType floatType;
		static const CatGenericType doubleType;
		static const CatGenericType boolType;
		static const CatGenericType stringType;
	
		static const CatGenericType stringWeakPtrType;
		static const CatGenericType stringConstantValuePtrType;
		static const CatGenericType stringMemberValuePtrType;
		static const std::unique_ptr<Reflection::TypeInfo, Reflection::TypeInfoDeleter> nullptrTypeInfo;
		static const CatGenericType nullptrType;
		static const CatGenericType unknownType;

		static const std::unique_ptr<Reflection::TypeCaster> charTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> uCharTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> intTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> uIntTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> int64TypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> uInt64TypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> floatTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> doubleTypeCaster;
		static const std::unique_ptr<Reflection::TypeCaster> boolTypeCaster; 
		static const std::unique_ptr<Reflection::TypeCaster> stringTypeCaster;

	private:
		SpecificType specificType;
		BasicType basicType;

		//Type modifiers/flags. These are not taken into account when comparing CatGenericType objects using operator ==.
		bool writable;
		bool constant;

		//not owned
		Reflection::TypeInfo* nestedType;

		std::unique_ptr<CatGenericType> pointeeType;

		Reflection::TypeOwnershipSemantics ownershipSemantics;
	};


} // End namespace jitcat