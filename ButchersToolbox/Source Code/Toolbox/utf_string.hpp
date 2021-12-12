#pragma once

// Standard
#include <string>
#include <vector>


// Forward
class utf8string;
class utf8string_iter;


// NOTE:
// Upon analizing the memory cost of rendering a single character on the GPU
// The idea of using UTF-8 to save memory of rendered characters is absolutely
// hilarious and absurd (pretty easy to make a utf-8 string type)
// it takes around 120 bytes of memory of CPU and GPU to render a single character


/* Low level Utilities */

// looks at the byte and determines how many bytes are used for the character
// in the encoding
// returns 0 if invalid
uint32_t getUTF8_SequenceLength(uint8_t first_byte);

// get how many bytes are required to encode the code point
// returns 0 if invalid
uint32_t getUTF8_CodePointLength(uint32_t code_point);

// encode code point into bytes
// ensure you call the right one based on code point
void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0);

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1);

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2);

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2, uint8_t& byte_3);

// get the number of bytes between iterators
uint32_t byteCount(utf8string_iter& begin, utf8string_iter& end);


class utf8string_iter {
public:
	utf8string* parent = nullptr;
	int32_t byte_index;

public:
	// Iteration
	void prev(uint32_t count = 1);
	void next(uint32_t count = 1);

	// calculate at which caracter does this point to
	int32_t characterIndex();

	// retrieve/decode unicode code point
	uint32_t codePoint();

	// get character as utf8string
	utf8string get();

	// get pointer to bytes in utf8string parent
	uint8_t* data();

	bool isBeforeBegin();
	bool isAtNull();

	// swap the contents of this iterator with other
	void swap(utf8string_iter& other);
};

bool operator==(utf8string_iter a, utf8string_iter b);
bool operator!=(utf8string_iter a, utf8string_iter b);
bool operator<(utf8string_iter a, utf8string_iter b);
bool operator>(utf8string_iter a, utf8string_iter b);
bool operator<=(utf8string_iter a, utf8string_iter b);
bool operator>=(utf8string_iter a, utf8string_iter b);

bool operator==(utf8string_iter a, const char8_t* utf8_string_literal);


class utf8string {
public:
	std::vector<uint8_t> bytes;

public:
	utf8string();
	utf8string(const char8_t* utf8_string_literal);
	utf8string(std::string& string);
	utf8string(std::vector<uint8_t>& bytes);

	/* Queries */

	// return the number of code points (excludes \\0)
	uint32_t length();

	// faster than calling length and checking for 0
	bool isEmpty();


	/* Iterators */

	// return iterator with byte index of -1 before begin iter
	utf8string_iter before();

	// return iterator with byte index of 0
	utf8string_iter begin();

	// return iterator to last character
	utf8string_iter last();
	
	// return iterator with byte index of the last character which is \\0
	utf8string_iter after();


	/* Insert */

	void push(uint32_t code_point);

	void insertAfter(utf8string_iter& location, utf8string& new_content);

	void insertAfter(utf8string_iter& location,
		utf8string_iter& new_content_start, utf8string_iter& new_content_end);

	// overwrites characters from new_content to location
	void overwrite(utf8string_iter& location, utf8string& new_content);

	void overwrite(utf8string_iter& location,
		utf8string_iter& new_content_start, uint32_t new_content_length);

	void overwrite(utf8string_iter& location,
		utf8string_iter& new_content_start, utf8string_iter& new_content_end);

	// designed for use in text edit
	// deletes selection and inserts new content
	// method is faster than delete + insert
	void replaceSelection(utf8string_iter& selection_start, uint32_t selection_length,
		utf8string& new_content);


	/* Delete */

	// delete a number of characters from the end
	// void pop();
	
	// erase starting at location
	void erase(utf8string_iter& selection_start, uint32_t selection_length);

	void erase(utf8string_iter& selection_start, utf8string_iter& selection_end);


	/* Output */

	// extract selection into a new utf8string
	utf8string extract(utf8string_iter& begin, utf8string_iter& end);
	utf8string extract(utf8string_iter& begin, uint32_t count);

	// fill other string with bytes content (excludes \0)
	void fill(std::string& other);

	// returns a C style null terminated string
	const char* c_str();
};

bool operator==(utf8string& a, const char8_t* utf8_string_literal);
bool operator!=(utf8string& a, const char8_t* utf8_string_literal);


namespace _utf8string_tests {

	void testInsertAfter();

	void testEverything();
}
