#ifndef TAR_H
#define TAR_H

#include <cstring>
#include <functional>
#include <string>

namespace tar {

struct TarData {
	std::string filename;
	int mode = 0;
	std::string uname;
	std::string gname;
	int uid = 0;
	int gid = 0;
	int chksum = 0;
	char typeflag = '0';
	char const *content = nullptr;
	int length = 0;
};

class TarWriter {
private:
	std::function<int (char const *ptr, int len)> writer_;
	int write(char const *ptr, int len);
	void write_header(const TarData &data);
	void write_content(char const *ptr, size_t len);
	void write_end();
public:
	TarWriter(std::function<int (const char *, int)> writer);
	void finish();
	void write_content(std::string const &filename, char const *content_begin, int content_length);
	bool archive(std::string const &src_dir, std::string dst_prefix_dir = {});
};

class TarReader {
private:
	std::function<int (char *ptr, int len)> reader_;
	int read(char *ptr, int len);
public:
	TarReader(std::function<int (char *ptr, int len)> reader);
	bool extract(std::string dstdir = {});
};

}

#endif // TAR_H
