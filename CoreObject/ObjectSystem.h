#pragma once

class MTClass;

class COREOBJECT_API MTObjectSystem
{
public:
    static void Destroy();
    static MTObjectSystem& Get();
    
    bool HasClass(const MTName& ClassName) const;
    
    bool AddClass(const MTName& ClassName, MTClass* Class);
    
    MTClass* GetClass(const MTName& ClassName) const;

private:
    void ClearClass();
    
    MTMap<MTName, MTClass*> m_Classes;
};
