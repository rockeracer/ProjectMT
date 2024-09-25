#pragma once

#include "Class.h"
#include "Property.h"
#include "ObjectSystem.h"

template<class T>
class MTClassRegister
{
public:
	MTClassRegister(const MTName& ClassName)
    {
		MTClass* NewClass = new MTClass;
		NewClass->CreateObject = [](){ return new T; };
		T::RegisterProperty(NewClass);
        
		if (MTObjectSystem::Get().AddClass(ClassName, NewClass) == false)
        {
            MT_ASSERT(0);
            delete NewClass;
        }
    }
};
