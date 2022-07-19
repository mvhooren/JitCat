#pragma once

#include "jitcat/TypeInfo.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeTraits.h"

#include <array>
#include <memory>
#include <type_traits>
#include <vector>


namespace jitcat::Reflection
{
	class VectorTypeInfo: public TypeInfo
	{
	private:
		VectorTypeInfo(CatGenericType scalarType, unsigned int length, 
					   std::unique_ptr<jitcat::Reflection::TypeCaster>&& typeCaster);
	protected:
		virtual ~VectorTypeInfo();

	public:
		template<typename ScalarT, unsigned int length>
		static VectorTypeInfo& createVectorType();

		const CatGenericType& getScalarType() const;
		// The size in bytes of each scalar in the vector.
		std::size_t getScalarSize() const;
		// The size in bytes of the vector.
		std::size_t getSize() const;
		// The number of scalar elements in the vector.
		std::size_t getLength() const;

		//from TypeInfo:
		virtual bool isVectorType() const override final;
		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const override final;
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize) override final;
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual bool isTriviallyConstructable() const override final;

		virtual bool getAllowInheritance() const override final;
		virtual bool inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual bool getAllowConstruction() const override final;
		virtual bool getAllowCopyConstruction() const override final;
		virtual bool getAllowMoveConstruction() const override final;

	private:
		CatGenericType scalarType;
		unsigned int length;

		static const char* getVectorTypeName(const CatGenericType& scalarType, unsigned int length);
		static std::vector<VectorTypeInfo*> vectorTypes;
		
	};

	template<typename ScalarT, unsigned int length>
	inline VectorTypeInfo& VectorTypeInfo::createVectorType()
	{
		static_assert(std::is_scalar_v<ScalarT>, "ScalarT is not a scalar");
		const CatGenericType& scalarType = TypeTraits<ScalarT>::toGenericType();
		for (auto& iter : vectorTypes)
		{
			if (iter->getScalarType().compare(scalarType, false, false))
			{
				return *(iter);
			}
		}
		vectorTypes.push_back(new VectorTypeInfo(scalarType, length, std::make_unique<ObjectTypeCaster<std::array<ScalarT, length>>>()));
		return *(vectorTypes.back());
	}
}