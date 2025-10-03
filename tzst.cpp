#include "tzst.h"
#include "tar.h"
#include "zs.h"
#include <cstdio>
#include <deque>
#include <fcntl.h>
#include <functional>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY (0)
#endif

/**
 * @brief Create a tar.zst archive from a directory
 * @param opt Compression options
 * @param archive_path Output archive file path
 * @param src_dir Source directory to archive
 * @param dst_prefix_dir Prefix directory path in the archive
 * @return true if successful, false otherwise
 */
bool tzst::archive_tar_zst(Option const &opt, std::string const &archive_path, std::string const &src_dir, std::string const &dst_prefix_dir)
{
	// Open output file for writing
	int fd_tarzst_out = open(archive_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
	if (fd_tarzst_out == -1) {
		fprintf(stderr, "Could not create file: %s", archive_path.c_str());
		return false;
	}

	// Create tar archive in memory buffer
	std::deque<char> tar_buffer;
	tar::TarWriter tar([&](char const *ptr, int len)->int{
		tar_buffer.insert(tar_buffer.end(), ptr, ptr + len);
		return len;
	});
	tar.archive(src_dir, dst_prefix_dir);

	{
		// Input callback: read from tar buffer
		auto In = [&](char *ptr, int len)->int{
			int total = 0;
			while (len > 0) {
				if (tar_buffer.empty()) break;
				int n = std::min((int)tar_buffer.size(), len);
				std::copy(tar_buffer.begin(), tar_buffer.begin() + n, ptr);
				tar_buffer.erase(tar_buffer.begin(), tar_buffer.begin() + n);
				ptr += n;
				len -= n;
				total += n;
			}
			return total;
		};
		// Output callback: write compressed data to file
		auto Out = [&](char const *ptr, int len)->int{
			write(fd_tarzst_out, ptr, len);
			return len;
		};
		// Compress tar data with zstd
		ZS zs;
		zs.compress(opt.zsopt, In, Out);
	}

	close(fd_tarzst_out);
	return true;
}

/**
 * @brief Extract tar.zst archive from memory buffer
 * @param opt Decompression options
 * @param tarzst_data Pointer to compressed data
 * @param tarzst_size Size of compressed data
 * @param dstdir Destination directory for extraction
 * @return true if successful, false otherwise
 */
bool tzst::extract_tar_zst(Option const &opt, char const *tarzst_data, size_t tarzst_size, std::string const &dstdir)
{
	// Decompress zstd data
	std::vector<char> uncompressed;
	{
		ZS zs;
		zs.decompress(opt.zsopt, [&tarzst_data, &tarzst_size](char *ptr, int len){
			// Input callback: read from compressed buffer
			len = std::min(len, (int)tarzst_size);
			memcpy(ptr, tarzst_data, len);
			tarzst_data += len;
			tarzst_size -= len;
			return len;
		}, [&uncompressed](char const *ptr, int len){
			// Output callback: store uncompressed data
			uncompressed.insert(uncompressed.end(), ptr, ptr + len);
			return len;
		});
	}
	// Extract tar archive from uncompressed data
	char const *uncomp_data = uncompressed.data();
	size_t uncomp_size = uncompressed.size();
	tar::TarReader tar_reader([&](char *ptr, int len)->int{
		len = std::min(len, (int)uncomp_size);
		if (len > 0) {
			memcpy(ptr, uncomp_data, len);
			uncomp_data += len;
			uncomp_size -= len;
			return len;
		} else {
			return 0;
		}
	});
	tar_reader.extract(dstdir);

	return true;
}

/**
 * @brief Extract tar.zst archive from file
 * @param opt Decompression options
 * @param tarzst_path Path to the tar.zst archive file
 * @param dstdir Destination directory for extraction
 * @return true if successful, false otherwise
 */
bool tzst::extract_tar_zst(Option const &opt, std::string const &tarzst_path, std::string const &dstdir)
{
	// Open archive file
	int fd_in = open(tarzst_path.c_str(), O_RDONLY | O_BINARY);
	if (fd_in != -1) {
		struct stat st;
		if (fstat(fd_in, &st) == 0) {
			if (st.st_size > 0) {
				// Read entire file into memory
				std::vector<char> buf(st.st_size);
				int n = read(fd_in, buf.data(), st.st_size);
				if (n == st.st_size) {
					// Extract from memory buffer
					return extract_tar_zst(opt, buf.data(), buf.size(), dstdir);
				}
			}
		}
	}
	return false;
}
