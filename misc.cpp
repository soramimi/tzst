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

/**
 * @brief Create a directory
 * @param dir Directory path to create
 * @return 0 on success, -1 on failure
 */
int misc::mkdir(char const *dir)
{
	return MKDIR(dir);
}


#ifdef _WIN32
#include <windows.h>
#include <direct.h>

/**
 * @brief Get directory entries (Windows implementation)
 * @param loc Directory path to scan
 * @param out Output vector to store directory entries
 */
void misc::getdirents(const std::string &loc, std::vector<DirEnt> *out)
{
	out->clear();
	std::string filter = loc / "*.*";
	WIN32_FIND_DATAA fd;
	// Find first file in directory
	HANDLE h = FindFirstFileA(filter.c_str(), &fd);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			DirEnt de;
			de.name = fd.cFileName;
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Skip current and parent directory entries
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

/**
 * @brief Get directory entries (Unix/Linux implementation)
 * @param loc Directory path to scan
 * @param out Output vector to store directory entries
 */
void misc::getdirents(std::string const &loc, std::vector<DirEnt> *out)
{
	out->clear();
	// Open directory
	DIR *dir = opendir(loc.c_str());
	if (dir) {
		// Read directory entries
		while (dirent *d = readdir(dir)) {
			DirEnt de;
			de.name = d->d_name;
			de.isdir = false;
#ifdef DT_DIR
			if (d->d_type & DT_DIR) {
				de.isdir = true;
			}
#else
			struct stat st;
			if (stat((loc / de.name).c_str(), &st) == 0) {
				if (st.st_mode & S_IFDIR) {
					de.isdir = true;
				}
			}
#endif
			if (de.isdir) {
				// Skip current and parent directory entries
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

/**
 * @brief Create directory hierarchy recursively
 * @param dir Directory path to create (can include multiple levels)
 * @return true if successful, false otherwise
 */
bool misc::mkdirs(const std::string &dir)
{
	std::vector<std::string> list;
	// Parse directory path into individual components
	parsedirs(dir, &list);
	// Create each directory level
	for (std::string const &d : list) {
		MKDIR(d.c_str());
	}
	return isdir(dir);
}

/**
 * @brief Parse directory path into individual directory components
 * @param dir Directory path to parse
 * @param out Output vector to store parsed directory levels
 */
void misc::parsedirs(const std::string &dir, std::vector<std::string> *out)
{
	out->clear();
	std::string d;
	char const *ptr = dir.c_str();
	char const *end = ptr + dir.size();
	// Handle absolute path (starting with /)
	if (*ptr == '/' || *ptr == '\\') {
		d += '/';
	}
	char const *sep = ptr;
	while (1) {
		int c = 0;
		if (sep < end) {
			c = (unsigned char)*sep;
		}
		// Split path at directory separators
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

/**
 * @brief Check if path is a directory
 * @param path Path to check
 * @return true if path is a directory, false otherwise
 */
bool misc::isdir(const std::string &path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0) {
		if (st.st_mode & S_IFDIR) return true;
	}
	return true;
}

/**
 * @brief Recursively scan directory for files
 * @param dir Directory path to scan
 * @param prefix Prefix to add to target paths
 * @param out Output vector to store file items
 */
void misc::scan_files(const std::string &dir, const std::string &prefix, std::vector<FileItem> *out)
{
	std::vector<DirEnt> ents;
	// Get all entries in current directory
	getdirents(dir, &ents);

	// Process each entry
	for (DirEnt const &ent : ents) {
		std::string name = ent.name;
		FileItem item;
		item.source_path = dir / name;
		item.target_path = prefix.empty() ? name : (prefix / name);
		struct stat st;
		if (stat(item.source_path.c_str(), &st) == 0) {
			if (st.st_mode & S_IFDIR) {
				// Recursively scan subdirectories
				scan_files(item.source_path, item.target_path, out);
			} else {
				// Add regular files to output
				item.size = st.st_size;
				out->push_back(item);
			}
		}
	}
}
