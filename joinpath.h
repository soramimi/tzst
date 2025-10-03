
#ifndef JOINPATH_H
#define JOINPATH_H

#include <string>

std::string joinpath(char const *left, char const *right);
std::string joinpath(std::string const &left, std::string const &right);
std::wstring joinpath(std::wstring const &left, std::wstring const &right);

static inline std::string operator / (std::string const &left, std::string const &right)
{
	return joinpath(left, right);
}

static inline std::wstring operator / (std::wstring const &left, std::wstring const &right)
{
	return joinpath(left, right);
}

#endif
