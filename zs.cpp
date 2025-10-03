#include "zs.h"
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/stat.h>
#include <zstd.h>

#ifdef _WIN32
#include <io.h>
#define DEFAULT_FILE_PERMISSION (S_IREAD | S_IWRITE)
#else
#include <unistd.h>
#define O_BINARY (0)
#endif


namespace {
template <typename T> void free_(T *p);
template <> void free_(ZSTD_DCtx *p) { ZSTD_freeDCtx(p); }
template <> void free_(ZSTD_CCtx *p) { ZSTD_freeCCtx(p); }

template <typename T> class Context {
private:
	T *ctx;
public:
	explicit Context(T *p)
	{
		ctx = p;
	}
	~Context()
	{
		if (ctx) {
			free_(ctx);
		}
	}
	operator bool () const
	{
		return (bool)ctx;
	}
	operator T * () const
	{
		return ctx;
	}
};

} // namespace

/**
 * @brief Decompress data using Zstandard
 * @param opt Decompression options
 * @param in_fn Input callback function to read compressed data
 * @param out_fn Output callback function to write decompressed data
 * @param maxlen Maximum length to decompress (-1 for unlimited)
 * @return true if successful, false otherwise
 */
bool ZS::decompress(Option const &opt, std::function<int (char *, int)> in_fn, std::function<int (char const *, int)> out_fn, filesize_t maxlen)
{
	error = {};

	// Allocate input and output buffers
	const size_t buffInSize = ZSTD_DStreamInSize();
	const size_t buffOutSize = ZSTD_DStreamOutSize();
	std::unique_ptr<char> buffer(new char [buffInSize + buffOutSize]);
	char *const buffIn = buffer.get();
	char *const buffOut = buffIn + buffInSize;

	// Create decompression context
	Context<ZSTD_DCtx> dctx(ZSTD_createDCtx());
	if (!dctx) {
		error = "ZSTD_createDCtx() failed";
		return false;
	}

	const size_t toRead = buffInSize;
	filesize_t total = 0;
	bool isEmpty = true;
	while (1) {
		// Read compressed data
		const int read = in_fn(buffIn, (int)toRead);
		if (read < 1) break;
		isEmpty = false;
		ZSTD_inBuffer input = { buffIn, (size_t)read, 0 };
		// Decompress in streaming mode
		while (input.pos < input.size) {
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
			const size_t ret = ZSTD_decompressStream(dctx, &output , &input);
			if (ZSTD_isError(ret)) {
				error = ZSTD_getErrorName(ret);
				return false;
			}
			// Write decompressed data
			const int len = (int)output.pos;
			out_fn(buffOut, len);
			total += len;
			// Check if reached maximum length
			if (maxlen != (filesize_t)-1 && total >= maxlen) {
				goto done;
			}
		}
	}
done:;
	if (isEmpty) {
		error = "input is empty";
		return false;
	}
	return true;
}

/**
 * @brief Compress data using Zstandard
 * @param opt Compression options (includes compression level)
 * @param in_fn Input callback function to read uncompressed data
 * @param out_fn Output callback function to write compressed data
 * @return true if successful, false otherwise
 */
bool ZS::compress(Option const &opt, std::function<int (char *, int)> const &in_fn, std::function<int (char const *, int)> const &out_fn)
{
	error = {};

	const int CLEVEL = opt.clevel; //ZSTD_CLEVEL_DEFAULT;

	// Allocate input and output buffers
	const size_t buffInSize = ZSTD_CStreamInSize();
	const size_t buffOutSize = ZSTD_CStreamOutSize();
	std::unique_ptr<char> buffer(new char [buffInSize + buffOutSize]);
	char *const buffIn = buffer.get();
	char *const buffOut = buffIn + buffInSize;

	// Create compression context
	Context<ZSTD_CCtx> cctx(ZSTD_createCCtx());
	if (!cctx) {
		error = "ZSTD_createCCtx() failed";
		return false;
	}

	size_t ret;
	// Set compression level
	ret = ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, CLEVEL);
	if (ZSTD_isError(ret)) {
		error = ZSTD_getErrorName(ret);
		return false;
	}
	// Enable checksum for data integrity
	ret = ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1);
	if (ZSTD_isError(ret)) {
		error = ZSTD_getErrorName(ret);
		return false;
	}

	const size_t toRead = buffInSize;
	while (1) {
		// Read uncompressed data
		const int read = in_fn(buffIn, (int)toRead);
		if (read < 0) return false;
		const bool lastChunk = (read == 0);
		const ZSTD_EndDirective mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;
		ZSTD_inBuffer input = { buffIn, (size_t)read, 0 };
		int finished;
		do {
			// Compress data in streaming mode
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
			const size_t remaining = ZSTD_compressStream2(cctx, &output , &input, mode);
			if (ZSTD_isError(remaining)) {
				error = ZSTD_getErrorName(remaining);
				return false;
			}
			// Write compressed data
			out_fn(buffOut, (int)output.pos);
			finished = lastChunk ? (remaining == 0) : (input.pos == input.size);
		} while (!finished);

		if (input.pos != input.size) {
			error = "zstd only returns 0 when the input is completely consumed";
			return false;
		}
		if (lastChunk) break;
	}

	return true;
}

