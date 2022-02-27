
#include "joinpath.h"
#include "strformat.h"
#include "zs.h"
#include <QDirIterator>
#include <QFile>
#include <cstdint>
#include <deque>
#include <fcntl.h>
#include <functional>
#include <mutex>
#include <stdio.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

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

class TarWriter {
private:
	std::function<int (char const *ptr, int len)> writer;

	struct TarData {
		std::string filename;
		std::string uname;
		std::string gname;
		int uid = 0;
		int gid = 0;
		char typeflag = '0';
		char const *content;
		int length;
	};

	int write(char const *ptr, int len)
	{
		return writer(ptr, len);
	}

	void WriteTarHeader(TarData const *data)
	{
		char tmp[512];
		memset(tmp, 0, sizeof(tmp));
		TarHeader *h = (TarHeader *)tmp;
		memcpy(h->name, data->filename.c_str(), std::min(100, (int)data->filename.size()));
		sprintf(h->mode, "%07o", 0664);
		sprintf(h->uid, "%07o", data->uid);
		sprintf(h->gid, "%07o", data->gid);
		sprintf(h->size, "%011o", data->length);
		sprintf(h->mtime, "%011o", 014202150465);
		memset(h->chksum, ' ', 8);
		h->typeflag[0] = data->typeflag;
		strncpy(h->magic, "ustar ", sizeof(h->magic));
		strncpy(h->version, " ", sizeof(h->version));
		strncpy(h->uname, data->uname.c_str(), sizeof(h->uname));
		strncpy(h->gname, data->gname.c_str(), sizeof(h->gname));

		uint sum = 0;
		for (int i = 0; i < 512; i++) {
			sum += tmp[i];
		}
		sprintf(h->chksum, "%06o", sum);

		write(tmp, 512);
	}

	void WriteTarContent(char const *ptr, size_t len)
	{
		int offset = 0;
		while (offset < len) {
			int n = len - offset;
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

	void WriteTarEnd()
	{
		char tmp[1024];
		memset(tmp, 0, 1024);
		write(tmp, 1024);
	}

public:
	TarWriter(std::function<int (char const *ptr, int len)> writer)
		: writer(writer)
	{

	}
	~TarWriter()
	{
	}
	void finish()
	{
		WriteTarEnd();
	}
	void write_content(std::string const &filename, char const *content_begin, int content_length)
	{
		if (filename.size() > 100) {
			TarData data;
			data.filename = "././@LongLink";
			data.uname = "root";
			data.gname = "root";
			data.uid = 0;
			data.gid = 0;
			data.typeflag = 'L';
			data.content = filename.c_str();
			data.length = filename.size() + 1;
			WriteTarHeader(&data);
			WriteTarContent(data.content, data.length);
		}
		{
			TarData data;
			data.filename = filename;
			data.uname = "nobody";
			data.gname = "nogroup";
			data.uid = 65534;
			data.gid = 65534;
			data.content = content_begin;
			data.length = content_length;
			WriteTarHeader(&data);
			WriteTarContent(data.content, data.length);
		}
	}
};


struct FileItem {
	uint64_t size = 0;
	QString source_path;
	QString target_path;
};


void scan_files(QString const &dir, QString const &prefix, std::vector<FileItem> *out)
{
	QDirIterator it(dir);
	while (it.hasNext()) {
		it.next();
		QFileInfo info = it.fileInfo();
		QString name = info.fileName();
		if (name.startsWith('.')) continue;
		FileItem item;
		item.source_path = dir / name;
		item.target_path = prefix.isEmpty() ? name : (prefix / name);
		if (info.isFile()) {
			item.size = info.size();
			out->push_back(item);
		} else if (info.isDir()) {
			scan_files(item.source_path, item.target_path, out);
		}
	}
}

void mkdirs(std::string const &dir)
{
	std::string d;
	char const *ptr = dir.c_str();
	char const *end = ptr + dir.size();
	char const *sep = ptr;
	while (1) {
		int c = 0;
		if (sep < end) {
			c = *sep;
		}
		if (*sep == '/' || *sep == '\\' || c == 0) {
			if (ptr < sep) {
				std::string s(ptr, sep);
				d = d.empty() ? s : (d / s);
				mkdir(d.c_str(), 0777);
			}
			if (c == 0) break;
			ptr = sep = sep + 1;
		} else {
			sep++;
		}
	}
}

bool archive_tar_zst(std::string const &archive_path, std::string const &source_dir, std::string const &dst_prefix_dir)
{
	QFile tarzst_out(QString::fromStdString(archive_path));
	if (!tarzst_out.open(QFile::WriteOnly)) {
		fprintf(stderr, "Could not create file: %s", archive_path.c_str());
		return false;
	}

	std::mutex mutex;

	std::deque<char> tar_buffer;

	TarWriter tw([&](char const *ptr, int len)->int{
		std::lock_guard lock(mutex);
		tar_buffer.insert(tar_buffer.end(), ptr, ptr + len);
		return len;
	});

	bool tar_eof = false;

	std::thread th([&](){
		auto In = [&](char *ptr, int len)->int{
			int total = 0;
			while (len > 0) {
				{
					std::lock_guard lock(mutex);
					if (tar_buffer.empty()) {
						if (tar_eof) break;
						mutex.unlock();
						goto wait;
					}
					int n = std::min((int)tar_buffer.size(), len);
					std::copy(tar_buffer.begin(), tar_buffer.begin() + n, ptr);
					tar_buffer.erase(tar_buffer.begin(), tar_buffer.begin() + n);
					ptr += n;
					len -= n;
					total += n;
					continue;
				}
wait:;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			return total;
		};
		auto Out = [&](char const *ptr, int len)->int{
			std::lock_guard lock(mutex);
			tarzst_out.write(ptr, len);
			return len;
		};

		ZS zs;
		zs.compress(In, Out);
	});

	std::vector<FileItem> files;

	QString srcdir = QString::fromStdString(source_dir);
	if (srcdir.isEmpty()) {
		srcdir = ".";
	}
	scan_files(srcdir, "", &files);

	for (FileItem const &item : files) {
		QFile file(item.source_path);
		if (file.open(QFile::ReadOnly)) {
			QString text = item.target_path;
			printf("%s\n", text.toStdString().c_str());

			std::string path = item.target_path.toStdString();
			if (!dst_prefix_dir.empty()) {
				path = dst_prefix_dir / path;
			}
			QByteArray ba_in = file.readAll();
			tw.write_content(path, ba_in.data(), ba_in.size());
		}
	}

	tw.finish();
	tar_eof = true;

	if (th.joinable()) {
		th.join();;
	}

	return true;
}

int main()
{
	std::string tarzst_path = "a.tar.zst";
	std::string source_dir = "../DeckLinkCapture";
	std::string dst_prefix_dir = "hoge/fuga/piyo";

	bool ret = archive_tar_zst(tarzst_path, source_dir, dst_prefix_dir);


	return 0;
}
