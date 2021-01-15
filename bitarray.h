/*
With this project I found wrapping my head around the bitproxy to be the most difficult aspect, however, after testing and trying new
things I was eventually able to get it all working. Lab 6 was useful in helping me think about the comparator methods and their implementation.
*/


#ifndef _BITARRAY_H
#define _BITARRAY_H

#include <iostream>
#include <vector>
#include <string>
#include <climits>
#include <cassert>
#include <sstream>
#include <cstddef>
#include <algorithm>
#include <utility>
#include <stdexcept>
using std::runtime_error;
using std::logic_error;
using std::move;
using std::min;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::ostream;
using std::istream;

template<class IType = size_t>
class BitArray
{
public:
	// Object Management
	explicit BitArray(size_t bitCount = 0)
	{
		numBits = bitCount;
		size_t numBlocks = numBits / BITS_PER_WORD;
		if (numBits % BITS_PER_WORD != 0)
			numBlocks++;
		myBits.resize(numBlocks);
	}

	explicit BitArray(const string& bitStr) // string constructor
	{
		numBits = 0;
		for (auto x : bitStr)
		{
			if (x == '0')
				*this += false;
			else
				*this += true;
		}
	} 

	BitArray(const BitArray& b) = default; // Copy constructor
	BitArray& operator=(const BitArray& b) = default; // Copy assignment
	BitArray(BitArray&& b) noexcept // Move constructor
	{
		cout << "move constructor" << endl;
		myBits.swap(b.myBits);
		numBits = b.numBits;
		b.numBits = 0;
	}
	BitArray& operator=(BitArray&& b) noexcept // Move assignment
	{
		cout << "move assignment" << endl;
		if (&b != this)
		{
			myBits.swap(b.myBits);
			size_t tmp = numBits;
			numBits = b.numBits;
			b.numBits = tmp;
		}
		return *this;

	}
	size_t capacity() const // # of bits the current allocation can hold
	{
		return myBits.size() * BITS_PER_WORD;
	}
		
	
   // Mutators
	BitArray& operator+=(bool value) // Append a bit
	{
		if (numBits % BITS_PER_WORD == 0)
			myBits.push_back(0);
		assign_bit(numBits++, value);
		return *this;
	}
	BitArray& operator+=(const BitArray& b) // Append a BitArray
	{
		size_t count = b.size();
		for (size_t i = 0; i < count; i++)
		{
			*this += b.read_bit(i);
		}
		return *this;
	}
	void erase(size_t pos, size_t nbits = 1) // Remove “nbits” bits at a position
	{
		while (nbits != 0)
		{
			size_t source = pos + 1;
			size_t dest = pos;
			while (source < numBits)
				assign_bit(dest++, read_bit(source++));
			numBits--;
			nbits--;
		}
	}
	void insert(size_t pos, bool val) // Insert a bit at a position (slide "right")
	{
		if (numBits++ % BITS_PER_WORD == 0)
			myBits.push_back(0);

		size_t dest = numBits - 1;
		size_t source = dest - 1;

		while (source >= pos)
			assign_bit(dest--, read_bit(source--));
		assign_bit(pos, val);
	}
	void insert(size_t pos, const BitArray& bArray) // Insert an entire BitArray object
	{
		for (size_t i = 0; i < bArray.size(); i++)
		{
			insert(pos, bArray.read_bit(i));
		}
	}

   // Bitwise ops
	friend class bitproxy;
	class bitproxy
	{
	public:
		BitArray& bArray;
		size_t index;
		bitproxy(BitArray& bArray, size_t index) : bArray(bArray), index(index) {};
		operator bool()
		{
			return bArray.read_bit(index);
		}
		bitproxy& operator= (bool val) 
		{
			bArray.assign_bit(index, val);
			return *this;
		}
		bitproxy& operator= (const bitproxy& compArray)
		{
			bArray.assign_bit(index, compArray.bArray.read_bit(compArray.index));
			return *this;
		}
	};
	bitproxy operator[](size_t index)
	{
		if (index >= numBits)
			throw logic_error("Index is out of bounds");
		return bitproxy(*this, index);
	}
	bool operator[](size_t bitPos) const
	{
		return read_bit(bitPos);
	}

	void toggle(size_t bitPos) // Toggles a Single bit
	{
		assign_bit(bitPos, !read_bit(bitPos));
	}
	void toggle() // Toggles all bits
	{
		for (size_t i = 0; i < numBits; i++)
		{
			toggle(i);
		}
	}

	BitArray operator~() const
	{
		BitArray tmp(*this);
		tmp.toggle();
		return tmp;
	}
	
	// Shift operators…

	BitArray operator<<(unsigned int numShift) const
	{
		BitArray tmp(*this);
		size_t dest = 0;
		size_t source = numShift;
		while (source < tmp.numBits)
			tmp.assign_bit(dest++, tmp.read_bit(source++));
		while (dest < tmp.numBits)
			tmp.assign_bit(dest++, false);
		return tmp;
	}
	BitArray operator>>(unsigned int numShift) const
	{
		BitArray tmp(*this);
		size_t dest = tmp.numBits;
		size_t source = dest - numShift;
		while (source > 0)
			tmp.assign_bit(dest-- - 1, tmp.read_bit(source-- - 1));
		while (dest > 0)
			tmp.assign_bit(dest-- - 1, false);
		return tmp;
	}
	BitArray& operator<<=(unsigned int numShift)
	{
		size_t dest = 0;
		size_t source = numShift;
		while (source < numBits)
			assign_bit(dest++, read_bit(source++));
		while (dest < numBits)
			assign_bit(dest++, false);
		return *this;
	}
	BitArray& operator>>=(unsigned int numShift)
	{
		size_t dest = numBits;
		size_t source = dest - numShift;
		while (source > 0)
			assign_bit(dest-- -1, read_bit(source-- - 1));
		while (dest > 0)
			assign_bit(dest-- - 1, false);
		return *this;
	}

