#include "tar.h"
#include "../tzst/joinpath.h"
#include "misc.h"
#include <cstdlib>
#include <fcntl.h>
#include <memory>
#include <set>
#include <sys/stat.h>
#include <thread>
#include <climits>

#ifdef _WIN32
#include <io.h>
#define PATH_MAX _MAX_PATH
#else
#include <unistd.h>
#define O_BINARY (0)
#endif

#define REGTYPE		'0'	/* Regular file (preferred code).  */
#define AREGTYPE	'\0'	/* Regular file (alternate code).  */
#define LNKTYPE		'1'	/* Hard link.  */
#define SYMTYPE		'2'	/* Symbolic link (hard if not supported).  */
#define CHRTYPE		'3'	/* Character special.  */
#define BLKTYPE		'4'	/* Block special.  */
#define DIRTYPE		'5'	/* Directory.  */
#define FIFOTYPE	'6'	/* Named pipe.  */
#define CONTTYPE	'7'	/* Contiguous file */
#define LONGLINKTYPE	'L'	/* LongLink */

struct TarHeader {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
};

int tar::TarWriter::write(const char *ptr, int len)
{
	return writer_(ptr, len);
}

void tar::TarWriter::write_header(const TarData &data)
{
	char tmp[512];
	memset(tmp, 0, sizeof(tmp));
	TarHeader *h = (TarHeader *)tmp;
	memcpy(h->name, data.filename.c_str(), std::min(100, (int)data.filename.size()));
	sprintf(h->mode, "%07o", data.mode);
	sprintf(h->uid, "%07o", data.uid);
	sprintf(h->gid, "%07o", data.gid);
	sprintf(h->size, "%011o", data.length);
	sprintf(h->mtime, "%011o", 014202150465);
	memset(h->chksum, ' ', 8);
	h->typeflag[0] = data.typeflag;
	strncpy(h->magic, "ustar ", sizeof(h->magic));
	strncpy(h->version, " ", sizeof(h->version));
	strncpy(h->uname, data.uname.c_str(), sizeof(h->uname));
	strncpy(h->gname, data.gname.c_str(), sizeof(h->gname));

	uint sum = 0;
	for (int i = 0; i < 512; i++) {
		sum += tmp[i];
	}
	sprintf(h->chksum, "%06o", sum);

	write(tmp, 512);
}

void tar::TarWriter::write_content(const char *ptr, size_t len)
{
	if (ptr && len > 0) {
		int offset = 0;
		while (offset < len) {
			int n = (int)len - offset;
			if (n > 512) {
				n = 512;
			}
			write(ptr + offset, n);
			if (n < 512) {
				char tmp[512];
				memset(tmp, 0, sizeof(tmp));
				write(tmp, 512 - n);
			}
			offset += n;
		}
	}
}

void tar::TarWriter::write_end()
{
	char tmp[1024];
	memset(tmp, 0, 1024);
	write(tmp, 1024);
}

tar::TarWriter::TarWriter(std::function<int (const char *, int)> writer)
	: writer_(writer)
{
}

void tar::TarWriter::finish()
{
	write_end();
}

void tar::TarWriter::write_content(const std::string &filename, const char *content_begin, int content_length)
{
	if (filename.empty()) return;

	if (filename.size() > 100) {
		TarData data;
		data.filename = "././@LongLink";
		data.uname = "root";
		data.gname = "root";
		data.uid = 0;
		data.gid = 0;
		data.typeflag = LONGLINKTYPE;
		data.content = filename.c_str();
		data.length = (int)filename.size() + 1;
		write_header(data);
		write_content(data.content, data.length);
	}
	{
		TarData data;
		data.filename = filename;
		data.uname = "nobody";
		data.gname = "nogroup";
		data.uid = 65534;
		data.gid = 65534;
		if (filename[filename.size() - 1] == '/') {
			data.mode = 0755;
			data.typeflag = DIRTYPE;
		} else {
			data.mode = 0644;
			data.typeflag = REGTYPE;
			data.content = content_begin;
			data.length = content_length;
		}
		write_header(data);
		write_content(data.content, data.length);
	}
}

bool tar::TarWriter::archive(const std::string &src_dir, std::string dst_prefix_dir)
{
	bool ok = true;

	if (misc::isdir(src_dir)) {
		std::string s = src_dir;
		size_t n = s.size();
		while (n > 0) {
			char c = s[n - 1];
			if (c == '/' || c == '\\') {
				// nop
			} else {
				s = s.substr(0, n);
				break;
			}
			n--;
		}
		auto pos = s.find_last_of('/');
		if (pos == std::string::npos) {
			pos = s.find_last_of('\\');
		}
		if (pos != std::string::npos) {
			s = s.substr(pos + 1);
		}
		dst_prefix_dir = dst_prefix_dir.empty() ? s : (dst_prefix_dir / s);
	}

	std::set<std::string> dirs;
	std::vector<misc::FileItem> files;

	std::string srcdir = src_dir;
	if (srcdir.empty()) {
		srcdir = ".";
	}
	misc::scan_files(srcdir, "", &files);

	for (misc::FileItem const &item : files) {
		int fd = open(item.source_path.c_str(), O_RDONLY | O_BINARY);
		if (fd == -1) {
			fprintf(stderr, "error: failed to open the file: %s\n", item.source_path.c_str());
			break;
		}

		std::string path = item.target_path;
		if (!dst_prefix_dir.empty()) {
			path = dst_prefix_dir / path;
		}
		{
			auto pos = path.find_last_of('/');
			if (pos != std::string::npos) {
				std::string dir = path.substr(0, pos) / "";
				auto it = dirs.find(dir);
				if (it == dirs.end()) {
					fprintf(stderr, " dir: %s\n", dir.c_str());
					dirs.insert(dirs.end(), dir);
					write_content(dir, nullptr, 0);
				}
			}
		}
		fprintf(stderr, "file: %s\n", path.c_str());
		struct stat st;
		if (fstat(fd, &st) == 0) {
			std::unique_ptr<char> arr(new char [st.st_size]);
			int64_t pos = 0;
			while (pos < st.st_size) {
				int n = (int)std::min((long)4096, (long)st.st_size - (long)pos);
				if (read(fd, arr.get() + pos, n) != n) {
					fprintf(stderr, "error: failed read from the file: %s\n", item.source_path.c_str());
					ok = false;
					goto err;
				}
				pos += n;
			}
			write_content(path, arr.get(), (int)st.st_size);
		} else {
			fprintf(stderr, "error: failed to stat the file: %s\n", item.source_path.c_str());
			ok = false;
		}
err:;
		close(fd);
	}

	finish();

	return ok;
}



