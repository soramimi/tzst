#include "base64.h"
#include "tar.h"
#include "tzst.h"
#include "zs.h"
#include <chrono>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>

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
	/**
	 * @brief Start the timer
	 */
	void start()
	{
		tp_start_ = now();
	}
	/**
	 * @brief Get elapsed time in milliseconds
	 * @return Elapsed time since start() was called
	 */
	int64_t elapsed() const
	{
		time_point tp = now();
		return (std::chrono::duration_cast<std::chrono::milliseconds>(tp - tp_start_)).count();
	}
};

/**
 * @brief Main entry point for tar.zst compression/decompression tool
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code (0 for success, 1 for error)
 */
int main(int argc, char **argv)
{
	// Check minimum argument count
	if (argc < 3) {
		return 1;
	}

	enum Command {
		None,
		Compress,
		Decompress,
	} command = None;

	// Parse command option from first argument
	{
		char const *p = argv[1];
		// Skip leading dash if present
		if (*p == '-') {
			p++;
		}
		// Process each character in the command option
		while (*p) {
			switch (*p) {
			case 'c':
				// Set compress command
				if (command == None) {
					command = Compress;
				} else {
					fprintf(stderr, "conflict command: %c\n", *p);
				}
				break;
			case 'x':
				// Set decompress/extract command
				if (command == None) {
					command = Decompress;
				} else {
					fprintf(stderr, "conflict command: %c\n", *p);
				}
				break;
			default:
				fprintf(stderr, "unknown command: %c\n", *p);
				return 1;
			}
			p++;
		}
	}

	// Ensure a command was specified
	if (command == None) {
		fprintf(stderr, "unspecified command\n");
		return 1;
	}

	// Get archive file path from second argument
	std::string tarzst_path = argv[2];

	// Collect remaining arguments as file list
	std::vector<std::string> files;
	int i = 3;
	while (i < argc) {
		char const *p = argv[i];
		if (command == Compress) {
			// Add files to compress
			files.push_back(p);
		} else {
			// Decompress doesn't take additional file arguments
			fprintf(stderr, "extra argument: %s\n", p);
			return 1;
		}
		i++;
	}

	// Execute compression or decompression
	ElapsedTimer t;
	t.start();
	tzst::Option opt;
	if (command == Compress) {
		// Perform compression
		if (files.empty()) {
			fprintf(stderr, "no file specified\n");
			return 1;
		}
		tzst::archive_tar_zst(opt, tarzst_path, files[0]);
	} else if (command == Decompress) {
		// Perform decompression/extraction
		tzst::extract_tar_zst(opt, tarzst_path);
	}
	// Print elapsed time in milliseconds
	// fprintf(stderr, "%d\n", (int)t.elapsed());

	return 0;
}
