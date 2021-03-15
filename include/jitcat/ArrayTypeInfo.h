#pragma once

#include "TypeInfo.h"
#include <vector>


namespace jitcat::Reflection
{
	class ArrayTypeInfo: public TypeInfo
	{
	public:
		struct Array
		{
			Array():
				arrayData(nullptr),
				size(0)
			{}
			unsigned char* arrayData;
			//current size in number of items (not bytes)
			int size;
		};
	private:
		ArrayTypeInfo(CatGenericType arrayItemType);
	protected:
		virtual ~ArrayTypeInfo();

	public:
		std::any index(Array* array, int index);

		static ArrayTypeInfo& createArrayTypeOf(const CatGenericType& arrayItemType);
		static void deleteArrayTypeOfType(const CatGenericType& arrayItemType);

		const CatGenericType& getArrayItemType() const;
		std::size_t getItemSize() const;

		//from TypeInfo:
		virtual bool isArrayType() const override final;
		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const override final;
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize) override final;
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;

		virtual bool getAllowInheritance() const override final;
		virtual bool inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual bool getAllowConstruction() const override final;
		virtual bool getAllowCopyConstruction() const override final;
		virtual bool getAllowMoveConstruction() const override final;

	private:
		CatGenericType arrayItemType;

		static std::vector<ArrayTypeInfo*> arrayTypes;
	};
}