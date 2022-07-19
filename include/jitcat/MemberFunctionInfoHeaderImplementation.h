/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once

#include "jitcat/Configuration.h"
#include "jitcat/FunctionNameMangler.h"
#include "jitcat/JitCat.h"
#include "jitcat/TypeConversionCastHelper.h"
#include "jitcat/TypeTraits.h"
#include "jitcat/MemberFunctionInfo.h"


namespace jitcat::Reflection
{
	template<typename ArgumentT>
	inline void MemberFunctionInfo::addParameterTypeInfo()
	{
		argumentTypes.push_back(TypeTraits<typename RemoveConst<ArgumentT>::type >::toGenericType());
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::MemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...)) :
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ((void)addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
		//Link the function to the pre-compiled expressions
		if (JitCat::get()->getHasPrecompiledExpression())
		{
			JitCat::get()->setPrecompiledLinkedFunction(getMangledName(Configuration::sretBeforeThisForCurrentProcess, FunctionType::Auto), getFunctionAddress(FunctionType::Auto).functionAddress);
			JitCat::get()->setPrecompiledLinkedFunction(getMangledName(Configuration::sretBeforeThisForCurrentProcess, FunctionType::Static), getFunctionAddress(FunctionType::Static).functionAddress);
			JitCat::get()->setPrecompiledGlobalVariable(getMangledFunctionInfoName(Configuration::sretBeforeThisForCurrentProcess, FunctionType::Member), reinterpret_cast<uintptr_t>(this));
		}
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::any MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
	{
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::size_t MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getNumberOfArguments() const
	{
		return sizeof...(TFunctionArguments);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ReturnT MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::staticExecute(ClassT* base, MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments ...args)
	{
		ReturnT(ClassT::*function)(TFunctionArguments...) = functionInfo->function;
		return (base->*function)(args...);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionCallData MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getFunctionAddress(FunctionType functionType) const
	{
		uintptr_t functionPtr = 0;
		MemberFunctionCallType callType = MemberFunctionCallType::Unknown;
		if (shouldUseStaticFunction(functionType))
		{
			memcpy(&functionPtr, &function, sizeof(uintptr_t));
			callType = MemberFunctionCallType::ThisCall;
		}
		else 
		{
			functionPtr = reinterpret_cast<uintptr_t>(&staticExecute);
			callType = MemberFunctionCallType::ThisCallThroughStaticFunction;
		}
		return MemberFunctionCallData(functionPtr, reinterpret_cast<uintptr_t>(this), nullptr, callType, false, !std::is_pointer_v<ReturnT>);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::string MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getMangledName(bool sRetBeforeThis, FunctionType functionType) const
	{
		std::string baseName = TypeTraits<ClassT>::toGenericType().getObjectType()->getQualifiedTypeName();
		if (shouldUseStaticFunction(functionType))
		{
			baseName = Tools::append(baseName, "_static");
		}
		return FunctionNameMangler::getMangledFunctionName(returnType, memberFunctionName, argumentTypes, true, baseName, sRetBeforeThis);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline bool MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::shouldUseStaticFunction(FunctionType functionType) const
	{
		//Non-member functions do not have weird function-pointers and can always be called directly
		if (functionType != FunctionType::Member)
		{
			return false;
		}
		//Polymorphic classes have virtual functions and a v-table.
		//To be on the safe size, it's best to call member functions through the static function.
		if constexpr (std::is_polymorphic_v<ClassT>)
		{
			return true;
		}
		else
		{
			//If the size of the function pointer is larger than an ordinary pointer, we should call it through the static function
			//An exception is when the function pointer is twice the size, and the extra data is zero.
			std::size_t functionPtrSize = sizeof(function);
			if (sizeof(function) != Configuration::basicMemberFunctionPointerSize
				&& !(functionPtrSize == 2 * Configuration::basicMemberFunctionPointerSize 
					 && reinterpret_cast<const uintptr_t*>(&function)[1] == 0))
			{
				return true;
			}

			return false;
		}
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	template<std::size_t ...Is>
	inline std::any MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
	{
		ClassT* baseObject = std::any_cast<ClassT*>(base);
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the parameters array
			//std::decay removes const and & from the type.
			if constexpr (std::is_void_v<ReturnT>)
			{
				(baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
			}
			else
			{
				return TypeTraits<ReturnT>::getCatValue((baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
			}

		}
		return TypeTraits<ReturnT>::toGenericType().createDefault();
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::ConstMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...) const) :
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ((void)addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
		//Link the function to the pre-compiled expressions
		if (JitCat::get()->getHasPrecompiledExpression())
		{
			JitCat::get()->setPrecompiledLinkedFunction(getMangledName(Configuration::sretBeforeThisForCurrentProcess, FunctionType::Auto), getFunctionAddress(FunctionType::Auto).functionAddress);
			JitCat::get()->setPrecompiledLinkedFunction(getMangledName(Configuration::sretBeforeThisForCurrentProcess, FunctionType::Static), getFunctionAddress(FunctionType::Static).functionAddress);
			JitCat::get()->setPrecompiledGlobalVariable(getMangledFunctionInfoName(Configuration::sretBeforeThisForCurrentProcess, FunctionType::Member), reinterpret_cast<uintptr_t>(this));
		}
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::any ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
	{
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::size_t ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getNumberOfArguments() const
	{
		return sizeof...(TFunctionArguments);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ReturnT ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::staticExecute(ClassT* base, ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments ...args)
	{
		ReturnT(ClassT::*function)(TFunctionArguments...) const = functionInfo->function;;
		return (base->*function)(args...);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionCallData ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getFunctionAddress(FunctionType functionType) const
	{
		uintptr_t functionPtr = 0;
		MemberFunctionCallType callType = MemberFunctionCallType::Unknown;
		if (shouldUseStaticFunction(functionType))
		{
			memcpy(&functionPtr, &function, sizeof(uintptr_t));
			callType = MemberFunctionCallType::ThisCall;
		}
		else
		{
			functionPtr = reinterpret_cast<uintptr_t>(&staticExecute);
			callType = MemberFunctionCallType::ThisCallThroughStaticFunction;
		}
		return MemberFunctionCallData(functionPtr, reinterpret_cast<uintptr_t>(this), nullptr, callType, false, !std::is_pointer_v<ReturnT>);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::string ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getMangledName(bool sRetBeforeThis, FunctionType functionType) const
	{
		std::string baseName = TypeTraits<ClassT>::toGenericType().getObjectType()->getQualifiedTypeName();
		if (shouldUseStaticFunction(functionType))
		{
			baseName = Tools::append(baseName, "_static");
		}
		return FunctionNameMangler::getMangledFunctionName(returnType, memberFunctionName, argumentTypes, true, baseName, sRetBeforeThis);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline bool ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::shouldUseStaticFunction(FunctionType functionType) const
	{
		//Non-member functions do not have weird function-pointers and can always be called directly
		if (functionType != FunctionType::Member)
		{
			return false;
		}
		//Polymorphic classes have virtual functions and a v-table.
		//To be on the safe size, it's best to call member functions through the static function.
		if constexpr (std::is_polymorphic_v<ClassT>)
		{
			return true;
		}
		else
		{
			//If the size of the function pointer is larger than an ordinary pointer, we should call it through the static function
			//An exception is when the function pointer is twice the size, and the extra data is zero.
			std::size_t functionPtrSize = sizeof(function);
			if (sizeof(function) != Configuration::basicMemberFunctionPointerSize
				&& !(functionPtrSize == 2 * Configuration::basicMemberFunctionPointerSize 
					 && reinterpret_cast<const uintptr_t*>(&function)[1] == 0))
			{
				return true;
			}

			return false;
		}
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	template<std::size_t ...Is>
	inline std::any ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
	{
		ClassT* baseObject = std::any_cast<ClassT*>(base);
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			if constexpr (std::is_void_v<ReturnT>)
			{
				(baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
			}
			else
			{
				return TypeTraits<ReturnT>::getCatValue((baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
			}
		}
		return TypeTraits<ReturnT>::toGenericType().createDefault();
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::PseudoMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(*function)(ClassT*, TFunctionArguments...)) :
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ((void)addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
		//Link the function to the pre-compiled expressions
		if (JitCat::get()->getHasPrecompiledExpression())
		{
			JitCat::get()->setPrecompiledLinkedFunction(getMangledName(Configuration::sretBeforeThisForCurrentProcess, FunctionType::Auto), getFunctionAddress(FunctionType::Auto).functionAddress);
		}
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::any PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
	{
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::size_t PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getNumberOfArguments() const
	{
		return sizeof...(TFunctionArguments);
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ReturnT PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::staticExecute(ClassT* base, PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments ...args)
	{
		ReturnT(*function)(ClassT*, TFunctionArguments...) = functionInfo->function;
		return (*function)(base, args...);
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionCallData PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getFunctionAddress(FunctionType functionType) const
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(pointer, reinterpret_cast<uintptr_t>(this), nullptr, MemberFunctionCallType::PseudoMemberCall, false, !std::is_pointer_v<ReturnT>);
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::string PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getMangledName(bool sRetBeforeThis, FunctionType functionType) const
	{
		std::string baseName = TypeTraits<ClassT>::toGenericType().getObjectType()->getQualifiedTypeName();
		return FunctionNameMangler::getMangledFunctionName(returnType, memberFunctionName, argumentTypes, true, baseName, sRetBeforeThis);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	template<std::size_t ...Is>
	inline std::any PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
	{
		ClassT* baseObject = std::any_cast<ClassT*>(base);
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			if constexpr (std::is_void_v<ReturnT>)
			{
				(*function)(baseObject, TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
			}
			else
			{
				return TypeTraits<ReturnT>::getCatValue((*function)(baseObject, TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
			}

		}
		return TypeTraits<ReturnT>::toGenericType().createDefault();
	}
} //End namespace jitcat::Reflection
