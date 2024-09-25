#pragma once

#include "TestClass.gen.h"

MTCLASS()
class TestClass : public MTObject
{
public:
	GENERATED_BODY(TestClass)

	MTPROPERTY()
    MTInt32 Int32;

	MTPROPERTY()
    MTInt64 Int64;
	
	MTPROPERTY()
    float Float;

	MTPROPERTY()
    MTString String;
};
