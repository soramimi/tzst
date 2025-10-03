#include "joinpath.h"
#include <cstring>
#include <sstream>
#include <vector>

#ifdef WIN32
#pragma warning(disable:4996)
#endif

/**
 * @brief Trim quotation marks from begin and end pointers
 * @param begin Pointer to beginning pointer
 * @param end Pointer to ending pointer
 */
template <typename T> static inline void trimquot(T const **begin, T const **end)
{
	if (*begin + 1 < *end && (*begin)[0] == '"' && (*end)[-1] == '"') {
		(*begin)++;
		(*end)--;
	}
}

/**
 * @brief Join two path components with proper separator handling
 * @param left Left path component
 * @param right Right path component
 * @param vec Output vector for joined path
 */
template <typename T, typename U> void joinpath_(T const *left, T const *right, U *vec)
{
	size_t llen = 0;
	size_t rlen = 0;
	if (left) {
		T const *leftend = left + std::char_traits<T>::length(left);
		// Remove quotation marks if present
		trimquot(&left, &leftend);
		// Remove trailing slashes from left component
		while (left < leftend && (leftend[-1] == '/' || leftend[-1] == '\\')) {
			leftend--;
		}
		llen = leftend - left;
	}
	if (right) {
		T const *rightend = right + std::char_traits<T>::length(right);
		// Remove quotation marks if present
		trimquot(&right, &rightend);
		// Remove leading slashes from right component
		while (right < rightend && (right[0] == '/' || right[0] == '\\')) {
			right++;
		}
		rlen = rightend - right;
	}
	// Build joined path with separator
	vec->resize(llen + 1 + rlen);
	if (llen > 0) {
		std::char_traits<T>::copy(&vec->at(0), left, llen);
	}
	vec->at(llen) = '/';
	if (rlen > 0) {
		std::char_traits<T>::copy(&vec->at(llen + 1), right, rlen);
	}
}

/**
 * @brief Join two path components (char version)
 * @param left Left path component
 * @param right Right path component
 * @return Joined path string
 */
std::string joinpath(char const *left, char const *right)
{
	std::vector<char> vec;
	joinpath_(left, right, &vec);
	return std::string(vec.begin(), vec.end());
}

/**
 * @brief Join two path components (wchar_t version)
 * @param left Left path component
 * @param right Right path component
 * @return Joined path wide string
 */
std::wstring joinpath(wchar_t const *left, wchar_t const *right)
{
	std::vector<wchar_t> vec;
	joinpath_(left, right, &vec);
	return std::wstring(vec.begin(), vec.end());
}

/**
 * @brief Join two path components (std::string version)
 * @param left Left path component
 * @param right Right path component
 * @return Joined path string
 */
std::string joinpath(std::string const &left, std::string const &right)
{
	return joinpath(left.c_str(), right.c_str());
}

/**
 * @brief Join two path components (std::wstring version)
 * @param left Left path component
 * @param right Right path component
 * @return Joined path wide string
 */
std::wstring joinpath(std::wstring const &left, std::wstring const &right)
{
	return joinpath(left.c_str(), right.c_str());
}

