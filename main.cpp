
#include "tar.h"
#include "tzst.h"
#include "zs.h"
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <chrono>
#include <thread>
#include "base64.h"

#ifndef _WIN32
#include <unistd.h>
#endif

class ElapsedTimer {
private:
	using time_point = std::chrono::system_clock::time_point;
	time_point tp_start_;
	time_point now() const
	{
		return std::chrono::system_clock::now();
	}
public:
	void start()
	{
		tp_start_ = now();
	}
	int64_t elapsed() const
	{
		time_point tp = now();
		return (std::chrono::duration_cast<std::chrono::milliseconds>(tp - tp_start_)).count();
	}
};

int main(int argc, char **argv)
{
	if (argc < 3) {
		return 1;
	}

	enum Command {
		None,
		Compress,
		Decompress,
	} command = None;

	{
		char const *p = argv[1];
		while (*p) {
			switch (*p) {
			case '-':
				break;
			case 'c':
				command = Compress;
				break;
			case 'x':
				command = Decompress;
				break;
			default:
				fprintf(stderr, "Unknown command: %c\n", *p);
				return 1;
			}
			p++;
		}
	}

	if (command == None) {
		fprintf(stderr, "Unspecified command\n");
		return 1;
	}

	std::string tarzst_path = argv[2];

	bool base64 = false;

	std::vector<std::string> files;
	int i = 3;
	while (i < argc) {
		char const *p = argv[i];
		if (*p == '-') {
			if (strcmp(p, "--base64") == 0) {
				base64 = true;
			}
		} else {
			std::string s = argv[i];
			if (base64) {
				s = base64_decode(s);
			}
			files.push_back(s);
		}
		i++;
	}

	tzst::Option opt;
	ElapsedTimer t;
	t.start();
	if (command == Compress) {
		if (files.empty()) {
			fprintf(stderr, "No file specified\n");
			return 1;
		}
		tzst::archive_tar_zst(opt, tarzst_path, files[0]);
	} else if (command == Decompress) {
		tzst::extract_tar_zst(opt, tarzst_path);
	}
	fprintf(stderr, "%d\n", (int)t.elapsed());

	return 0;
}
