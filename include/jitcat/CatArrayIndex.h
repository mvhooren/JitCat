/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatGenericType.h"

#include <memory>

namespace jitcat::AST
{

	class CatArrayIndex: public CatTypedExpression
	{
	public:
		CatArrayIndex(CatTypedExpression* base, CatTypedExpression* arrayIndex);
		CatArrayIndex(const CatArrayIndex&) = delete;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual CatGenericType typeCheck() override final;

		virtual CatGenericType getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;

		CatTypedExpression* getBase() const;
		CatTypedExpression* getIndex() const;

	private:
		CatGenericType arrayType;
		std::unique_ptr<CatTypedExpression> array;
		CatGenericType indexType;
		std::unique_ptr<CatTypedExpression> index;
		CatGenericType containerItemType;
	};


} //End namespace jitcat::AST