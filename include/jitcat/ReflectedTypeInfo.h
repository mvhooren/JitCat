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

	class ReflectedTypeInfo: public TypeInfo
	{
	public:
		ReflectedTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* typeCaster, bool allowConstruction,
						  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
						  std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
						  std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
						  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor);
	protected:
		virtual ~ReflectedTypeInfo() {};
	public:

		//Adds information of a member of type U inside struct/class T
		//A second function exists to differentiate between U and U*
		template <typename ReflectedT, typename MemberT>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberT ReflectedT::* member, MemberFlags flags = MF::none);

		template <typename ReflectedT, typename MemberT, typename ... Args>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberT (ReflectedT::*function)(Args...));

		template <typename ReflectedT, typename MemberT, typename ... Args>
		inline ReflectedTypeInfo& addMember(const std::string& identifier, MemberT (ReflectedT::*function)(Args...) const);

		ReflectedTypeInfo& enableConstruction();
		ReflectedTypeInfo& disableConstruction();
		ReflectedTypeInfo& enableCopyConstruction();
		ReflectedTypeInfo& disableCopyConstruction();
		ReflectedTypeInfo& enableMoveConstruction();
		ReflectedTypeInfo& disableMoveConstruction();
		void setTriviallyCopyable(bool triviallyCopyable_);

		ReflectedTypeInfo& enableInheritance();
		ReflectedTypeInfo& disableInheritance();
		ReflectedTypeInfo& setInheritanceChecker(std::function<bool (CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*)>& checkFunction);
		ReflectedTypeInfo& setConstructors(std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
										   std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
										   std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
										   std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor);
		ReflectedTypeInfo& setTypeSize(std::size_t newSize);

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