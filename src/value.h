/*************************************************************************************************/
/**
	value.h


	Copyright (C) Charles Reilly 2021

	This file is part of BeebAsm.

	BeebAsm is free software: you can redistribute it and/or modify it under the terms of the GNU
	General Public License as published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	BeebAsm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with BeebAsm, as
	COPYING.txt.  If not, see <http://www.gnu.org/licenses/>.
*/
/*************************************************************************************************/

#ifndef VALUE_H_
#define VALUE_H_

#include <cassert>
#include <cstring>

// A simple immutable string buffer with a length and a reference count.
// This doesn't have constructors, etc. so it can be stuffed into a union.
struct StringHeader
{
private:
	unsigned int m_refCount;
	unsigned int m_length;

public:
	static StringHeader* Allocate(const char* text, unsigned int length)
	{
		StringHeader* header = Allocate(length);
		char* buffer = StringBuffer(header);
		memcpy(buffer, text, length);
		buffer[length] = 0;
		return header;
	}

	static const char* StringData(StringHeader* header)
	{
		return StringBuffer(header);
	}

	static void AddRef(StringHeader* header)
	{
		header->m_refCount++;
	}

	static void Release(StringHeader** ppheader)
	{
		StringHeader*& header = *ppheader;
		header->m_refCount--;
		if (header->m_refCount == 0)
		{
			free(header);
		}
		header = 0;
	}

	static unsigned int Length(StringHeader* header)
	{
		return header->m_length;
	}

	static int Compare(StringHeader* header1, StringHeader* header2)
	{
		int result = memcmp(StringData(header1), StringData(header2), std::min(header1->m_length, header2->m_length));
		if (result != 0)
			return result;
		return Compare(header1->m_length, header2->m_length);
	}

	static StringHeader* Concat(StringHeader* header1, StringHeader* header2)
	{
		int length = header1->m_length + header2->m_length;
		StringHeader* header = Allocate(length);
		char* buffer = StringBuffer(header);
		memcpy(buffer, StringData(header1), header1->m_length);
		memcpy(buffer + header1->m_length, StringData(header2), header2->m_length);
		buffer[length] = 0;
		return header;
	}

	static StringHeader* SubString(StringHeader* source, unsigned int index, unsigned int length)
	{
		assert(index <= Length(source));
		unsigned int realLength = std::min(length, Length(source) - index);
		if ((index == 0) && (realLength == Length(source)))
		{
			return source;
		}
		return Allocate(StringData(source) + index, realLength);
	}

	static StringHeader* Repeat(StringHeader* source, unsigned int count)
	{
		int sourceLength = Length(source);
		const char* sourceData = StringData(source);

		assert((sourceLength < 0x10000) && (count < 0x10000));
		unsigned int length = sourceLength * count;
		assert(length < 0x10000);

		StringHeader* header = Allocate(length);
		char* buffer = StringBuffer(header);
		for (unsigned int i = 0; i != count; ++i)
		{
			memcpy(buffer, sourceData, sourceLength);
			buffer += sourceLength;
		}
		*buffer = 0;
		return header;
	}

private:
	static char* StringBuffer(StringHeader* header)
	{
		return reinterpret_cast<char*>(header) + sizeof(StringHeader);
	}

	static StringHeader* Allocate(unsigned int length)
	{
		int fullLength = sizeof(StringHeader) + length + 1;
		StringHeader* header = static_cast<StringHeader*>(malloc(fullLength));
		if (!header)
			return 0;
		header->m_refCount = 0;
		header->m_length = length;
		return header;
	}

	static int Compare(unsigned int a, unsigned int b)
	{
		if (a == b)
			return 0;
		else if (a < b)
			return -1;
		else
			return 1;
	}
};

// A simple immutable string.
class String
{
public:
	~String()
	{
		StringHeader::Release(&m_header);
	}
	String(const String& that)
	{
		m_header = that.m_header;
		StringHeader::AddRef(m_header);
	}
	String(const char* text, unsigned int length)
	{
		m_header = StringHeader::Allocate(text, length);
		StringHeader::AddRef(m_header);
	}
	String& operator=(const String& that)
	{
		if (m_header != that.m_header)
		{
			StringHeader::Release(&m_header);
			m_header = that.m_header;
			StringHeader::AddRef(m_header);
		}
		return *this;
	}
	String operator+(const String& that)
	{
		StringHeader* header = StringHeader::Concat(m_header, that.m_header);
		return String(header);
	}
	String SubString(unsigned int index, unsigned int length)
	{
		StringHeader* header = StringHeader::SubString(m_header, index, length);
		return String(header);
	}
	String Repeat(unsigned int count)
	{
		StringHeader* header = StringHeader::Repeat(m_header, count);
		return String(header);
	}
	unsigned int Length() const
	{
		return StringHeader::Length(m_header);
	}
	const char* Text() const
	{
		return StringHeader::StringData(m_header);
	}
	char operator[](int index) const
	{
		assert((0 <= index) && (static_cast<unsigned int>(index) < Length()));
		return Text()[index];
	}
private:
	friend class Value;
	StringHeader* m_header;

	String(StringHeader* header)
	{
		m_header = header;
		StringHeader::AddRef(m_header);
	}
};

// A value that can be a string or a number
class Value
{
public:
	enum Type
	{
		NumberValue,
		StringValue
	};

	Value()
	{
		m_type = NumberValue;
		m_number = 0;
	}

	Value(double number)
	{
		m_type = NumberValue;
		m_number = number;
	}

	Value(String s)
	{
		m_type = StringValue;
		m_string = s.m_header;
		StringHeader::AddRef(m_string);
	}

	Value(const Value& that)
	{
		Assign(that);
	}

	Value& operator=(const Value& that)
	{
		if ((m_type != StringValue) || (that.m_type != StringValue) || (m_string != that.m_string))
		{
			Clear();
			Assign(that);
		}
		return *this;
	}

	~Value()
	{
		Clear();
	}

	Type GetType() const
	{
		return m_type;
	}

	double GetNumber() const
	{
		assert(m_type == NumberValue);
		return m_number;
	}

	String GetString() const
	{
		assert(m_type == StringValue);
		return String(m_string);
	}

	static int Compare(Value value1, Value value2)
	{
		if (value1.GetType() == value2.GetType())
		{
			if (value1.GetType() == NumberValue)
			{
				return Compare(value1.GetNumber(), value2.GetNumber());
			}
			else if (value1.GetType() == StringValue)
			{
				return StringHeader::Compare(value1.m_string, value2.m_string);
			}
			else
			{
				assert(false);
			}
			return 0;
		}
		if (value1.GetType() < value2.GetType())
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}

private:
	union
	{
		double m_number;
		StringHeader* m_string;
	};

	Type m_type;

	void Clear()
	{
		if ((m_type == StringValue) && m_string)
		{
			StringHeader::Release(&m_string);
		}
	}

	void Assign(const Value& that)
	{
		m_type = that.m_type;
		if (m_type == NumberValue)
		{
			m_number = that.m_number;
		}
		else if (m_type == StringValue)
		{
			m_string = that.m_string;
			StringHeader::AddRef(m_string);
		}
		else
		{
			assert(false);
		}
	}

	static int Compare(double a, double b)
	{
		if (a == b)
			return 0;
		else if (a < b)
			return -1;
		else
			return 1;
	}
};

#endif // VALUE_H_
