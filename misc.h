#ifndef MISC_H
#define MISC_H

#include <string>
#include <vector>
#include <cstdint>

class misc {
public:
	struct DirEnt {
		std::string name;
		bool isdir = false;
	};

	struct FileItem {
		uint64_t size = 0;
		std::string source_path;
		std::string target_path;
	};

	static void scan_files(const std::string &dir, const std::string &prefix, std::vector<FileItem> *out);
	static void getdirents(const std::string &loc, std::vector<DirEnt> *out);
	static int mkdir(char const *dir);
	static bool mkdirs(const std::string &dir);
	static void parsedirs(const std::string &dir, std::vector<std::string> *out);
	static bool isdir(const std::string &path);
};

#endif // MISC_H
