#pragma once

class MTVariant
{
public:
	MTVariant();
	~MTVariant();

	enum class Type { None, Int32, Int64, Float, String };

	explicit MTVariant(MTInt32 Value);
	explicit MTVariant(MTInt64 Value);
	explicit MTVariant(float Value);
	explicit MTVariant(const MTString& Value);

	bool IsValid() const;

	bool Get(MTInt32& Value);
	bool Get(MTInt64& Value);
	bool Get(float& Value);
	bool Get(MTString& Value);

private:
	Type m_Type = Type::None;

	union
	{
		MTInt32 m_I32;
		MTInt64 m_I64;
		float m_Float;
		MTString m_String;
	};
};

