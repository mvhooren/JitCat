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
				size(0),
				reserved(0)
			{}
			unsigned char* arrayData;
			//current size in number of items (not bytes)
			int size;
			//reserved size in number of items (not bytes)
			int reserved;
		};
	private:
		ArrayTypeInfo(CatGenericType arrayItemType);
	protected:
		virtual ~ArrayTypeInfo();

	public:
		int add(Array* array, const std::any& value);
		void remove(Array* array, int index);
		std::any index(Array* array, int index);

		static ArrayTypeInfo& createArrayTypeOf(CatGenericType arrayItemType);
		static void deleteArrayTypeOfType(CatGenericType arrayItemType);

		const CatGenericType& getArrayItemType() const;

		//from TypeInfo:
		virtual bool isArrayType() const override final;
		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const override final;
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize) override final;
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;

		virtual bool getAllowInheritance() const;
		virtual bool inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext);
		virtual bool getAllowConstruction() const;
		virtual bool getAllowCopyConstruction() const;
		virtual bool getAllowMoveConstruction() const;

	private:
		CatGenericType arrayItemType;

		static std::vector<ArrayTypeInfo*> arrayTypes;
	};
}