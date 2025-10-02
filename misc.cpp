#include "misc.h"
#include "joinpath.h"
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define MKDIR(D) _mkdir(D)
#else
#include <unistd.h>
#define MKDIR(D) ::mkdir(D, 0755)
#define O_BINARY (0)
#endif

int misc::mkdir(char const *dir)
{
	return MKDIR(dir);
}


#ifdef _WIN32
#include <windows.h>
#include <direct.h>

void misc::getdirents(const std::string &loc, std::vector<DirEnt> *out)
{
	out->clear();
	std::string filter = loc / "*.*";
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA(filter.c_str(), &fd);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			DirEnt de;
			de.name = fd.cFileName;
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (de.name == "." || de.name == "..") {
					continue;
				}
				de.isdir = true;
			}
			out->push_back(de);
		} while (FindNextFileA(h, &fd));
		FindClose(h);
	}
}

#else // _WIN32
#include <unistd.h>
#include <dirent.h>

void misc::getdirents(std::string const &loc, std::vector<DirEnt> *out)
{
	out->clear();
	DIR *dir = opendir(loc.c_str());
	if (dir) {
		while (dirent *d = readdir(dir)) {
			DirEnt de;
			de.name = d->d_name;
			de.isdir = false;
			if (d->d_type & DT_DIR) {
				if (de.name == "." || de.name == "..") {
					continue;
				}
				de.isdir = true;
			}
			out->push_back(de);
		}
		closedir(dir);
	}
}

#endif

bool misc::mkdirs(const std::string &dir)
{
	std::vector<std::string> list;
	parsedirs(dir, &list);
	for (std::string const &d : list) {
		MKDIR(d.c_str());
	}
	return isdir(dir);
}

void misc::parsedirs(const std::string &dir, std::vector<std::string> *out)
{
	out->clear();
	std::string d;
	char const *ptr = dir.c_str();
	char const *end = ptr + dir.size();
	if (*ptr == '/' || *ptr == '\\') {
		d += '/';
	}
	char const *sep = ptr;
	while (1) {
		int c = 0;
		if (sep < end) {
			c = (unsigned char)*sep;
		}
		if (*sep == '/' || *sep == '\\' || c == 0) {
			if (ptr < sep) {
				std::string s(ptr, sep);
				d = d.empty() ? s : (d / s);
				out->push_back(d);
			}
			if (c == 0) {
				break;
			}
			ptr = sep = sep + 1;
		} else {
			sep++;
		}
	}
}

bool misc::isdir(const std::string &path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0) {
		if (st.st_mode & S_IFDIR) return true;
	}
	return true;
}

void misc::scan_files(const std::string &dir, const std::string &prefix, std::vector<FileItem> *out)
{
	std::vector<DirEnt> ents;
	getdirents(dir, &ents);

	for (DirEnt const &ent : ents) {
		std::string name = ent.name;
		FileItem item;
		item.source_path = dir / name;
		item.target_path = prefix.empty() ? name : (prefix / name);
		struct stat st;
		if (stat(item.source_path.c_str(), &st) == 0) {
			if (st.st_mode & S_IFDIR) {
				scan_files(item.source_path, item.target_path, out);
			} else {
				item.size = st.st_size;
				out->push_back(item);
			}
		}
	}
}
