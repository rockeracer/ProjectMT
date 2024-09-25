#pragma once

#include <vector>

template<class T>
class MTArray
{
public:
	using Iterator = typename std::vector<T>::iterator;
	using Const_Iterator = typename std::vector<T>::const_iterator;

	MTArray()
	{
	}

	MTArray(MTInt32 Count) : m_Array(Count)
	{
	}
	
	MTArray(MTInt32 Count, const T& Value) : m_Array(Count, Value)
	{
	}

	MTArray(std::initializer_list<T> Ilist) : m_Array(Ilist)
	{
	}


	void Add(const T& Value)
	{
		m_Array.push_back(Value);
	}

	MTInt32 Num() const
	{
		return (MTInt32)m_Array.size();
	}

	T& operator[](MTInt32 Index)
	{
		return m_Array[Index];
	}

	const T& operator[](MTInt32 Index) const
	{
		return m_Array[Index];
	}

	void Clear()
	{
		m_Array.clear();
	}

	Iterator Remove(Const_Iterator Itr)
	{
		return m_Array.erase(Itr);
	}

	void Remove(const T& Item)
	{
		auto Itr = std::find(begin(), end(), Item);
		if (Itr != end())
		{
			Remove(Itr);
		}
	}

	void Reserve(MTInt32 Size)
	{
		m_Array.reserve(Size);
	}

	const T* Data() const
	{
		return m_Array.data();
	}

	T* Data()
	{
		return m_Array.data();
	}

	void Resize(MTInt32 Size)
	{
		m_Array.resize(Size);
	}

	bool IsEmpty() const
	{
		return m_Array.empty();
	}

	bool IsValidIndex(MTInt32 Index) const
	{
		return (0 <= Index) && (Index < m_Array.size());
	}

	MTInt32 Find(const T& Item) const
	{
		auto Itr = std::find(begin(), end(), Item);
		if (Itr != end())
		{
			return (MTInt32)std::distance(begin(), Itr);
		}

		return -1;
	}

	bool Contain(const T& Item) const
	{
		return Find(Item) >= 0;
	}

	Iterator begin()
	{
		return m_Array.begin();
	}

	Const_Iterator begin() const
	{
		return m_Array.begin();
	}

	Iterator end()
	{
		return m_Array.end();
	}

	Const_Iterator end() const
	{
		return m_Array.end();
	}

private:
	std::vector<T> m_Array;
};
