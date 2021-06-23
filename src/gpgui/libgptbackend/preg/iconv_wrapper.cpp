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
#include "iconv_wrapper.h"
#include <climits>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <vector>

gptbackend::iconv_wrapper::iconv_wrapper(std::string from_encoding,
    std::string to_encoding) {
    this->from_encoding = from_encoding;
    this->to_encoding = to_encoding;
    this->conv =
        iconv_open(this->to_encoding.c_str(), this->from_encoding.c_str());
    if (this->invalid_open == this->conv) {
        throw std::system_error(errno, std::system_category());
    }
}

gptbackend::iconv_wrapper::~iconv_wrapper() {
    if (this->invalid_open != this->conv) {
        int result = iconv_close(this->conv);
        this->conv = this->invalid_open;
        if (0 != result) {
            std::cout << "Error on iconv_close " << errno << std::endl;
        }
    }
}

std::string gptbackend::iconv_wrapper::convert(std::string from) {
    /*
    Copyright (c) 2011, Yuya Unno
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of the Yuya Unno nor the
          names of its contributors may be used to endorse or promote products
          derived from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    */
    bool ignore_error_ = true;
    /* Values like INT_MAX cause awful slowdown */
    size_t buf_size_ = 1024;
    // copy the string to a buffer as iconv function requires a non-const char
    // pointer.
    std::vector<char> in_buf(from.begin(), from.end());
    char *src_ptr = &in_buf[0];
    size_t src_size = from.size();

    std::vector<char> buf(buf_size_);
    std::string dst;
    while (0 < src_size) {
        char *dst_ptr = &buf[0];
        size_t dst_size = buf.size();
        size_t res =
            ::iconv(this->conv, &src_ptr, &src_size, &dst_ptr, &dst_size);
        if (res == (size_t) -1) {
            if (errno == E2BIG) {
                // ignore this error
            } else if (ignore_error_) {
                // skip character
                ++src_ptr;
                --src_size;
            } else {
                this->check_conversion_error();
            }
        }
        dst.append(&buf[0], buf.size() - dst_size);
    }
    std::string output;
    dst.swap(output);
    return output;
}

void gptbackend::iconv_wrapper::check_conversion_error() {
    switch (errno) {
        case EBADF: {
            std::cout << "EBADF" << std::endl;
            break;
        }
        case E2BIG: {
            std::cout << "E2BIG" << std::endl;
            break;
        }
        case EILSEQ: {
            std::cout << "EILSEQ" << std::endl;
            break;
        }
        case EINVAL: {
            std::cout << "EINVAL" << std::endl;
            break;
        }
        default: {
            std::cout << "Unknown error " << errno << std::endl;
            break;
        }
    }
}