int tar::TarReader::read(char *ptr, int len)
{
	return reader_(ptr, len);
}

tar::TarReader::TarReader(std::function<int (char *, int)> reader)
	: reader_(reader)
{
}

class FileWriter {
private:
	int fd_ = -1;
	std::thread th_;
	std::vector<char> buffer_;
	void closefile()
	{
		if (fd_ != -1) {
			::close(fd_);
			fd_ = -1;
		}
	}
	void run()
	{
		::write(fd_, buffer_.data(), buffer_.size());
		closefile();
	}
public:
	~FileWriter()
	{
		close();
	}
	bool open(std::string const &path, int mode)
	{
		fd_ = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);
		if (fd_ == -1) return false;
		return true;
	}
	void set(std::vector<char> const &data)
	{
		buffer_ = data;
	}
	void close()
	{
		if (th_.joinable()) {
			th_.join();
		}
		closefile();
	}
	void start()
	{
		th_ = std::thread([this](){
			run();
		});
	}
};

bool tar::TarReader::extract(std::string dstdir)
{
	if (dstdir.empty()) {
		dstdir = ".";
	}

	std::set<std::string> dirs;
	std::vector<std::shared_ptr<FileWriter>> fwriters;

	char tmp[513];
	memset(tmp, 0, sizeof(tmp));
	while (1) {
		auto ReadHeader = [&](TarData *data){
			if (read(tmp, 512) != 512) return false;
			if (tmp[0] == 0) return true;

			TarHeader *h = (TarHeader *)tmp;

			data->filename = h->name;
			data->mode = (int)strtoul(h->mode, nullptr, 8);
			data->uname = h->uname;
			data->gname = h->gname;
			data->uid = (int)strtoul(h->uid, nullptr, 8);
			data->gid = (int)strtoul(h->gid, nullptr, 8);
			data->chksum = (int)strtoul(h->chksum, nullptr, 8);
			data->typeflag = *h->typeflag;
			data->length = (int)strtoul(h->size, nullptr, 8);

			int sum = 0;
			memset(h->chksum, ' ', 8);
			for (int i = 0; i < 512; i++) {
				sum += tmp[i];
			}

			if (sum != data->chksum) {
				fprintf(stderr, "error: checksum incorrect\n");
				return false;
			}

			return true;
		};

		TarData data;
		if (!ReadHeader(&data)) {
			return false;
		}

		if (tmp[0] == 0) break;

		auto ReadContent = [&](const std::function<int (char const *ptr, int len)> &writer){
			bool ok = true;
			int offset = 0;
			while (offset < data.length) {
				if (read(tmp, 512) != 512) {
					fprintf(stderr, "error: failed to read from the tar archive\n");
					return false;
				}
				int n = data.length - offset;
				n = std::min(n, 512);
				if (ok && writer(tmp, n) != n) {
					ok = false;
				}
				offset += 512;
			}
			return ok;
		};

		if (data.typeflag == 'L' && data.filename == "././@LongLink") {
			std::vector<char> vec;
			ReadContent([&](char const *ptr, int len){
				int n = std::min(len, PATH_MAX - (int)vec.size());
				vec.insert(vec.end(), ptr, ptr + n);
				return len;
			});
			vec.push_back(0);

			if (!ReadHeader(&data)) {
				return false;
			}
			data.filename = vec.data();
		}

		if (data.typeflag == '0' || data.typeflag == 0) {
			bool ok = true;
			size_t n = data.filename.size();
			if (n > 0) {
				char c = data.filename[n - 1];
				if (c == '/' || c == '\\') {
					ok = false;
				}
			}
			if (ok) {
				{
					auto p = data.filename.find_last_of('/');
					if (p != std::string::npos) {
						std::string dir = data.filename.substr(0, p);
						auto it = dirs.find(dir);
						if (it == dirs.end()) {
							dirs.insert(dirs.end(), dir);
							fprintf(stderr, " dir: %s\n", dir.c_str());
							if (!misc::mkdirs(dstdir / dir)) {
								fprintf(stderr, "error: failed to make directory\n");
								return false;
							}
						}
					}
				}
				fprintf(stderr, "file: %s\n", data.filename.c_str());
				std::shared_ptr<FileWriter> writer = std::make_shared<FileWriter>();
				if (writer->open(dstdir / data.filename, data.mode)) {
					std::vector<char> data;
					ReadContent([&](char const *ptr, int len){
						data.insert(data.end(), ptr, ptr + len);
						return len;
					});
					writer->set(data);
					fwriters.push_back(writer);
					writer->start();
				} else {
					fprintf(stderr, "error: failed to create file: %s\n", data.filename.c_str());
				}
			}
		}
	}

	return true;
}
