
// Header
#include "utf8_string.hpp"

// Standard
#include <array>


inline void assert_cond(bool condition) {
#ifndef NDEBUG  // or _DEBUG
	if (condition != true) {
		throw std::exception();
	}
#endif
}

inline void assert_cond(bool condition, const char* fail_msg) {
#ifndef NDEBUG  // or _DEBUG
	if (condition != true) {
		throw std::exception(fail_msg);
	}
#endif
}

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
	byte_0 = (uint8_t)code_point;
}

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1)
{
	// 11 bit = 5 + 6
	byte_0 = (uint8_t)((code_point >> 6) | 0b1100'0000);
	byte_1 = (uint8_t)((code_point & 0b111111) | 0b1000'0000);
}

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2)
{
	// 16 bit = 4 + 6 + 6
	byte_0 = (uint8_t)((code_point >> 12) | 0b1110'0000);
	byte_1 = (uint8_t)((code_point >> 6) & 0b111111 | 0b1000'0000);
	byte_2 = (uint8_t)(code_point & 0b111111 | 0b1000'0000);
}

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2, uint8_t& byte_3)
{
	// 21 bit = 3 + 6 + 6 + 6
	byte_0 = (uint8_t)((code_point >> 18) | 0b1111'0000);
	byte_1 = (uint8_t)((code_point >> 12) & 0b111111 | 0b1000'0000);
	byte_2 = (uint8_t)((code_point >> 6) & 0b111111 | 0b1000'0000);
	byte_3 = (uint8_t)(code_point & 0b111111 | 0b1000'0000);
}

uint32_t byteCount(utf8string_iter& begin, utf8string_iter& end)
{
	assert_cond(begin <= end, "begin cannot be further back than end");

	return end.byte_index - begin.byte_index;
}

utf8string_iter::utf8string_iter(utf8string* p, int32_t i)
{
	this->parent = p;
	this->byte_index = i;
}

void utf8string_iter::operator++()
{
	next();
}

void utf8string_iter::operator++(int)
{
	next();
}

void utf8string_iter::operator--()
{
	prev();
}

void utf8string_iter::operator--(int)
{
	prev();
}

void utf8string_iter::prev(uint32_t count)
{
	if (isBeforeBegin()) {
		return;
	}

	auto& bytes = parent->bytes;

	uint32_t i = 0;
	while (i < count) {

		byte_index--;

		if (byte_index == -1) {
			return;
		}

		uint8_t byte = bytes[byte_index];

		// only count sequence start bytes, skip the rest
		if (((byte & 0b1000'0000) == 0) ||
			((byte & 0b1100'0000) == 0b1100'0000) ||
			((byte & 0b1110'0000) == 0b1110'0000) ||
			((byte & 0b1111'0000) == 0b1111'0000))
		{
			i++;
		}
	}
}

void utf8string_iter::next(uint32_t count)
{
	if (isAtNull()) {
		return;
	}

	if (byte_index == -1) {
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
			throw std::exception("invalid byte, usually a continuation byte");
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
			throw std::exception("invalid byte, usually a continuation byte");
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
	if (byte_index < 0 || isAtNull()) {
		return utf8string();
	}

	auto& bytes = parent->bytes;
	uint8_t byte = bytes[byte_index];

	utf8string character;

	if (byte == 0) {
		// do nothing, default counstructor is null char
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

bool utf8string_iter::isNotAtNull()
{
	return isAtNull() == false;
}

bool utf8string_iter::isAtNull()
{
	if (byte_index == -1) {
		return false;
	}

	auto& bytes = parent->bytes;
	return bytes[byte_index] == 0;
}

void utf8string_iter::swap(utf8string_iter& other)
{
	int32_t aux = byte_index;
	this->byte_index = other.byte_index;
	other.byte_index = aux;
}

bool operator==(const utf8string_iter& a, const utf8string_iter& b)
{
	return a.byte_index == b.byte_index;
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

bool operator<(const utf8string_iter& a, const utf8string_iter& b)
{
	return a.byte_index < b.byte_index;
}

bool operator>(const utf8string_iter& a, const utf8string_iter& b)
{
	return a.byte_index > b.byte_index;
}

bool operator<=(const utf8string_iter& a, const utf8string_iter& b)
{
	return a.byte_index <= b.byte_index;
}

bool operator>=(const utf8string_iter& a, const utf8string_iter& b)
{
	return a.byte_index >= b.byte_index;
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
	if (isEmpty()) {
		return 0;
	}

	uint32_t count = 0;

	for (auto iter = begin(); iter.isNotAtNull(); iter.next()) {
		count++;
	}

	return count;
}

bool utf8string::isEmpty()
{
	return bytes.size() == 1;
}

bool utf8string::contains(const utf8string& content)
{
	uint32_t content_idx = 0;
	uint32_t start = 0;

	for (int32_t i = 0; i < bytes.size() - 1; i++) {

		if (bytes[i] == content.bytes[content_idx]) {
			content_idx++;

			if (content_idx == 1) {
				start = i;
			}

			if (content_idx == content.bytes.size() - 1) {
				return true;
			}
		}
		else {
			content_idx = 0;
		}
	}

	return false;
}

utf8string_iter utf8string::find(const utf8string& content)
{
	uint32_t content_idx = 0;
	uint32_t start = 0;

	for (int32_t i = 0; i < bytes.size() - 1; i++) {

		if (bytes[i] == content.bytes[content_idx]) {
			content_idx++;

			if (content_idx == 1) {
				start = i;
			}

			if (content_idx == content.bytes.size() - 1) {
				return iter(this, start);
			}
		}
		else {
			content_idx = 0;
		}
	}

	return iter(this, -1);
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

utf8string_iter utf8string::first()
{
	return begin();
}

utf8string_iter utf8string::last()
{
	utf8string_iter iter;
	iter.parent = this;

	if (isEmpty() == false) {
		iter.byte_index = bytes.size() - 1;
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

utf8string_iter utf8string::end()
{
	return after();
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

void utf8string::push(utf8string& new_content)
{
	auto location = after();
	overwrite(location, new_content);
}

void utf8string::push(const char8_t* utf8_string_literal)
{
	utf8string new_content = utf8_string_literal;
	push(new_content);
}

void utf8string::prepend(utf8string& new_content)
{
	auto location = before();
	insertAfter(location, new_content);
}

void utf8string::prepend(const char8_t* utf8_string_literal)
{
	auto location = before();
	utf8string new_content = utf8_string_literal;
	insertAfter(location, new_content);
}

void utf8string::insertAfter(iter& location,
	iter& new_content_start, iter& new_content_end)
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
	iter displaced_content_begin = location;
	displaced_content_begin.next();

	iter displaced_content_end = after();

	iter after_new_content = displaced_content_begin;
	after_new_content.byte_index += byteCount(new_content_start, new_content_end);

	overwrite(after_new_content, displaced_content_begin, displaced_content_end);

	// add new content
	overwrite(displaced_content_begin, new_content_start, new_content_end);

	bytes[bytes.size() - 1] = '\0';
}

void utf8string::insertAfter(iter& location, utf8string& new_content)
{
	iter new_content_start = new_content.begin();
	iter new_content_end = new_content.after();

	insertAfter(location, new_content_start, new_content_end);
}

void utf8string::insertAfter(iter& location, const char8_t* utf8_string_literal)
{
	utf8string new_content = utf8_string_literal;
	insertAfter(location, new_content);
}

void utf8string::overwrite(iter& location,
	iter& new_content_start, iter& new_content_end)
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

void utf8string::overwrite(iter& location,
	iter& new_content_start, uint32_t new_content_length)
{
	iter new_content_end = new_content_start;
	new_content_end.next(new_content_length);

	overwrite(location, new_content_start, new_content_end);
}

void utf8string::overwrite(iter& location, utf8string& new_content)
{
	auto new_content_start = new_content.begin();
	auto new_content_end = new_content.after();

	overwrite(location, new_content_start, new_content_end);
}

void utf8string::overwrite(iter& location, const char8_t* utf8_string_literal)
{
	utf8string new_content = utf8_string_literal;
	overwrite(location, new_content);
}

void utf8string::replaceSelection(iter& select_start, uint32_t select_length,
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
	iter new_content_end = select_start;
	new_content_end.next(new_content.length());

	iter select_end = select_start;
	select_end.next(select_length);

	iter old_end = after();

	uint32_t select_bytes_length = select_end.byte_index - select_start.byte_index;
	uint32_t new_bytes_size = bytes.size() - select_bytes_length + new_content.bytes.size() - 1;

	overwrite(new_content_end, select_end, old_end);

	// move new content into position
	overwrite(select_start, new_content);

	bytes.resize(new_bytes_size);
	bytes[bytes.size() - 1] = '\0';
}

void utf8string::replaceSelection(iter& selection_start, uint32_t selection_length,
	const char8_t* utf8_string_literal)
{
	utf8string new_content = utf8_string_literal;
	replaceSelection(selection_start, selection_length, new_content);
}

void utf8string::erase(iter& select_start, iter& select_end)
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
		iter displaced_content_end = after();

		overwrite(select_start, select_end, displaced_content_end);
	}

	bytes.resize(bytes.size() - selection_bytes_count);
	bytes[bytes.size() - 1] = '\0';
}

void utf8string::erase(iter& select_start, uint32_t selection_length)
{
	iter select_end = select_start;
	select_end.next(selection_length);

	erase(select_start, select_end);
}

void utf8string::erase(utf8string& target)
{
	iter start = find(target);

	if (start != target.before()) {
		
		iter end = start;
		end.byte_index += target.bytes.size() - 1;

		erase(start, end);
	}
}

void utf8string::erase(const char8_t* utf8_string_literal)
{
	utf8string target = utf8_string_literal;
	erase(target);
}

void utf8string::pop(uint32_t chars_to_delete)
{
	auto i = end();
	
	while (chars_to_delete > 0 && i != before()) {
		i--;
		chars_to_delete--;
	}

	auto select_end = end();
	erase(i, select_end);
}

void utf8string::popFront(uint32_t chars_to_delete)
{
	auto i = begin();

	while (chars_to_delete > 0 && i < end()) {
		i++;
		chars_to_delete--;
	}

	auto select_begin = begin();
	erase(select_begin, i);
}

utf8string utf8string::copy(iter& begin, iter& end)
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

utf8string utf8string::copy(iter& begin, uint32_t count)
{
	if (begin.isAtNull() || count == 0) {
		return utf8string();
	}

	utf8string result;
	result.bytes.resize(count + 1);

	uint32_t i = 0;
	iter end = begin;
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

bool operator==(const utf8string& left, const char8_t* str_literal)
{
	for (uint32_t i = 0; i < left.bytes.size(); i++) {

		if (left.bytes[i] != str_literal[i]) {

			// uint8_t a = left.bytes[i];
			// uint8_t b = str_literal[i];
			return false;
		}

		if (str_literal[i] == '\0') {
			break;
		}
	}

	return true;
}

//std::wstring utf8string::w_str()
//{
//	std::wstring wide_string;
//	mbtowc(wide_string.data(), reinterpret_cast<const char*>(str.c_str()), str.length());
//
//	return wide_string
//}
