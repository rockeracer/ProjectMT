#include "gtest/gtest.h"
#include "Core.h"
#include "RapidJson.h"

class MTArchive
{
public:
    virtual void operator<<(MTInt32 i32)
    {
    }
    
    virtual void operator<<(const MTString& Str)
    {
    }
};

class MTJsonArchive : public MTArchive
{
public:
    MTJsonArchive() : m_StringWriter(m_StringBuffer)
    {}
    
    void StartObject()
    {
        m_StringWriter.StartObject();
    }
    
    void EndObject()
    {
        m_StringWriter.EndObject();
    }
    
    void StartArray()
    {
        m_StringWriter.StartArray();
    }
    
    void EndArray()
    {
        m_StringWriter.EndArray();
    }
    
    virtual void operator<<(MTInt32 i32) override
    {
        m_StringWriter.Int(i32);
    }
    
    virtual void operator<<(const MTString& Str) override
    {
        m_StringWriter.String(Str.c_str());
    }

    void EndStream()
    {
        rapidjson::StringStream StringStream(m_StringBuffer.GetString());
        m_Document.ParseStream(StringStream);

//        const char* t = "{\"a\" : [1, 2, \"tsdfgsgfd\"]}";
//        rapidjson::StringStream TestStringStream(t);
//        m_Document.ParseStream(TestStringStream);
//        bool b0 = m_Document["a"][0].IsInt();
//        bool b2 = m_Document["a"][2].IsString();
//        std::cout << m_Document["a"][2].GetString() << std::endl;
//        
//        rapidjson::Value& v = (m_Document["a"][0]);
    }
    
    void SaveToFile()
    {
//        FILE* fp = fopen("output.json", "w");
//        rapidjson::FileWriteStream os(fp, (char*)m_StringBuffer.GetString(), m_StringBuffer.GetSize());
//        rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
//        m_Document.Accept(writer);
//        fclose(fp);
    }
    
private:
    rapidjson::Document m_Document;
    
    rapidjson::StringBuffer m_StringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> m_StringWriter;
};

TEST(JsonTest, RapidJsonTest)
{
    MTJsonArchive Archive;
    Archive.StartArray();
    Archive.StartObject();
    Archive << "Size";
    Archive << 123;
    Archive.EndObject();
    Archive.EndArray();
    Archive.EndStream();
    Archive.SaveToFile();
}
