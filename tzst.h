#ifndef TZST_H
#define TZST_H

#include "zs.h"

#include <functional>
#include <string>

namespace tzst {

struct Option {
	ZS::Option zsopt;
};

bool archive_tar_zst(Option const &opt, const std::string &archive_path, const std::string &src_dir, const std::string &dst_prefix_dir = {});
bool extract_tar_zst(Option const &opt, const char *tarzst_data, size_t tarzst_size, const std::string &dstdir = {});
bool extract_tar_zst(Option const &opt, std::string const &tarzst_path, std::string const &dstdir = {});

}

#endif // TZST_H
