#pragma once

static auto ClassRegister_TestClass = MTClassRegister<class TestClass>("TestClass");

#define REGISTER_CLASS_TestClass()\
	static void RegisterProperty(MTClass* Class)\
	{\
		Class->AddProperty("MTInt32", "Int32", &TestClass::Int32);\
		Class->AddProperty("MTInt64", "Int64", &TestClass::Int64);\
		Class->AddProperty("float", "Float", &TestClass::Float);\
		Class->AddProperty("MTString", "String", &TestClass::String);\
	}
