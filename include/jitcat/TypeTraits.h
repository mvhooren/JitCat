/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

#include <any>
#include <map>
#include <memory>
#include <string>
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

		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return ObjectT::getTypeName(); }
		static std::any getCatValue(void) { return std::any(ObjectT());}
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
		typedef ObjectT* containerItemReturnType;
	};


	template <typename PointerT>
	class TypeTraits<PointerT*, void>
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return PointerT::getTypeName(); }
		static std::any getCatValue(PointerT* value);
		static constexpr PointerT* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(getDefaultValue()); }
		static PointerT* getValue(const std::any& value) {return std::any_cast<PointerT*>(value);}
		static PointerT* stripValue(PointerT* value) {return value;}

		typedef PointerT* getValueType;
		typedef PointerT type;
		typedef PointerT* cachedType;
		typedef PointerT* functionParameterType;
		typedef PointerT* containerItemReturnType;
	};


	template <typename RefT>
	class TypeTraits<RefT&, void>
	{
	public:
		static inline const CatGenericType& toGenericType();

		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return RefT::getTypeName(); }
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
		typedef RefT* containerItemReturnType;
	};


	template <typename PointerRefT>
	class TypeTraits<PointerRefT*&, void>
	{
	public:
		static inline const CatGenericType& toGenericType();

		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return PointerRefT::getTypeName(); }
		static std::any getCatValue(void) 
		{
			return std::any(getDefaultValue());
		}
		static std::any getCatValue(PointerRefT*& value) { return std::any(&value);};
		static constexpr PointerRefT** getDefaultValue() 
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
		typedef PointerRefT** containerItemReturnType;
	};


	template <typename UniquePtrT>
	class TypeTraits<std::unique_ptr<UniquePtrT>, void>
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return true; }

		static const char* getTypeName() { return UniquePtrT::getTypeName(); }
		static std::any getCatValue(std::unique_ptr<UniquePtrT>& value);
		static constexpr UniquePtrT* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::unique_ptr<UniquePtrT>>::getDefaultValue()); }
		static UniquePtrT* getValue(const std::any& value) {return static_cast<UniquePtrT*>(std::any_cast<UniquePtrT*>(value));}
		static UniquePtrT* stripValue(std::unique_ptr<UniquePtrT>& value) { return value.get(); }

		typedef UniquePtrT* getValueType;
		typedef UniquePtrT type;
		typedef UniquePtrT* cachedType;
		typedef UniquePtrT* functionParameterType;
		typedef UniquePtrT* containerItemReturnType;
	};


	template <>
	class TypeTraits<void, void>
	{
	public:
		static inline const CatGenericType& toGenericType() { return CatGenericType::voidType; }
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		template <typename U>
		static std::any getCatValue(const U& param) { return std::any();}
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
		typedef int containerItemReturnType;
	};


	template <typename FundamentalT>
	class TypeTraits<FundamentalT, std::enable_if_t<std::is_fundamental_v<FundamentalT> && !std::is_void_v<FundamentalT> > >
	{
	public:
		static inline const CatGenericType& toGenericType() 
		{ 
			if constexpr		(std::is_same_v<float, FundamentalT>)	return CatGenericType::floatType; 
			else if constexpr	(std::is_same_v<int,   FundamentalT>)	return CatGenericType::intType;
			else if constexpr	(std::is_same_v<bool,  FundamentalT>)	return CatGenericType::boolType;
			else														static_assert(false, "Fundamental type not yet supported by JitCat.");
		}

		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(FundamentalT value) { return std::any(value);}
		static constexpr FundamentalT getDefaultValue() { return FundamentalT(); }
		static std::any getDefaultCatValue() { return std::any(FundamentalT()); }
		static FundamentalT getValue(const std::any& value) { return std::any_cast<FundamentalT>(value);}
		static FundamentalT stripValue(FundamentalT value) { return value; }
		static constexpr const char* getTypeName()
		{
			if constexpr		(std::is_same_v<float, FundamentalT>)	return "float"; 
			else if constexpr	(std::is_same_v<int,   FundamentalT>)	return "int";
			else if constexpr	(std::is_same_v<bool,  FundamentalT>)	return "bool";
			else														static_assert(false, "Fundamental type not yet supported by JitCat.");	
		}

		typedef FundamentalT getValueType;
		typedef FundamentalT type;
		typedef FundamentalT cachedType;
		typedef FundamentalT functionParameterType;
		typedef FundamentalT containerItemReturnType;
	};


	template <typename StringT>
	class TypeTraits<StringT, std::enable_if_t<std::is_same_v<std::remove_cv_t<StringT>, std::string>>>
	{
	public:
		static const CatGenericType& toGenericType() { return CatGenericType::stringType; }
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(const std::string& value) { return std::any(value);}
		static std::string getDefaultValue() { return Tools::empty; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::string>::getDefaultValue()); }
		static std::string getValue(const std::any& value) { return std::any_cast<std::string>(value);}
		static const std::string& stripValue(const std::string& value) { return value; }
		static const char* getTypeName()
		{
			return "string";
		}

		typedef std::string getValueType;
		typedef std::string type;
		typedef std::string cachedType;
		typedef const std::string& functionParameterType;
		typedef std::string containerItemReturnType;
	};


	template <typename ItemType, typename AllocatorT>
	class TypeTraits<std::vector<ItemType, AllocatorT>, void >
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return true; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return ""; }
		static std::any getCatValue(void) { return std::any((std::vector<ItemType, AllocatorT>*)nullptr);}
		static constexpr std::vector<ItemType, AllocatorT>* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::vector<ItemType, AllocatorT> >::getDefaultValue()); }
		static std::vector<ItemType, AllocatorT>& getValue(const std::any& value) { *std::any_cast<std::vector<ItemType, AllocatorT>*>(value); }
		static std::vector<ItemType, AllocatorT>* stripValue(std::vector<ItemType, AllocatorT>* value) { return value; }

		typedef std::vector<ItemType, AllocatorT>& getValueType;
		typedef ItemType type;
		typedef std::vector<ItemType, AllocatorT> cachedType;
		typedef std::vector<ItemType, AllocatorT>* functionParameterType;
		typedef std::vector<ItemType, AllocatorT>* containerItemReturnType;
	};


	template <typename KeyType, typename ItemType, typename ComparatorT, typename AllocatorT>
	class TypeTraits<std::map<KeyType, ItemType, ComparatorT, AllocatorT>, void >
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return true; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return ""; }
		static std::any getCatValue(void) { return std::any((std::map<KeyType, ItemType, ComparatorT, AllocatorT>*)nullptr);}
		static constexpr std::map<KeyType, ItemType, ComparatorT, AllocatorT>* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::map<KeyType, ItemType, ComparatorT, AllocatorT> >::getDefaultValue()); }
		static std::map<KeyType, ItemType, ComparatorT, AllocatorT>& getValue(const std::any& value) { return *std::any_cast<std::map<KeyType, ItemType, ComparatorT, AllocatorT>*>(value);}
		static std::map<KeyType, ItemType, ComparatorT, AllocatorT>* stripValue(std::map<KeyType, ItemType, ComparatorT, AllocatorT>* value) { return value; }

		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT>& getValueType;
		typedef ItemType type;
		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT> cachedType;
		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT>* functionParameterType;
		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT>* containerItemReturnType;
	};

	template <typename AnyType>
	struct RemoveConst
	{
		typedef AnyType type;
	};
	template<typename ConstType>
	struct RemoveConst<ConstType&>
	{
		typedef typename RemoveConst<ConstType>::type& type;
	};
	template<typename ConstType>
	struct RemoveConst<ConstType*>
	{
		typedef typename RemoveConst<ConstType>::type* type;
	};
	template<typename ConstType>
	struct RemoveConst<ConstType* const>
	{
		typedef typename RemoveConst<ConstType>::type* type;
	};
	template<typename ConstType>
	struct RemoveConst<const ConstType>
	{
		typedef typename RemoveConst<ConstType>::type type;
	};
	template<typename ConstType>
	struct RemoveConst<volatile ConstType>
	{
		typedef typename RemoveConst<ConstType>::type type;
	};

} //End namespace jitcat

#include "jitcat/TypeTraitsHeaderImplementation.h"