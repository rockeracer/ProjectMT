#pragma once

#include <sstream>
#include <fstream>

class MTFile
{
public:
    enum
    {
        Read = 1 << 0,
        Write = 1 << 1,
        Binary = 1 << 2,
        StartAtEnd = 1 << 3,
        Append = 1 << 4,
        Discard = 1 << 5,
    };
    typedef MTInt32 OpenMode;
    
	CORE_API MTFile() {}
    
	CORE_API explicit MTFile( const MTString& FilePath, OpenMode Mode )
    {
        Open(FilePath, Mode);
    }
    
	CORE_API ~MTFile()
    {
        Close();
    }
    
    void Open( const MTString& FilePath, OpenMode Mode )
    {
        if (FilePath.IsEmpty() == false)
        {
            m_File.open(FilePath.c_str(), ToSTLMode(Mode));
        }
    }
    
    void Close()
    {
        if (m_File.is_open())
        {
            m_File.close();
        }
    }
    
    MTString ToString()
    {
        if (m_File.is_open())
        {
            std::stringstream strStream;
            strStream << m_File.rdbuf();
            return strStream.str();
        }
        else
        {
            return "";
        }
    }
    
    MTFile& operator<<( const MTString& str )
    {
        m_File.write( str.c_str(), str.Length() );
        return *this;
    }

private:
    std::ios_base::openmode ToSTLMode( OpenMode Mode )
    {
        std::ios_base::openmode result = 0;
        if ( Mode & Read )			result |= std::ios_base::in;
        if ( Mode & Write )			result |= std::ios_base::out;
        if ( Mode & Binary )		result |= std::ios_base::binary;
        if ( Mode & StartAtEnd )	result |= std::ios_base::ate;
        if ( Mode & Append )		result |= std::ios_base::app;
        if ( Mode & Discard )		result |= std::ios_base::trunc;
        
        return result;
    }

	std::fstream m_File;
};
