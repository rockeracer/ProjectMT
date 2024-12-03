#pragma once

#include <map>

template<class KeyType, class ValueType>
class MTMap
{
public:
	using Iterator = typename std::map<KeyType, ValueType>::iterator;
	using Const_Iterator = typename std::map<KeyType, ValueType>::const_iterator;

	void Clear()
	{
		m_Map.clear();
	}

	ValueType* Get(const KeyType& Key)
	{
		Iterator itr = m_Map.find(Key);
		if (itr != m_Map.end())
		{
			return &itr->second;
		}
		else
		{
			return nullptr;
		}
	}

	const ValueType* Get(const KeyType& Key) const
	{
		Const_Iterator itr = m_Map.find(Key);
		if (itr != m_Map.end())
		{
			return &itr->second;
		}
		else
		{
			return nullptr;
		}
	}

	bool Add(const KeyType& Key, ValueType& Value)
	{
		return m_Map.insert(std::pair<KeyType, ValueType>(Key, Value)).second;
	}

	bool Add(const KeyType& Key, const ValueType& Value)
	{
		return m_Map.insert(std::pair<KeyType, ValueType>(Key, Value)).second;
	}

	bool Contain(const KeyType& Key) const
	{
		return m_Map.find(Key) != m_Map.end();
	}

	void Remove(const KeyType& Key)
	{
		auto itr = m_Map.find(Key);
		if (itr != m_Map.end())
		{
			m_Map.erase(itr);
		}
	}

	ValueType& operator[](KeyType& Key)
	{
		return m_Map[Key];
	}

	ValueType& operator[](const KeyType& Key)
	{
		return m_Map[Key];
	}

	Iterator begin()
	{
		return m_Map.begin();
	}

	Const_Iterator begin() const
	{
		return m_Map.begin();
	}

	Iterator end()
	{
		return m_Map.end();
	}

	Const_Iterator end() const
	{
		return m_Map.end();
	}

private:
	std::map<KeyType, ValueType> m_Map;
};
