#include "gtest/gtest.h"
#include "Core.h"
#include "CoreObject.h"
#include "TestClass.h"

TEST(ClassRegisterTest, RegisterClass)
{
    MTClass* Class = MTObjectSystem::Get().GetClass("TestClass");

    EXPECT_NE(Class, nullptr);
}

TEST(ClassRegisterTest, ClassNewObject)
{
    MTClass* Class = MTObjectSystem::Get().GetClass("TestClass");
    MTObject* NewObject = Class->NewObject();
    TestClass* MyObject = dynamic_cast<TestClass*>(NewObject);
    
    EXPECT_NE(NewObject, nullptr);
    EXPECT_NE(MyObject, nullptr);
    
    delete MyObject;
}

TEST(ClassRegisterTest, RegisterProperty)
{
    MTClass* Class = MTObjectSystem::Get().GetClass("TestClass");
    
    EXPECT_NE(Class->GetProperty("Int32"), nullptr);
    EXPECT_NE(Class->GetProperty("Int64"), nullptr);
    EXPECT_NE(Class->GetProperty("Float"), nullptr);
    EXPECT_NE(Class->GetProperty("String"), nullptr);
}



class MTVariable_Parse
{
public:
	MTString m_Name;
	MTString m_Type;
	bool bConst = false;
	bool bPointer = false;
	bool bReference = false;

	virtual void GetValue(void* obj, void* v) = 0;
	virtual void SetValue(void* obj, void* v) = 0;
};

template<typename T, typename Class>
class MTVariableTemplate : public MTVariable_Parse
{
public:
	MTVariableTemplate(T Class::* Property) : m_Property(Property)
	{}

	virtual void GetValue(void* obj, void* v) override
	{
		Class* MyObj = (Class*)obj;
		T* MyValue = (T*)v;
		*MyValue = MyObj->*m_Property;
	}

	virtual void SetValue(void* obj, void* v) override
	{
		Class* MyObj = (Class*)obj;
		T* MyValue = (T*)v;
		MyObj->*m_Property = *MyValue;
	}

private:
	T Class::* m_Property;
};


class MTClass_Parse
{
public:
	MTClass_Parse() {}

	virtual class MTObject_Parse* CreateObj() = 0;

	MTMap<MTName, MTVariable_Parse*> m_Properties;
};

template<class T>
class MTClassImp : public MTClass_Parse
{
public:

	virtual class MTObject_Parse* CreateObj() override
	{
		T* obj = new T;
		obj->RegisterProperty(this);

		return (MTObject_Parse*)(obj);
	}
};


static MTArray<MTClass_Parse*> G_Class;

class MTObject_Parse
{
public:
	//virtual void RegisterProperty(MTClass_Parse* Class) = 0;
	virtual void Print() = 0;
};




//Begin Generated Code
template<class T>
class MTRegisterClass
{
public:
	MTRegisterClass()
	{
		MTClass_Parse* NewClass = new MTClassImp<T>();

// 		NewClass->m_Properties.Add("Int32", new MTVariableTemplate<MTInt32, T>(&T::Int32));
// 		NewClass->m_Properties.Add("Int64", new MTVariableTemplate<MTInt64, T>(&T::Int64));
// 		NewClass->m_Properties.Add("Float", new MTVariableTemplate<float, T>(&T::Float));
// 		NewClass->m_Properties.Add("String", new MTVariableTemplate<MTString, T>(&T::String));
		T::RegisterProperty(NewClass);

		G_Class.Add(NewClass);
	}
};

//class TestClass_Parse;
static auto t = MTRegisterClass<class TestClass_Parse>();
//End Generated Code


#define REGISTER_CLASS_TestClass_Parse()\
	static void RegisterProperty(MTClass_Parse* Class)\
	{\
		Class->m_Properties.Add("Int32", new MTVariableTemplate<MTInt32, TestClass_Parse>(&TestClass_Parse::Int32));\
		Class->m_Properties.Add("Int64", new MTVariableTemplate<MTInt64, TestClass_Parse>(&TestClass_Parse::Int64));\
		Class->m_Properties.Add("Float", new MTVariableTemplate<float, TestClass_Parse>(&TestClass_Parse::Float));\
		Class->m_Properties.Add("String", new MTVariableTemplate<MTString, TestClass_Parse>(&TestClass_Parse::String));\
	}


class TestClass_Parse : public MTObject_Parse
{
	//friend class TestClass_Parse_Ref;
public:
	TestClass_Parse()
	{
	}
//Begin Generated Code
	//friend MTRegisterClass<TestClass_Parse>;
	REGISTER_CLASS_TestClass_Parse()
// 	static void RegisterProperty(MTClass_Parse* Class)
// 	{
// 		Class->m_Properties.Add("Int32", new MTVariableTemplate<MTInt32, TestClass_Parse>(&TestClass_Parse::Int32));
// 		Class->m_Properties.Add("Int64", new MTVariableTemplate<MTInt64, TestClass_Parse>(&TestClass_Parse::Int64));
// 		Class->m_Properties.Add("Float", new MTVariableTemplate<float, TestClass_Parse>(&TestClass_Parse::Float));
// 		Class->m_Properties.Add("String", new MTVariableTemplate<MTString, TestClass_Parse>(&TestClass_Parse::String));
// 	}
//End Generated Code

	virtual void Print() override
	{
		std::cout << "I'm TestClass_Parse." << std::endl;
	}
private:
	MTInt32 Int32 = 32;
	
	MTInt64 Int64 = 64;
	
	float Float;
	
	MTString String;
};



// class TestClass_Parse_Ref : public MTClass_Parse
// {
// public:
// 	TestClass_Parse_Ref()
// 	{
// 		// 		MTObject_Parse* DefaultObj = new TestClass_Parse();
// 		// 		DefaultObj->RegisterProperty(this);
// 		MTInt32 TestClass_Parse::* Property = &TestClass_Parse::Int32;
// 		std::cout << "#!$@%@^$#%&%^&$%^*#^$@^$%^&$#%&%$^&#^#$%&^%&$%&$%&$%^&#%&^$%^&$%&$%^&#@%@&$*^&($%^" << std::endl;
// 	}
// };
// static TestClass_Parse_Ref TestClass_Parse_Ref_Instance;



TEST(ClassRegisterTest, TestHeaderParser)
{
	TestClass_Parse TestObj;

	for (auto Class : G_Class)
	{
		MTInt32 int32 = 0;
		MTInt64 int64 = 0;
		MTString str;
		Class->m_Properties["Int32"]->GetValue(&TestObj, &int32);
		Class->m_Properties["Int64"]->GetValue(&TestObj, &int64);
		Class->m_Properties["String"]->GetValue(&TestObj, &str);

		std::cout << int32 << "   " << int64 << "   " << std::endl;

		int32 = 3232;
		int64 = 6464;
		str = "WriteString";
		Class->m_Properties["Int32"]->SetValue(&TestObj, &int32);
		Class->m_Properties["Int64"]->SetValue(&TestObj, &int64);
		Class->m_Properties["String"]->SetValue(&TestObj, &str);

		std::cout << int32 << "   " << int64 << "   " << std::endl;

		//MTObject_Parse* obj = Class->CreateObj();
		//MTObject_Parse* obj = new TestClass_Parse;
		//obj->Print();
		//delete obj;
	}
}