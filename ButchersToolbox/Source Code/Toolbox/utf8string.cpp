
// Header
#include "utf_string.hpp"

// Standard
#include <array>

// Mine
#include "DebugUtils.hpp"


uint32_t getUTF8_SequenceLength(uint8_t byte)
{
	if ((byte & 0b1000'0000) == 0) {
		return 1;
	}
	else if ((byte & 0b1100'0000) == 0b1100'0000) {
		return 2;
	}
	else if ((byte & 0b1110'0000) == 0b1110'0000) {
		return 3;
	}
	else if ((byte & 0b1111'0000) == 0b1111'0000) {
		return 4;
	}

	return 0xFFFF'FFFF;
}

uint32_t getUTF8_CodePointLength(uint32_t code_point)
{
	// 7 bit
	if (code_point <= 0b111'1111) {
		return 1;
	}
	// 11 bit
	else if (code_point <= 0b11111'111111) {
		return 2;
	}
	// 16 bit
	else if (code_point <= 0b1111'111111'111111) {
		return 3;
	}
	// 21 bit
	else if (code_point <= 0b111'111111'111111'111111) {
		return 4;
	}

	return 0;
}

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0)
{
	// 7 bit
	byte_0 = code_point;
}

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1)
{
	// 11 bit = 5 + 6
	byte_0 = (code_point >> 6) | 0b1100'0000;
	byte_1 = (code_point & 0b111111) | 0b1000'0000;
}

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2)
{
	// 16 bit = 4 + 6 + 6
	byte_0 = (code_point >> 12) | 0b1110'0000;
	byte_1 = (code_point >> 6) & 0b111111 | 0b1000'0000;
	byte_2 = code_point & 0b111111 | 0b1000'0000;
}

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2, uint8_t& byte_3)
{
	// 21 bit = 3 + 6 + 6 + 6
	byte_0 = (code_point >> 18) | 0b1111'0000;
	byte_1 = (code_point >> 12) & 0b111111 | 0b1000'0000;
	byte_2 = (code_point >> 6) & 0b111111 | 0b1000'0000;
	byte_3 = code_point & 0b111111 | 0b1000'0000;
}

uint32_t byteCount(utf8string_iter& begin, utf8string_iter& end)
{
	assert_cond(begin <= end, "begin cannot be further back than end");

	return end.byte_index - begin.byte_index;
}

