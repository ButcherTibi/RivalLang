
// Header
#include "WindowsSpecific.hpp"

using namespace win32;

std::string win32::getLastError()
{
	LPSTR buffer;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0,
		(LPSTR)&buffer,
		0,
		NULL
	);

	std::string error_msg = buffer;

	LocalFree(buffer);

	return error_msg;
}
