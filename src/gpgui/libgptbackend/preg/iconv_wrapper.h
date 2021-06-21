/*
 * GPGUI - Group Policy Editor GUI
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#if !defined(__GPTBACKEND_ICONV_WRAPPER)
#define __GPTBACKEND_ICONV_WRAPPER 1

#include <string>

#include <iconv.h>

namespace gptbackend {

/**
 * Wrapper for POSIX iconv functionality to ease the access from C++
 * and provide a convenient way to operate on std::string buffers.
 */
class iconv_wrapper {
    /* C++ reinterpret_cast<> and static_cast<> can't be used in
     * this case */
    const iconv_t invalid_open = (iconv_t) -1;

    /* Disable copy since iconv_t can't duplicate */
    iconv_wrapper(iconv_wrapper const &) = delete;
    iconv_wrapper &operator=(iconv_wrapper const &) = delete;

    iconv_t conv;
    std::string from_encoding;
    std::string to_encoding;

public:
    iconv_wrapper(std::string from_encoding, std::string to_encoding);
    ~iconv_wrapper();

    /**
     * Convert std::string to another format.
     */
    std::string convert(std::string from);

private:
    /**
     * Check if there were conversion errors.
     */
    void check_conversion_error();
}; /* class iconv_wrapper */

} /* namespace gptbackend */

#endif /* __GPTBACKEND_ICONV_WRAPPER */
