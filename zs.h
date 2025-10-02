#ifndef ZS_H
#define ZS_H

#include <functional>
#include <string>
#include <zstd.h>

class ZS {
private:
public:
	struct Option {
		int clevel = ZSTD_CLEVEL_DEFAULT;
	};
	using filesize_t = size_t;
	std::string error;
	bool decompress(Option const &opt, std::function<int (char *, int)> in_fn, std::function<int (const char *, int)> out_fn, filesize_t maxlen = -1);
	bool compress(Option const &opt, std::function<int (char *, int)> const &in_fn, std::function<int (char const *, int)> const &out_fn);
};

#endif // ZS_H
