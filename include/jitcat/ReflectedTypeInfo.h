/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include "jitcat/TypeInfo.h"

#include <cstddef>
#include <functional>


namespace jitcat::Reflection
{
	class Reflectable;
	class TypeCaster;

	//ReflectedTypeInfo represents type information about a C++ class or struct that is made to be Reflectable.

	//Any class or struct can be made reflectable as follows:
	//		Inherit from Reflectable so that a ReflectableHandle to the object can track if the object has been deleted.
	//		implement: static void reflect(ReflectedTypeInfo& typeInfo); //Add class members inside this function using typeInfo.addMember
	//		implement: static const char* getTypeName(); //Return the name of the class

	//Creating an instance of ReflectedTypeInfo and then passing it into the static reflect member of a class or struct will fill the datastructure with type information.
	//This is typically done through the TypeRegistry class (see TypeRegistry.h)

	//Notice that the addMember function does not require an instance of the class to exist. Generating type information is done entirely statically.

	class ReflectedTypeInfo: public TypeInfo
	{
	public:
		ReflectedTypeInfo(const char* typeName, std::size_t typeSize, std::unique_ptr<TypeCaster> typeCaster, bool allowConstruction,
						  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
						  std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
						  std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
						  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor);
	protected:
		virtual ~ReflectedTypeInfo();
	public:

		//Adds information of a member of type U inside struct/class T
		//A second function exists to differentiate between U and U*
		template <typename ReflectedT, typename MemberT>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberT ReflectedT::* member, MemberFlags flags = MF::none);

		template <typename MemberCVT>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberCVT* member, MemberFlags flags = MF::none);

		template <typename ReflectedT, typename ReturnT, typename ... Args>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, ReturnT (ReflectedT::*function)(Args...));

		template <typename ReflectedT, typename ReturnT, typename ... Args>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, ReturnT (ReflectedT::*function)(Args...) const);

		template <typename ReturnT, typename ... Args>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, ReturnT (*function)(Args...));

		template <typename ReflectedT, typename ReturnT, typename ... Args>
		inline ReflectedTypeInfo& addPseudoMemberFunction(const std::string& identifier, ReturnT (*function)(ReflectedT*, Args...));

		template <typename ConstantT>
		inline ReflectedTypeInfo& addConstant(const std::string& identifier, ConstantT value);

		//Set weither or not construction is allowed.
		ReflectedTypeInfo& enableConstruction();
		ReflectedTypeInfo& disableConstruction();

		//Set weither or not copy construction is allowed.
		ReflectedTypeInfo& enableCopyConstruction();
		ReflectedTypeInfo& disableCopyConstruction();

		//Set weither or not move construction is allowed.
		ReflectedTypeInfo& enableMoveConstruction();
		ReflectedTypeInfo& disableMoveConstruction();

		//Set weither or not the type is trivially copyable.
		void setTriviallyCopyable(bool triviallyCopyable_);

		//Set weither or not custom types can inherit from this type.
		ReflectedTypeInfo& enableInheritance();
		ReflectedTypeInfo& disableInheritance();

		//Sets a callback that can check a derived type for compatibility.
		ReflectedTypeInfo& setInheritanceChecker(const std::function<bool (CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*)>& checkFunction);

		//Sets callbacks for construction and destruction.
		ReflectedTypeInfo& setConstructors(std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
										   std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
										   std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
										   std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor);
		//Sets the size of the type.
		ReflectedTypeInfo& setTypeSize(std::size_t newSize);

		virtual bool isReflectedType() const override final;

		//Construct or destruct instances of the type (if allowed)
		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const override final;
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize) override final;
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;

		virtual bool getAllowInheritance() const override final;
		virtual bool inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual bool getAllowConstruction() const override final;
		virtual bool getAllowCopyConstruction() const override final;
		virtual bool getAllowMoveConstruction() const override final;

		virtual bool isTriviallyCopyable() const override final;


	private:
		std::function<void(unsigned char* buffer, std::size_t bufferSize)> placementConstructor;
		std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)> copyConstructor;
		std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)> moveConstructor;			;
		std::function<void(unsigned char* buffer, std::size_t bufferSize)> placementDestructor;
		std::function<bool (CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*)> inheritanceCheckFunction;

		bool allowConstruction;
		bool allowCopyConstruction;
		bool allowMoveConstruction;
		bool allowInheritance;
		bool triviallyCopyable;
	};

}

#include "jitcat/ReflectedTypeInfoHeaderImplementation.h"