void utf8string_iter::prev(uint32_t count)
{
	auto& bytes = parent->bytes;

	uint32_t i = 0;
	while (i < count) {

		if (byte_index == 0) {
			byte_index = -1;
			return;
		}

		byte_index--;
		uint8_t byte = bytes[byte_index];

		if (((byte & 0b1000'0000) == 0) ||
			((byte & 0b1100'0000) == 0b1100'0000) ||
			((byte & 0b1110'0000) == 0b1110'0000) ||
			((byte & 0b1111'0000) == 0b1111'0000))
		{
			return;
		}

		i++;
	}
}

void utf8string_iter::next(uint32_t count)
{
	// before iter
	if (byte_index == - 1) {
		byte_index = 0;
		return;
	}

	auto& bytes = parent->bytes;

	uint8_t byte = bytes[byte_index];

	uint32_t i = 0;
	while (i < count) {

		// don't pass after
		if (byte == 0) {
			return;
		}
		else if ((byte & 0b1000'0000) == 0) {
			byte_index += 1;
		}
		else if ((byte & 0b1100'0000) == 0b1100'0000) {
			byte_index += 2;
		}
		else if ((byte & 0b1110'0000) == 0b1110'0000) {
			byte_index += 3;
		}
		else if ((byte & 0b1111'0000) == 0b1111'0000) {
			byte_index += 4;
		}
		else {
			throw std::exception("invalid byte");
		}

		i++;
	}
}

int32_t utf8string_iter::characterIndex()
{
	auto& bytes = parent->bytes;

	if (byte_index == -1) {
		return -1;
	}
	else if (byte_index == bytes.size() - 1) {
		return 0x7FFF'FFFF;
	}

	uint32_t character_index = 0;

	int32_t byte_i = 0;
	uint8_t byte = bytes[byte_i];

	while (byte_i < byte_index) {

		if ((byte & 0b1000'0000) == 0) {
			byte_i += 1;
		}
		else if ((byte & 0b1100'0000) == 0b1100'0000) {
			byte_i += 2;
		}
		else if ((byte & 0b1110'0000) == 0b1110'0000) {
			byte_i += 3;
		}
		else if ((byte & 0b1111'0000) == 0b1111'0000) {
			byte_i += 4;
		}
		else {
			throw std::exception("invalid byte");
		}

		character_index++;
	}

	return character_index;
}

uint32_t utf8string_iter::codePoint()
{
	auto& bytes = parent->bytes;

	uint8_t byte = bytes[byte_index];

	if (byte == 0) {
		return 0;
	}
	else if ((byte & 0b1000'0000) == 0) {
		return byte & 0b0111'1111;
	}
	else if ((byte & 0b1100'0000) == 0b1100'0000) {
		uint32_t byte_0 = byte & 0b0001'1111;
		uint32_t byte_1 = bytes[byte_index + 1] & 0b0011'1111;
		return (byte_0 << 6) | byte_1;
	}
	else if ((byte & 0b1110'0000) == 0b1110'0000) {
		uint32_t byte_0 = byte & 0b0000'1111;
		uint32_t byte_1 = bytes[byte_index + 1] & 0b0011'1111;
		uint32_t byte_2 = bytes[byte_index + 2] & 0b0011'1111;
		return (byte_0 << 12) | (byte_1 << 6) | byte_2;
	}
	else if ((byte & 0b1111'0000) == 0b1111'0000) {
		uint32_t byte_0 = byte & 0b0000'0111;
		uint32_t byte_1 = bytes[byte_index + 1] & 0b0011'1111;
		uint32_t byte_2 = bytes[byte_index + 2] & 0b0011'1111;
		uint32_t byte_3 = bytes[byte_index + 3] & 0b0011'1111;
		return (byte_0 << 18) | (byte_1 << 12) | (byte_2 << 6) | byte_3;
	}

	return 0xFFFF'FFFF;
}

utf8string utf8string_iter::get()
{
#ifdef _DEBUG
	if (byte_index < 0) {
		throw std::exception();
	}
#endif

	auto& bytes = parent->bytes;
	uint8_t byte = bytes[byte_index];

	utf8string character;

	if (byte == 0) {
		
	}
	else if ((byte & 0b1000'0000) == 0) {
		character.bytes.resize(2);
		character.bytes[0] = byte & 0b0111'1111;
		character.bytes[1] = '\0';
	}
	else if ((byte & 0b1100'0000) == 0b1100'0000) {
		character.bytes.resize(3);
		character.bytes[0] = bytes[byte_index];
		character.bytes[1] = bytes[byte_index + 1];
		character.bytes[2] = '\0';
	}
	else if ((byte & 0b1110'0000) == 0b1110'0000) {
		character.bytes.resize(4);
		character.bytes[0] = bytes[byte_index];
		character.bytes[1] = bytes[byte_index + 1];
		character.bytes[2] = bytes[byte_index + 2];
		character.bytes[3] = '\0';
	}
	else if ((byte & 0b1111'0000) == 0b1111'0000) {
		character.bytes.resize(5);
		character.bytes[0] = bytes[byte_index];
		character.bytes[1] = bytes[byte_index + 1];
		character.bytes[2] = bytes[byte_index + 2];
		character.bytes[3] = bytes[byte_index + 3];
		character.bytes[4] = '\0';
	}

	return character;
}

uint8_t* utf8string_iter::data()
{
	auto& bytes = parent->bytes;
	return bytes.data() + byte_index;
}

bool utf8string_iter::isBeforeBegin()
{
	return byte_index == -1;
}

bool utf8string_iter::isAtNull()
{
	auto& bytes = parent->bytes;
	return bytes[byte_index] == 0;
}

void utf8string_iter::swap(utf8string_iter& other)
{
	int32_t aux = byte_index;
	this->byte_index = other.byte_index;
	other.byte_index = aux;
}

bool operator==(utf8string_iter a, utf8string_iter b)
{
	return a.byte_index == b.byte_index;
}

bool operator!=(utf8string_iter a, utf8string_iter b)
{
	return a.byte_index != b.byte_index;
}

bool operator<(utf8string_iter a, utf8string_iter b)
{
	return a.byte_index < b.byte_index;
}

bool operator>(utf8string_iter a, utf8string_iter b)
{
	return a.byte_index > b.byte_index;
}

bool operator<=(utf8string_iter a, utf8string_iter b)
{
	return a.byte_index <= b.byte_index;
}

bool operator>=(utf8string_iter a, utf8string_iter b)
{
	return a.byte_index >= b.byte_index;
}

bool operator==(utf8string_iter iter, const char8_t* str_literal)
{
	utf8string str = iter.get();

	for (uint32_t i = 0; i < str.bytes.size(); i++) {

		if (str.bytes[i] != str_literal[i]) {
			return false;
		}

		if (str_literal[i] == '\0') {
			break;
		}
	}

	return true;
}

utf8string::utf8string()
{
	bytes = { 0 };
}

utf8string::utf8string(const char8_t* utf8_string_literal)
{
	uint32_t count = 0;

	for (uint32_t i = 0; i < 2048; i++) {
		
		count++;

		if (utf8_string_literal[i] == '\0') {
			break;
		}
	}

	bytes.resize(count);
	std::memcpy(bytes.data(), utf8_string_literal, count);
}

utf8string::utf8string(std::string& string)
{
	bytes.resize(string.size() + 1);
	std::memcpy(bytes.data(), string.data(), bytes.size());

	bytes[bytes.size() - 1] = '\0';
}

utf8string::utf8string(std::vector<uint8_t>& bytes)
{
	bytes.resize(bytes.size() + 1);
	std::memcpy(bytes.data(), bytes.data(), bytes.size());

	bytes[bytes.size() - 1] = '\0';
}

uint32_t utf8string::length()
{
	uint32_t count = 0;

	for (utf8string_iter iter = begin(); !iter.isAtNull(); iter.next()) {
		count++;
	}

	return count;
}

bool utf8string::isEmpty()
{
	return bytes.size() == 1;
}

utf8string_iter utf8string::before()
{
	utf8string_iter iter;
	iter.parent = this;
	iter.byte_index = -1;

	return iter;
}

utf8string_iter utf8string::begin()
{
	utf8string_iter iter;
	iter.parent = this;
	iter.byte_index = 0;

	return iter;
}

utf8string_iter utf8string::last()
{
	utf8string_iter iter;
	iter.parent = this;
	iter.byte_index = bytes.size() - 1;

	if (bytes.size() > 1) {
		iter.prev();
	}

	return iter;
}

utf8string_iter utf8string::after()
{
	utf8string_iter iter;
	iter.parent = this;
	iter.byte_index = bytes.size() - 1;

	return iter;
}

void utf8string::push(uint32_t code_point)
{
	switch (getUTF8_CodePointLength(code_point)) {
	case 1: {
		bytes.resize(bytes.size() + 1);
		encodeUTF8_CodePoint(code_point, bytes[bytes.size() - 2]);
		break;
	}
	case 2: {
		bytes.resize(bytes.size() + 2);
		encodeUTF8_CodePoint(code_point, bytes[bytes.size() - 3], bytes[bytes.size() - 2]);
		break;
	}
	case 3: {
		bytes.resize(bytes.size() + 3);
		encodeUTF8_CodePoint(code_point, bytes[bytes.size() - 4], bytes[bytes.size() - 3], bytes[bytes.size() - 2]);
		break;
	}
	case 4: {
		bytes.resize(bytes.size() + 4);
		encodeUTF8_CodePoint(code_point, bytes[bytes.size() - 5], bytes[bytes.size() - 4], bytes[bytes.size() - 3], bytes[bytes.size() - 2]);
		break;
	}
	default: {
		throw std::exception("invalid code point length");
	}
	}

	bytes[bytes.size() - 1] = '\0';
}

void utf8string::insertAfter(utf8string_iter& location,
	utf8string_iter& new_content_start, utf8string_iter& new_content_end)
{
	assert_cond(location < after(), "cannot insert a character past the after iterator i.e past NULL terminator");

	// Optimization
	if (new_content_start == new_content_end) {
		return;
	}

	// move old content
	//     |displaced_content_begin
	// 0 1 2 3 4
	//         |displaced_content_end
	//
	// 0 1 X X X X 2 3 4
	//             |after_new_content
	utf8string_iter displaced_content_begin = location;
	displaced_content_begin.next();

	utf8string_iter displaced_content_end = after();

	utf8string_iter after_new_content = displaced_content_begin;
	after_new_content.byte_index += byteCount(new_content_start, new_content_end);

	overwrite(after_new_content, displaced_content_begin, displaced_content_end);

	// add new content
	overwrite(displaced_content_begin, new_content_start, new_content_end);

	bytes[bytes.size() - 1] = '\0';
}

void utf8string::insertAfter(utf8string_iter& location, utf8string& new_content)
{
	utf8string_iter new_content_start = new_content.begin();
	utf8string_iter new_content_end = new_content.after();

	insertAfter(location, new_content_start, new_content_end);
}

void utf8string::overwrite(utf8string_iter& location,
	utf8string_iter& new_content_start, utf8string_iter& new_content_end)
{
	if (new_content_start == new_content_end) {
		return;
	}
	
	uint32_t byte_count = new_content_end.byte_index - new_content_start.byte_index;

	// 0 1 2 3 4 5
	//         N
	if (location.byte_index + byte_count >= bytes.size()) {
		bytes.resize(location.byte_index + byte_count + 1);
		bytes[bytes.size() - 1] = '\0';
	}

	std::memcpy(bytes.data() + location.byte_index, new_content_start.data(), byte_count);
}

void utf8string::overwrite(utf8string_iter& location,
	utf8string_iter& new_content_start, uint32_t new_content_length)
{
	utf8string_iter new_content_end = new_content_start;
	new_content_end.next(new_content_length);

	overwrite(location, new_content_start, new_content_end);
}

void utf8string::overwrite(utf8string_iter& location, utf8string& new_content)
{
	utf8string_iter new_content_start = new_content.begin();
	utf8string_iter new_content_end = new_content.after();

	overwrite(location, new_content_start, new_content_end);
}

void utf8string::replaceSelection(utf8string_iter& select_start, uint32_t select_length,
	utf8string& new_content)
{
	if (new_content.length() == 0) {
		return;
	}

	// 2 = select start
	// 
	//         select end
	// 0 1 2 3 4 5 6 7 8 9 <- old end
	//             new content end
	// 
	//     new content start
	// 0 1 X X X X 6 5 6 7 8 9 <- new end further back or more to the begining
	//             new content end
	
	// move existing content further back after where the new content will be placed
	utf8string_iter new_content_end = select_start;
	new_content_end.next(new_content.length());

	utf8string_iter select_end = select_start;
	select_end.next(select_length);

	utf8string_iter old_end = after();

	uint32_t select_bytes_length = select_end.byte_index - select_start.byte_index;
	uint32_t new_bytes_size = bytes.size() - select_bytes_length + new_content.bytes.size() - 1;

	overwrite(new_content_end, select_end, old_end);

	// move new content into position
	overwrite(select_start, new_content);

	bytes.resize(new_bytes_size);
	bytes[bytes.size() - 1] = '\0';
}

void utf8string::erase(utf8string_iter& select_start, utf8string_iter& select_end)
{
	assert_cond(before() < select_start && select_start <= after(),
		"");
	assert_cond(before() < select_end && select_end <= after(),
		"");

	uint32_t selection_bytes_count = byteCount(select_start, select_end);
	assert_cond(select_start.byte_index + selection_bytes_count < bytes.size(),
		"cannot erase more characters than existing");

	if (select_start == select_end) {
		return;
	}
	// displace existing content to the left to fill the created gap
	else if (select_end < after()) {

		//         |select end
		// 0 1 2 3 4 5 6
		//     |select start
		// 
		//     new content start
		// 0 1 4 5 6
		//             new content end
		utf8string_iter displaced_content_end = after();

		overwrite(select_start, select_end, displaced_content_end);
	}

	bytes.resize(bytes.size() - selection_bytes_count);
	bytes[bytes.size() - 1] = '\0';
}

void utf8string::erase(utf8string_iter& select_start, uint32_t selection_length)
{
	utf8string_iter select_end = select_start;
	select_end.next(selection_length);

	erase(select_start, select_end);
}

utf8string utf8string::extract(utf8string_iter& begin, utf8string_iter& end)
{
	if (begin.byte_index == end.byte_index) {
		return utf8string();
	}

	utf8string result;
	result.bytes.resize((end.byte_index - begin.byte_index) + 1);

	utf8string* other = begin.parent;

	uint32_t result_i = 0;
	for (int32_t i = begin.byte_index; i < end.byte_index; i++) {
		result.bytes[result_i] = other->bytes[i];
		result_i++;
	}

	result.bytes[result.bytes.size() - 1] = '\0';

	return result;
}

utf8string utf8string::extract(utf8string_iter& begin, uint32_t count)
{
	if (begin.isAtNull() || count == 0) {
		return utf8string();
	}

	utf8string result;
	result.bytes.resize(count + 1);

	uint32_t i = 0;
	utf8string_iter end = begin;
	while (i < count && end.isAtNull() == false) {
		i++;
		end.next();
	}

	std::memcpy(result.bytes.data(), begin.data(),
		end.byte_index - begin.byte_index);

	result.bytes[result.bytes.size() - 1] = '\0';

	return result;
}

void utf8string::fill(std::string& other)
{
	other.resize(bytes.size() - 1);
	std::memcpy(other.data(), bytes.data(), other.size());
}

const char* utf8string::c_str()
{
	return reinterpret_cast<const char*>(bytes.data());
}


//std::wstring utf8string::w_str()
//{
//	std::wstring wide_string;
//	mbtowc(wide_string.data(), reinterpret_cast<const char*>(str.c_str()), str.length());
//
//	return wide_string
//}

bool operator==(utf8string& str, const char8_t* str_literal)
{
	for (uint32_t i = 0; i < str.bytes.size(); i++) {

		if (str.bytes[i] != str_literal[i]) {
			return false;
		}

		if (str_literal[i] == '\0') {
			break;
		}
	}

	return true;
}

bool operator!=(utf8string& str, const char8_t* str_literal)
{
	return !(str == str_literal);
}

namespace _utf8string_tests {

	void testInsertAfter()
	{
		// Before
		{
			utf8string str = u8"ăîșț";
			utf8string_iter str_iter = str.before();
			utf8string new_content = u8"Ă";

			utf8string new_str = str;
			new_str.insertAfter(str_iter, new_content);

			if (new_str != u8"Ăăîșț") {
				throw std::exception();
			}
		}

		// Begin
		{
			utf8string str = u8"ăîșț";
			utf8string_iter str_iter = str.begin();
			utf8string new_content = u8"Ă";

			utf8string new_str = str;
			new_str.insertAfter(str_iter, new_content);

			if (new_str != u8"ăĂîșț") {
				throw std::exception();
			}
		}

		// Last
		{
			utf8string str = u8"ăîșț";
			utf8string_iter str_iter = str.last();
			utf8string new_content = u8"Ă";

			utf8string new_str = str;
			new_str.insertAfter(str_iter, new_content);

			if (new_str != u8"ăîșțĂ") {
				throw std::exception();
			}
		}
	}

	void testEverything()
	{
		testInsertAfter();
	}
}
