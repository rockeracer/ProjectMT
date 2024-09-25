#include "gtest/gtest.h"
#include "Core.h"

class FooTest : public ::testing::Test {
};

int main(int argc, char * argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    
    InitCore();
	Core::LoadModule("CoreObject");
    
    int Exit = RUN_ALL_TESTS();
    
    ShutdownCore();
    
    return Exit;
}
