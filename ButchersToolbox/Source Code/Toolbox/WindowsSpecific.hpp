#pragma once

// Standard
#include <cstdio>
#include <string>

// Error Handling
#include "DebugUtils.hpp"


namespace win32 {

	// Win32's GetLastError but as a human readable string
	std::string getLastError();


	// Wrap Handle so that it frees memory automatically
	class Handle {
	public:
		HANDLE handle;

	public:
		Handle()
		{
			this->handle = INVALID_HANDLE_VALUE;
		}

		Handle(HANDLE ms_handle)
		{
			this->handle = ms_handle;
		}

		Handle& operator=(HANDLE ms_handle)
		{
			assert_cond(this->handle == INVALID_HANDLE_VALUE);
		
			this->handle = ms_handle;
		
			return *this;
		}
		
		bool isValid()
		{
			void* f = (void*)0xFFFF'FFFF'FFFF'FFFF;

			if (handle == INVALID_HANDLE_VALUE) {
				return false;
			}
			else if (handle == f) {
				return false;
			}

			return true;
		}
		
		~Handle()
		{
			if (isValid()) {
		
		#if _DEBUG
				if (CloseHandle(handle) == 0) {
					printf((std::string(code_location) + "failed close handle" + getLastError()).c_str());
				}
		#else
				CloseHandle(_file_handle);
		#endif
			}
		}
	};
}
