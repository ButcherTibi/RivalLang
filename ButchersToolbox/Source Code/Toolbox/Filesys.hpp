#pragma once

// Standard
#include <cstdint>
#include <string>
#include <vector>


namespace filesys {

	extern uint32_t max_path_length;


	void readFile(std::string& absolute_path, std::vector<uint8_t>& r_bytes);
}


//namespace io {
//
//	extern uint32_t max_path;
//
//
//	class Path {
//	public:
//		std::vector<std::string> entries;
//
//	public:
//		Path() = default;
//		Path(std::string path);  // create absolute file handle
//		Path(std::string& path);  // create absolute file handle
//
//		ErrStack recreateFromRelativePath(std::string relative_path);
//
//		bool hasExtension(std::string extension);
//
//		void push_back(std::string path);
//		void pop_back(size_t count = 1);
//
//		void push_front(std::string path);
//		void pop_front(size_t count = 1);
//
//		void erase(size_t start, size_t end);
//
//		std::string toWindowsPath();
//
//		// create temporary file handle and read the whole content
//		template<typename T = char>
//		ErrStack readOnce(std::vector<T>& content);
//	};
//
//
//	class File {
//	public:
//		std::string file_path;
//		Handle _file_handle;
//
//	public:
//		File() = default;
//		File(File& src_file) = delete;
//
//		void create(Path& path);
//
//		// opens the file with minor changes in flags for parsing files
//		ErrStack openForParsing();
//
//		// get size in bytes of the file
//		ErrStack size(size_t& r_byte_count);
//
//		ErrStack getLastWrite(uint32_t& r_day, uint32_t& r_hour, uint32_t& r_minute, uint32_t& r_second);
//
//		template<typename T = char>
//		ErrStack read(std::vector<T>& content);
//	};
//
//
//	ErrStack getSolutionPath(std::string& r_path);
//
//	template<typename T = char>
//	ErrStack readFile(std::string& path, std::vector<T>& content);
//
//	template<typename T = char>
//	ErrStack readLocalFile(std::string path, std::vector<T>& content);
//}
