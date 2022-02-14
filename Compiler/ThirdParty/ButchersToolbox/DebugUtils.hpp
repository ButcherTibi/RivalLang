#pragma once

// Standard
#include <string>


#define code_location \
	("ln = " + std::to_string(__LINE__) + \
	" fn = " + __func__ + \
	" in file " + __FILE__).c_str()


#define varName(variabile) \
	#variabile


/* Exceptions */

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
