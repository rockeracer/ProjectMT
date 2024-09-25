#pragma once

#include <string>

class MTString
{
public:
	MTString()
	{
	}

	MTString(const std::string& Str)
	{
		m_String = Str;
	}

	MTString(const char* Str)
	{
		m_String = Str;
	}

	bool IsEmpty() const
	{
		return m_String.empty();
	}

	const char* c_str() const
	{
		return m_String.c_str();
	}

	void Replace(const MTString& Old, const MTString& New)
	{
		std::size_t Found = m_String.find(Old.m_String);
		if (Found != std::string::npos)
		{
			m_String.replace(Found, Old.Length(), New.m_String);
		}
	}

	MTInt32 Length() const
	{
		return (MTInt32)m_String.size();
	}

	void Clear()
	{
		m_String.clear();
	}

	template<class T>
	static MTString ToString(T Value)
	{
		return std::to_string(Value);
	}

	MTString operator+(const char* Str)
	{
		return m_String + Str;
	}

	MTString operator+(const MTString& Str)
	{
		return m_String + Str.m_String;
	}

	MTString operator+(const std::string& Str)
	{
		return m_String + Str;
	}

	MTString& operator+=(const char* Str)
	{
		m_String += Str;
		return (*this);
	}

	MTString& operator+=(const MTString& Str)
	{
		m_String += Str.m_String;
		return (*this);
	}

	MTString& operator+=(const std::string& Str)
	{
		m_String += Str;
		return (*this);
	}

	friend MTString operator+(const MTString& Lhs, const MTString& Rhs)
	{
		return Lhs.m_String + Rhs.m_String;
	}

	bool operator<(const MTString& Str) const
	{
		return m_String < Str.m_String;
	}

	bool operator==(const MTString& Str) const
	{
		return m_String == Str.m_String;
	}

private:
	std::string m_String;
};