	 // Extraction ops
	BitArray slice(size_t pos, size_t count) const // Extracts a new sub-array
	{
		BitArray tmp;
		for (size_t i = 0; i < count; i++)
		{
			tmp += read_bit(pos + i);
		}
		return tmp;
	}

	// Comparison ops
	bool operator==(const BitArray& compArray) const
	{
		if (numBits != compArray.numBits)
			return false;
		for (size_t i = 0; i < numBits; i++)
		{
			if (read_bit(i) != compArray.read_bit(i))
				return false;
		}
		return true;
	}
	bool operator!=(const BitArray& compArray) const
	{
		if (numBits != compArray.numBits)
		{
			return true;
		}
		for (size_t i = 0; i < numBits; i++)
		{
			if (read_bit(i) != compArray.read_bit(i))
				return true;
		}
		return false;
	}
	bool operator<(const BitArray& compArray) const
	{
		for (size_t i = 0; i < min(numBits, compArray.numBits); i++)
		{
			if (read_bit(i) < compArray.read_bit(i))
				return true;
			if (read_bit(i) > compArray.read_bit(i))
				return false;
		}
		return numBits < compArray.numBits;
	}
	bool operator<=(const BitArray& compArray) const
	{
		for (size_t i = 0; i < min(numBits, compArray.numBits); i++)
		{
			if (read_bit(i) < compArray.read_bit(i))
				return true;
			if (read_bit(i) > compArray.read_bit(i))
				return false;
		}
		return numBits <= compArray.numBits;
	}
	bool operator>(const BitArray& compArray) const
	{
		for (size_t i = 0; i < min(numBits, compArray.numBits); i++)
		{
			if (read_bit(i) < compArray.read_bit(i))
				return false;
			if (read_bit(i) > compArray.read_bit(i))
				return true;
		}
		return numBits > compArray.numBits;
	}
	bool operator>=(const BitArray& compArray) const
	{
		for (size_t i = 0; i < min(numBits, compArray.numBits); i++)
		{
			if (read_bit(i) < compArray.read_bit(i))
				return false;
			if (read_bit(i) > compArray.read_bit(i))
				return true;
		}
		return numBits >= compArray.numBits;
	}
	

	// Counting ops
	size_t size() const // Number of bits in use in the vector
	{
		return numBits;
	}; 
	size_t count() const // The number of 1-bits present
	{
		size_t count = 0;
		for (size_t i = 0; i < numBits; i++)
		{
			if (read_bit(i))
				++count;
		}
		return count;
	}
	bool any() const // Optimized version of count() > 0 
	{
		for (size_t i = 0; i < numBits; i++)
		{
			if (read_bit(i))
			{
				return true;
			}
		}
		return false;
	}
	
	// Stream I/O (define these in situ—meaning the bodies are inside the class)
	friend ostream& operator<<(ostream& os, const BitArray& bits)
	{
		for (size_t i = 0; i < bits.size(); i++)
		{
			if (bits.read_bit(i) == true)
				os << '1';
			else
				os << '0';
		}
		return os;
	}
	friend istream& operator>>(istream& is, BitArray& bits)
	{
		char ch = is.get();
		if (ch != '0' and ch != '1')
		{
			is.setstate(std::ios::failbit);
			return is;
		}
		bits.erase(0, bits.numBits);
		is.unget();
		while (is >> ch)
		{
			if (ch == '0')
			{
				bits += false;
			}
			else if (ch == '1')
			{
				bits += true;
			}
			else
			{
				is.unget();
					break;
			}
		}
		return is;
	}

	// String conversion
	string to_string() const
	{
		string tmp;
		for (size_t i = 0; i < numBits; i++)
		{
			if (read_bit(i))
				tmp += '1';
			else
				tmp += '0';
		}
		return tmp;
	}

private: 
	enum { BITS_PER_WORD = CHAR_BIT * sizeof(IType) };
	vector<IType> myBits;
	size_t numBits;

	void assign_bit(size_t bitpos, bool val)
	{
		if (bitpos >= numBits)
			throw logic_error("assign_bit position out of range");
		size_t block = bitpos / BITS_PER_WORD;
		size_t offset = bitpos % BITS_PER_WORD;
	    size_t mask = 1u << offset;
		if (val == true)
			myBits[block] |= mask;
		else
			myBits[block] &= ~mask;
	}
	bool read_bit(size_t bitpos) const
	{
		if (bitpos >= numBits)
			throw logic_error("read_bit position out of range");
		size_t block = bitpos / BITS_PER_WORD;
		size_t offset = bitpos % BITS_PER_WORD;
		size_t mask = 1u << offset;
		return !!(myBits[block] & mask);
	}
};


#endif;