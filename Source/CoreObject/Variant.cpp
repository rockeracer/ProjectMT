#include "Variant.h"

MTVariant::MTVariant(MTInt32 Value)
{
	m_I32 = Value;
	m_Type = Type::Int32;
}

MTVariant::MTVariant(MTInt64 Value)
{
	m_I64 = Value;
	m_Type = Type::Int64;
}

MTVariant::MTVariant(float Value)
{
	m_Float = Value;
	m_Type = Type::Float;
}

MTVariant::MTVariant(const MTString& Value)
{
	m_String = Value;
	m_Type = Type::String;
}

MTVariant::~MTVariant()
{

}

bool MTVariant::IsValid() const
{
	return m_Type != Type::None;
}

bool MTVariant::Get(MTInt32& Value)
{
	if (m_Type == Type::Int32)
	{
		Value = m_I32;
		return true;
	}

	return false;
}

bool MTVariant::Get(MTInt64& Value)
{
	if (m_Type == Type::Int64)
	{
		Value = m_I64;
		return true;
	}

	return false;
}

bool MTVariant::Get(float& Value)
{
	if (m_Type == Type::Float)
	{
		Value = m_Float;
		return true;
	}

	return false;
}

bool MTVariant::Get(MTString& Value)
{
	if (m_Type == Type::String)
	{
		Value = m_String;
		return true;
	}

	return false;
}
