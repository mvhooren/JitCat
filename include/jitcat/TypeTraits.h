/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/Tools.h"

#include <any>
#include <array>
#include <atomic>
#include <map>
#include <memory>
#include <vector>

namespace jitcat
{

	//These classes use template specialization to get properties of types relevant for the reflection, serialisation and expression system.
	//It allows to translate a type T to a CatGenericType and to check if a T is a reflectable/serialisable container.
	//The top class is the default case where T is neither a basic type nor a container type.
	//All other classes are specializations for specific types.
	template <typename ObjectT, typename EnabledT = void>
	class TypeTraits
	{
	public:
		static inline const CatGenericType& toGenericType();
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static inline const char* getTypeName();

		static std::any getCatValue(void) { return std::any(ObjectT());}
		static std::any getCatValue(ObjectT&& value);
		static std::any getCatValue(ObjectT& value);
		static constexpr ObjectT getDefaultValue() { return ObjectT(); }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<ObjectT>::getDefaultValue()); }
		static ObjectT getValue(const std::any& value)  { return std::any_cast<ObjectT>(value);}
		static ObjectT& stripValue(ObjectT& value) { return value; }
		static ObjectT& stripValue(ObjectT* value) {return *value;}
		
		typedef ObjectT getValueType;
		typedef ObjectT type;
		typedef ObjectT cachedType;
		typedef ObjectT functionParameterType;
	};


	template <typename ObjectT>
	class TypeTraits<ObjectT, std::enable_if_t<std::is_abstract_v<ObjectT>>>
	{
	public:
		static inline const CatGenericType& toGenericType();
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static inline const char* getTypeName();

		static std::any getCatValue(ObjectT&& value);
		static std::any getCatValue(ObjectT& value);
		static std::any getDefaultCatValue() { return std::any(TypeTraits<ObjectT>::getDefaultValue()); }
		static ObjectT& stripValue(ObjectT& value) { return value; }
		static ObjectT& stripValue(ObjectT* value) {return *value;}
		
		typedef ObjectT getValueType;
		typedef ObjectT type;
		typedef ObjectT cachedType;
		typedef ObjectT functionParameterType;
	};


	template <typename PointerT>
	class TypeTraits<const PointerT*, void>
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return TypeTraits<PointerT>::getTypeName(); }
		static std::any getCatValue(const PointerT* value) { return const_cast<PointerT*>(value);};
		static constexpr PointerT* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(getDefaultValue()); }
		static const PointerT* getValue(const std::any& value) {return std::any_cast<PointerT*>(value);}
		static PointerT* stripValue(PointerT* value) {return value;}

		typedef PointerT* getValueType;
		typedef PointerT type;
		typedef const PointerT* cachedType;
		typedef PointerT* functionParameterType;
	};


	template <typename PointerT>
	class TypeTraits<PointerT*, void>
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return TypeTraits<PointerT>::getTypeName(); }
		static std::any getCatValue(PointerT* value);
		static constexpr PointerT* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(getDefaultValue()); }
		static PointerT* getValue(const std::any& value) {return std::any_cast<PointerT*>(value);}
		static PointerT* stripValue(PointerT* value) {return value;}

		typedef PointerT* getValueType;
		typedef PointerT type;
		typedef PointerT* cachedType;
		typedef PointerT* functionParameterType;
	};

	
	template <typename RefT>
	class TypeTraits<const RefT&, void>
	{
	public:
		static inline const CatGenericType& toGenericType();

		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return TypeTraits<RefT>::getTypeName(); }
		static std::any getCatValue(void) { return std::any((RefT*)nullptr);}
		static std::any getCatValue(const RefT& value) { return std::any(const_cast<RefT*>(&value));};
		static constexpr RefT* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(getDefaultValue()); }
		static RefT* getValue(const std::any& value)  { return std::any_cast<RefT*>(value);}
		static RefT* stripValue(RefT& value) { return &value; }

		typedef RefT* getValueType;
		typedef RefT type;
		typedef RefT& cachedType;
		typedef RefT& functionParameterType;
	};


	template <typename RefT>
	class TypeTraits<RefT&, void>
	{
	public:
		static inline const CatGenericType& toGenericType();

		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return TypeTraits<RefT>::getTypeName(); }
		static std::any getCatValue(void) { return std::any((RefT*)nullptr);}
		static std::any getCatValue(RefT& value) { return std::any(&value);};
		static constexpr RefT* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(getDefaultValue()); }
		static RefT* getValue(const std::any& value)  { return std::any_cast<RefT*>(value);}
		static RefT* stripValue(RefT& value) { return &value; }
		
		typedef RefT* getValueType;
		typedef RefT type;
		typedef RefT& cachedType;
		typedef RefT& functionParameterType;
	};


	template <typename PointerRefT>
	class TypeTraits<PointerRefT*&, void>
	{
	public:
		static inline const CatGenericType& toGenericType();

		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return TypeTraits<PointerRefT>::getTypeName(); }
		static std::any getCatValue(void) 
		{
			return std::any(getDefaultValue());
		}
		static std::any getCatValue(PointerRefT*& value) { return std::any(&value);};
		static PointerRefT** getDefaultValue() 
		{
			static PointerRefT* nullReflectable = nullptr;
			return &nullReflectable;
		}
		static std::any getDefaultCatValue() { return getValue(); }
		static PointerRefT** getValue(const std::any& value)  { return std::any_cast<PointerRefT**>(value);}
		static PointerRefT** stripValue(PointerRefT*& value) { return &value; }
		
		typedef PointerRefT** getValueType;
		typedef PointerRefT type;
		typedef PointerRefT* cachedType;
		typedef PointerRefT*& functionParameterType;
	};


	template <typename UniquePtrT>
	class TypeTraits<std::unique_ptr<UniquePtrT>, void>
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return true; }

		static const char* getTypeName() { return TypeTraits<UniquePtrT>::getTypeName(); }
		static std::any getCatValue(std::unique_ptr<UniquePtrT>& value);
		static constexpr UniquePtrT* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::unique_ptr<UniquePtrT>>::getDefaultValue()); }
		static UniquePtrT* getValue(const std::any& value) {return static_cast<UniquePtrT*>(std::any_cast<UniquePtrT*>(value));}
		static UniquePtrT* stripValue(std::unique_ptr<UniquePtrT>& value) { return value.get(); }
		
		typedef UniquePtrT* getValueType;
		typedef UniquePtrT type;
		typedef UniquePtrT* cachedType;
		typedef UniquePtrT* functionParameterType;
	};
	
	
	template <>
	class TypeTraits<void, void>
	{
	public:
		static inline const CatGenericType& toGenericType() { return CatGenericType::voidType; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(void) { return std::any();}
		static constexpr void getDefaultValue() { return; }
		static std::any getDefaultCatValue() { return std::any(); }
		static void stripValue(void) { }
		static void getValue() { return;}
		static void getValue(const std::any& value) { return;}
		

		static constexpr const char* getTypeName()
		{
			return "void"; 
		}

		typedef void getValueType;
		typedef void type;
		typedef int cachedType;
		typedef int functionParameterType;
	};

	
	template <typename FundamentalT>
	class TypeTraits<FundamentalT, std::enable_if_t<(std::is_fundamental_v<FundamentalT> && !std::is_void_v<FundamentalT>) || std::is_same_v<FundamentalT, std::array<float, 4>> > >
	{
	public:
		static inline const CatGenericType& toGenericType() 
		{ 
			if constexpr		(std::is_same_v<float,				 FundamentalT>)	return CatGenericType::floatType; 
			else if constexpr	(std::is_same_v<std::array<float,4>, FundamentalT>)	return CatGenericType::vector4fType; 
			else if constexpr	(std::is_same_v<double,				 FundamentalT>)	return CatGenericType::doubleType; 
			else if constexpr	(std::is_same_v<char,				 FundamentalT>)	return CatGenericType::charType;
			else if constexpr	(std::is_same_v<unsigned char,		 FundamentalT>)	return CatGenericType::uCharType;
			else if constexpr	(std::is_same_v<int,				 FundamentalT>)	return CatGenericType::intType;
			else if constexpr	(std::is_same_v<unsigned int,		 FundamentalT>)	return CatGenericType::uIntType;
			else if constexpr	(std::is_same_v<int64_t,			 FundamentalT>)	return CatGenericType::int64Type;
			else if constexpr	(std::is_same_v<uint64_t,			 FundamentalT>)	return CatGenericType::uInt64Type;
			else if constexpr	(std::is_same_v<bool,				 FundamentalT>)	return CatGenericType::boolType;
			else static_assert(std::is_same_v<bool,  FundamentalT>, "Fundamental type not yet supported by JitCat.");
		}

		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(FundamentalT value) { return std::any(value);}
		static constexpr FundamentalT getDefaultValue() { return FundamentalT(); }
		static std::any getDefaultCatValue() { return std::any(FundamentalT()); }
		static FundamentalT getValue(const std::any& value) { return std::any_cast<FundamentalT>(value);}
		static FundamentalT stripValue(FundamentalT value) { return value; }
		static inline const char* getTypeName();

		typedef FundamentalT getValueType;
		typedef FundamentalT type;
		typedef FundamentalT cachedType;
		typedef FundamentalT functionParameterType;
	};


	template <typename EnumT>
	class TypeTraits<EnumT, std::enable_if_t<std::is_enum_v<EnumT>>>
	{
	public:
		static const CatGenericType& toGenericType();

		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(EnumT value) { return std::any(value);}
		static EnumT getDefaultValue() { return EnumT(); }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<EnumT>::getDefaultValue()); }
		static EnumT getValue(const std::any& value) { return std::any_cast<EnumT>(value);}
		static EnumT stripValue(EnumT value) { return value; }
		static inline const char* getTypeName();

		typedef EnumT getValueType;
		typedef EnumT type;
		typedef EnumT cachedType;
		typedef EnumT functionParameterType;
	};

} //End namespace jitcat

#include "jitcat/TypeTraitsHeaderImplementation.h"