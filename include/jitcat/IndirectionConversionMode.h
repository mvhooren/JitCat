/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once
namespace jitcat
{
    enum class IndirectionConversionMode
    {
        None,
        DereferencePointer,
        DereferencePointerToPointer,
        DereferencePointerToPointerTwice,
        AddressOfValue,
        AddressOfPointer,
        ErrorNotCopyConstructible,
        ErrorTooMuchIndirection,
        ErrorTypeMismatch
    };

    inline bool isValidConversionMode(IndirectionConversionMode mode)
    {
        switch (mode)
        {
            case IndirectionConversionMode::None:
            case IndirectionConversionMode::DereferencePointer:
            case IndirectionConversionMode::DereferencePointerToPointer:
            case IndirectionConversionMode::DereferencePointerToPointerTwice:
            case IndirectionConversionMode::AddressOfValue:
            case IndirectionConversionMode::AddressOfPointer:
                return true;
            default:
                return false;
        }
    }

    inline bool isDereferenceConversionMode(IndirectionConversionMode mode)
    {
        switch (mode)
        {
            case IndirectionConversionMode::DereferencePointer:
            case IndirectionConversionMode::DereferencePointerToPointer:
            case IndirectionConversionMode::DereferencePointerToPointerTwice:
                return true;
            default:
                return false;
        }
    }

    inline bool isReferenceConversionMode(IndirectionConversionMode mode)
    {
        switch (mode)
        {
            case IndirectionConversionMode::AddressOfValue:
            case IndirectionConversionMode::AddressOfPointer:
                return true;
            default:
                return false;
        }
    }
}