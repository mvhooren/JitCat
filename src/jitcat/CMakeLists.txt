project(JitCat)

set(JitCatHeaderPath ${JitCatIncludePath}/jitcat)

include_directories(${JitCatIncludePath})

file(GLOB JitCatHeaderFiles ${JitCatHeaderPath}/*.h)

if (JitCatHeaderFiles)
	set_source_files_properties(${JitCatHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)
endif()

#Create a list variable for every virtual folder of source files.
#These lists are then used in source_group and add_library

set(Source_Root
	${JitCatHeaderPath}/Configuration.h
	JitCat.cpp
	${JitCatHeaderPath}/JitCat.h
)
if(LLVM_ENABLED AND NOT ${LLVM_NATVIS_FILE} STREQUAL "")
	list(APPEND Source_Root ${LLVM_NATVIS_FILE})
endif()

set(Source_Cat
	CatGenericType.cpp
	${JitCatHeaderPath}/CatGenericType.h
	CatGrammar.cpp
	${JitCatHeaderPath}/CatGrammar.h
	${JitCatHeaderPath}/CatGrammarType.h
	CatRuntimeContext.cpp
	${JitCatHeaderPath}/CatRuntimeContext.h
	${JitCatHeaderPath}/CatScopeID.h
	${JitCatHeaderPath}/CatTokenIds.h
	CatTokenizer.cpp
	${JitCatHeaderPath}/CatTokenizer.h
)

set(Source_Cat_ASTNodes
	CatArgumentList.cpp 
	${JitCatHeaderPath}/CatArgumentList.h 
	${JitCatHeaderPath}/CatAssignableExpression.h
	${JitCatHeaderPath}/CatASTNode.h 
	${JitCatHeaderPath}/CatASTNodes.h 
	${JitCatHeaderPath}/CatASTNodesDeclares.h 
	${JitCatHeaderPath}/CatASTNodeType.h 
	${JitCatHeaderPath}/CatAssignmentOperator.h 
	CatAssignmentOperator.cpp 
	CatBuiltInFunctionCall.cpp
	${JitCatHeaderPath}/CatBuiltInFunctionCall.h
	${JitCatHeaderPath}/CatBuiltInFunctionType.h
	CatClassDefinition.cpp
	${JitCatHeaderPath}/CatClassDefinition.h
	CatConstruct.cpp
	${JitCatHeaderPath}/CatConstruct.h
	${JitCatHeaderPath}/CatDefinition.h
	CatDestruct.cpp
	${JitCatHeaderPath}/CatDestruct.h
	CatErrorExpression.cpp
	${JitCatHeaderPath}/CatErrorExpression.h
	CatForLoop.cpp
	${JitCatHeaderPath}/CatForLoop.h
	CatFunctionDefinition.cpp
	${JitCatHeaderPath}/CatFunctionDefinition.h
	CatFunctionOrConstructor.cpp
	${JitCatHeaderPath}/CatFunctionOrConstructor.h
	CatFunctionParameterDefinitions.cpp
	${JitCatHeaderPath}/CatFunctionParameterDefinitions.h
	CatIdentifier.cpp
	${JitCatHeaderPath}/CatIdentifier.h
	CatIfStatement.cpp
	${JitCatHeaderPath}/CatIfStatement.h
	CatIndirectionConversion.cpp
	${JitCatHeaderPath}/CatIndirectionConversion.h
	CatInfixOperator.cpp
	${JitCatHeaderPath}/CatInfixOperator.h
	${JitCatHeaderPath}/CatInfixOperatorHeaderImplementation.h
	CatInfixOperatorType.cpp
	${JitCatHeaderPath}/CatInfixOperatorType.h
	CatInheritanceDefinition.cpp
	${JitCatHeaderPath}/CatInheritanceDefinition.h
	CatLinkNode.cpp
	${JitCatHeaderPath}/CatLinkNode.h
	CatLiteral.cpp
	${JitCatHeaderPath}/CatLiteral.h
	CatMemberAccess.cpp
	${JitCatHeaderPath}/CatMemberAccess.h
	CatMemberFunctionCall.cpp
	${JitCatHeaderPath}/CatMemberFunctionCall.h
	CatOperatorNew.cpp
	${JitCatHeaderPath}/CatOperatorNew.h
	CatOwnershipSemanticsNode.cpp
	${JitCatHeaderPath}/CatOwnershipSemanticsNode.h
	CatPrefixOperator.cpp
	${JitCatHeaderPath}/CatPrefixOperator.h
	CatRange.cpp
	${JitCatHeaderPath}/CatRange.h
	CatReturnStatement.cpp
	${JitCatHeaderPath}/CatReturnStatement.h
	${JitCatHeaderPath}/CatScope.h
	CatScopeBlock.cpp
	${JitCatHeaderPath}/CatScopeBlock.h
	CatScopeFunctionCall.cpp
	${JitCatHeaderPath}/CatScopeFunctionCall.h
	CatScopeRoot.cpp
	${JitCatHeaderPath}/CatScopeRoot.h
	CatStaticMemberAccess.cpp
	${JitCatHeaderPath}/CatStaticMemberAccess.h
	CatStaticScope.cpp
	${JitCatHeaderPath}/CatStaticScope.h
	CatTypeNode.cpp
	CatTypeOrIdentifier.cpp
	${JitCatHeaderPath}/CatTypeOrIdentifier.h
	CatSourceFile.cpp
	${JitCatHeaderPath}/CatSourceFile.h
	CatStaticFunctionCall.cpp
	${JitCatHeaderPath}/CatStaticFunctionCall.h
	CatStaticIdentifier.cpp
	${JitCatHeaderPath}/CatStaticIdentifier.h
	${JitCatHeaderPath}/CatStatement.h
	${JitCatHeaderPath}/CatTypeNode.h
	${JitCatHeaderPath}/CatTypedExpression.h
	CatVariableDeclaration.cpp
	${JitCatHeaderPath}/CatVariableDeclaration.h
	CatVariableDefinition.cpp
	${JitCatHeaderPath}/CatVariableDefinition.h
)

set(Source_Cat_ASTNodes_ASTHelper
	InfixOperatorOptimizer.cpp
	${JitCatHeaderPath}/InfixOperatorOptimizer.h
	ASTHelper.cpp
	${JitCatHeaderPath}/ASTHelper.h
	${JitCatHeaderPath}/IndirectionConversionMode.h
	InfixOperatorResultInfo.cpp
	${JitCatHeaderPath}/InfixOperatorResultInfo.h
	StringConstantPool.cpp
	${JitCatHeaderPath}/StringConstantPool.h
)

set(Source_Cat_CodeGeneration
	${JitCatHeaderPath}/PrecompilationContext.h
)

set(Source_Cat_CodeGeneration_LLVM
	LLVMCatIntrinsics.cpp
	${JitCatHeaderPath}/LLVMCatIntrinsics.h
	${JitCatHeaderPath}/LLVMCodeGenerator.h
	${JitCatHeaderPath}/LLVMCodeGeneratorHelper.h
	${JitCatHeaderPath}/LLVMCompileOptions.h
	${JitCatHeaderPath}/LLVMCompileTimeContext.h
	${JitCatHeaderPath}/LLVMForwardDeclares.h
	${JitCatHeaderPath}/LLVMMemoryManager.h
	LLVMPreGeneratedExpression.cpp
	${JitCatHeaderPath}/LLVMPreGeneratedExpression.h
	LLVMTypes.cpp
	${JitCatHeaderPath}/LLVMTypes.h
)

if(LLVM_ENABLED)
	list(APPEND Source_Cat_CodeGeneration_LLVM 
		${JitCatHeaderPath}/LLVMJit.h
		${JitCatHeaderPath}/LLVMJitHeaderImplementation.h
		LLVMPrecompilationContext.cpp
		${JitCatHeaderPath}/LLVMPrecompilationContext.h
		LLVMCodeGenerator.cpp
		LLVMCodeGeneratorHelper.cpp
		LLVMCompileTimeContext.cpp
		LLVMJit.cpp
		LLVMMemoryManager.cpp)
endif()

set(Source_Cat_Expression
	CatLib.cpp
	${JitCatHeaderPath}/CatLib.h
	${JitCatHeaderPath}/Expression.h
	ExpressionAny.cpp
	${JitCatHeaderPath}/ExpressionAny.h
	ExpressionAssignAny.cpp
	${JitCatHeaderPath}/ExpressionAssignAny.h
	${JitCatHeaderPath}/ExpressionAssignment.h
	${JitCatHeaderPath}/ExpressionAssignmentHeaderImplementation.h
	ExpressionBase.cpp
	${JitCatHeaderPath}/ExpressionBase.h	
	${JitCatHeaderPath}/ExpressionHeaderImplementation.h
	ExpressionHelperFunctions.cpp
	${JitCatHeaderPath}/ExpressionHelperFunctions.h
)

set(Source_Cat_Expression_Autocompletion
	AutoCompletion.cpp
	${JitCatHeaderPath}/AutoCompletion.h
)

set(Source_Cat_Expression_Errors
	ErrorContext.cpp
	${JitCatHeaderPath}/ErrorContext.h
	ExpressionErrorManager.cpp
	${JitCatHeaderPath}/ExpressionErrorManager.h
)

set(Source_Cat_SourceFile
	SourceFile.cpp
	${JitCatHeaderPath}/SourceFile.h
)

set(Source_Grammar
	GrammarBase.cpp
	${JitCatHeaderPath}/GrammarBase.h
	Production.cpp
	${JitCatHeaderPath}/Production.h
	ProductionEpsilonToken.cpp
	${JitCatHeaderPath}/ProductionEpsilonToken.h
	ProductionNonTerminalToken.cpp
	${JitCatHeaderPath}/ProductionNonTerminalToken.h
	ProductionRule.cpp
	${JitCatHeaderPath}/ProductionRule.h
	ProductionTerminalToken.cpp
	${JitCatHeaderPath}/ProductionTerminalToken.h
	ProductionToken.cpp
	${JitCatHeaderPath}/ProductionToken.h
	ProductionTokenSet.cpp
	${JitCatHeaderPath}/ProductionTokenSet.h
	ProductionTokenSetMember.cpp
	${JitCatHeaderPath}/ProductionTokenSetMember.h
	${JitCatHeaderPath}/ProductionTokenType.h
	${JitCatHeaderPath}/TokenFlag.h
)

set(Source_Parser
	${JitCatHeaderPath}/ASTNode.h
	ASTNodeParser.cpp
	${JitCatHeaderPath}/ASTNodeParser.h
	DFAState.cpp
	${JitCatHeaderPath}/DFAState.h
	Item.cpp
	${JitCatHeaderPath}/Item.h
	${JitCatHeaderPath}/RuntimeContext.h
	SLRParser.cpp
	${JitCatHeaderPath}/SLRParser.h
	SLRParseResult.cpp
	${JitCatHeaderPath}/SLRParseResult.h
	${JitCatHeaderPath}/StackItem.h
	StackItemProduction.cpp
	${JitCatHeaderPath}/StackItemProduction.h
	${JitCatHeaderPath}/StackItemToken.h
)

set(Source_Reflection
	${JitCatHeaderPath}/CustomObject.h
	${JitCatHeaderPath}/ExternalReflector.h
	Reflectable.cpp
	${JitCatHeaderPath}/Reflectable.h
	ObjectInstance.cpp
	${JitCatHeaderPath}/ObjectInstance.h
	ReflectableHandle.cpp
	${JitCatHeaderPath}/ReflectableHandle.h
	${JitCatHeaderPath}/STLTypeReflectors.h
	${JitCatHeaderPath}/STLTypeReflectorsHeaderImplementation.h
	TypeRegistry.cpp
	${JitCatHeaderPath}/TypeRegistry.h
)

set(Source_Reflection_Enums
	${JitCatHeaderPath}/MemberFlags.h
)

set(Source_Reflection_Serialisation
	XMLHelper.cpp
	${JitCatHeaderPath}/XMLHelper.h
)

set(Source_Reflection_TypeInfo
	ArrayTypeInfo.cpp
	${JitCatHeaderPath}/ArrayTypeInfo.h
	CustomTypeInfo.cpp
	${JitCatHeaderPath}/CustomTypeInfo.h
	ReflectedEnumTypeInfo.cpp
	${JitCatHeaderPath}/ReflectedEnumTypeInfo.h
	ReflectedTypeInfo.cpp
	${JitCatHeaderPath}/ReflectedTypeInfo.h
	${JitCatHeaderPath}/ReflectedTypeInfoHeaderImplementation.h
	TypeInfo.cpp
	${JitCatHeaderPath}/TypeInfo.h
	TypeInfoDeleter.cpp
	${JitCatHeaderPath}/TypeInfoDeleter.h
)

set(Source_Reflection_TypeInfo_Members
	CustomTypeMemberInfo.cpp
	${JitCatHeaderPath}/CustomTypeMemberInfo.h
	${JitCatHeaderPath}/CustomTypeMemberInfoHeaderImplementation.h
	MemberInfo.cpp
	${JitCatHeaderPath}/MemberInfo.h
	${JitCatHeaderPath}/MemberInfoHeaderImplementation.h
	StaticConstMemberInfo.cpp
	${JitCatHeaderPath}/StaticConstMemberInfo.h
	StaticMemberInfo.cpp
	${JitCatHeaderPath}/StaticMemberInfo.h
	${JitCatHeaderPath}/StaticMemberInfoHeaderImplementation.h
	TypeMemberInfo.cpp;
	${JitCatHeaderPath}/TypeMemberInfo.h
)

set(Source_Reflection_TypeInfo_MemberFunctions
	ArrayMemberFunctionInfo.cpp
	${JitCatHeaderPath}/ArrayMemberFunctionInfo.h
	${JitCatHeaderPath}/BuildIndicesHelper.h
	CustomTypeMemberFunctionInfo.cpp
	${JitCatHeaderPath}/CustomTypeMemberFunctionInfo.h
	FunctionSignature.cpp
	${JitCatHeaderPath}/FunctionSignature.h
	MemberFunctionInfo.cpp
	${JitCatHeaderPath}/MemberFunctionInfo.h
	${JitCatHeaderPath}/MemberFunctionInfoHeaderImplementation.h
	StaticMemberFunctionInfo.cpp
	${JitCatHeaderPath}/StaticMemberFunctionInfo.h
	${JitCatHeaderPath}/StaticMemberFunctionInfoHeaderImplementation.h
	${JitCatHeaderPath}/TypeConversionCastHelper.h
)

set(Source_Reflection_TypeInfo_Helper
	FunctionNameMangler.cpp
	${JitCatHeaderPath}/FunctionNameMangler.h
	${JitCatHeaderPath}/FunctionPresenceTest.h
	${JitCatHeaderPath}/HandleTrackingMethod.h
	${JitCatHeaderPath}/MemberTypeInfoCreator.h
	${JitCatHeaderPath}/MemberVisibility.h
	TypeCaster.cpp
	${JitCatHeaderPath}/TypeCaster.h
	${JitCatHeaderPath}/TypeCasterHeaderImplementation.h
	${JitCatHeaderPath}/TypeTools.h
	${JitCatHeaderPath}/TypeOwnershipSemantics.h
	${JitCatHeaderPath}/TypeTraits.h
	${JitCatHeaderPath}/TypeTraitsHeaderImplementation.h
	${JitCatHeaderPath}/VariableEnumerator.h
)

set(Source_Tokenizer
	Document.cpp
	${JitCatHeaderPath}/Document.h
	${JitCatHeaderPath}/DocumentSelection.h
	${JitCatHeaderPath}/Lexeme.h
	ParseToken.cpp
	${JitCatHeaderPath}/ParseToken.h
	TokenizerBase.cpp
	${JitCatHeaderPath}/TokenizerBase.h
)

set(Source_Tokenizer_Tokens
	CommentToken.cpp
	${JitCatHeaderPath}/CommentToken.h
	${JitCatHeaderPath}/CommentTokenSubTypes.h
	ConstantToken.cpp
	${JitCatHeaderPath}/ConstantToken.h
	${JitCatHeaderPath}/ConstantTokenSubTypes.h
	ErrorToken.cpp
	${JitCatHeaderPath}/ErrorToken.h
	IdentifierToken.cpp
	${JitCatHeaderPath}/IdentifierToken.h
	${JitCatHeaderPath}/IdentifierTokenSubTypes.h
	OneCharToken.cpp
	${JitCatHeaderPath}/OneCharToken.h
	${JitCatHeaderPath}/OneCharTokenSubTypes.h
	ParseHelper.cpp
	${JitCatHeaderPath}/ParseHelper.h
	TwoCharToken.cpp
	${JitCatHeaderPath}/TwoCharToken.h
	${JitCatHeaderPath}/TwoCharTokenSubTypes.h
	WhitespaceToken.cpp
	${JitCatHeaderPath}/WhitespaceToken.h
)

set(Source_Tools
	CatLog.cpp
	${JitCatHeaderPath}/CatLog.h
	Timer.cpp
	${JitCatHeaderPath}/Timer.h
	Tools.cpp
	${JitCatHeaderPath}/Tools.h
	${JitCatHeaderPath}/ToolsHeaderImplementation.h
)

#Project file organisation
source_group(Cat FILES ${Source_Cat})
source_group(Cat\\ASTNodes FILES ${Source_Cat_ASTNodes})
source_group(Cat\\ASTNodes\\ASTHelper FILES ${Source_Cat_ASTNodes_ASTHelper})
source_group(Cat\\CodeGeneration FILES ${Source_Cat_CodeGeneration})
source_group(Cat\\CodeGeneration\\LLVM FILES ${Source_Cat_CodeGeneration_LLVM})
source_group(Cat\\Expression FILES ${Source_Cat_Expression})
source_group(Cat\\Expression\\AutoCompletion FILES ${Source_Cat_Expression_Autocompletion})
source_group(Cat\\Expression\\Errors FILES ${Source_Cat_Expression_Errors})
source_group(Cat\\SourceFile FILES ${Source_Cat_SourceFile})
source_group(Grammar FILES ${Source_Grammar})
source_group(Parser FILES ${Source_Parser})
source_group(Reflection FILES ${Source_Reflection})
source_group(Reflection\\Enums FILES ${Source_Reflection_Enums})
source_group(Reflection\\Serialisation FILES ${Source_Reflection_Serialisation})
source_group(Reflection\\TypeInfo FILES ${Source_Reflection_TypeInfo})
source_group(Reflection\\TypeInfo\\Members FILES ${Source_Reflection_TypeInfo_Members})
source_group(Reflection\\TypeInfo\\MemberFunctions FILES ${Source_Reflection_TypeInfo_MemberFunctions})
source_group(Reflection\\TypeInfo\\Helper FILES ${Source_Reflection_TypeInfo_Helper})
source_group(Tokenizer FILES ${Source_Tokenizer})
source_group(Tokenizer\\Tokens FILES ${Source_Tokenizer_Tokens})
source_group(Tools FILES ${Source_Tools})
source_group("" FILES ${Source_Root} llvm.natvis)

if(NOT MSVC)
	add_compile_options(-fPIC)
endif()

add_library(JitCat STATIC 
	${Source_Root}
	${Source_Cat}
	${Source_Cat_ASTNodes}
	${Source_Cat_ASTNodes_ASTHelper}
	${Source_Cat_CodeGeneration}
	${Source_Cat_CodeGeneration_LLVM}
	${Source_Cat_Expression}
	${Source_Cat_Expression_Autocompletion}
	${Source_Cat_Expression_Errors}
	${Source_Cat_SourceFile}
	${Source_Grammar}
	${Source_Parser}
	${Source_Reflection}
	${Source_Reflection_Enums}
	${Source_Reflection_MemberReference}
	${Source_Reflection_Serialisation}
	${Source_Reflection_TypeInfo}
	${Source_Reflection_TypeInfo_Members}
	${Source_Reflection_TypeInfo_MemberFunctions}
	${Source_Reflection_TypeInfo_Helper}
	${Source_Tokenizer}
	${Source_Tokenizer_Tokens}
	${Source_Tools}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER JitCat)
if (MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_PDB_NAME "$(ProjectName)")
endif()