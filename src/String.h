// Copyright (c) 2011 the gearbox-node project authors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef GEARBOX_STRING_H
#define GEARBOX_STRING_H

#include <gearbox.h>

#include <string>
#include <algorithm>

namespace Gearbox {
    class String : public std::string {
    public:
        String() {}
        String(char *pString) : std::string(pString) {}
        String(char *pString, size_t nLength) : std::string(pString, nLength) {}
        String(const char *pString) : std::string(pString) {}
        String(const char *pString, size_t nLength) : std::string(pString, nLength) {}
        String(uint8_t *pString) : std::string(reinterpret_cast<char*>(pString)) {}
        String(uint8_t *pString, size_t nLength) : std::string(reinterpret_cast<char*>(pString), nLength) {}
        String(const std::string &that) : std::string(that) {}
        String(const std::string &&that) : std::string(that) {}
        
        String toLower() const {
            std::string str(length(), 0);
            std::transform(begin(), end(), str.begin(), [](char c) {return 'A' <= c && c <= 'Z' ? c - 'A' + 'a' : c;});
            return str;
        }
        String toUpper() const {
            std::string str(length(), 0);
            std::transform(begin(), end(), str.begin(), [](char c) {return 'a' <= c && c <= 'z' ? c - 'a' + 'A' : c;});
            return str;
        }
        
        /** Convert operators */
        operator const char*() const {
            return c_str();
        }
        operator char*() const {
            return const_cast<char*>(c_str());
        }
        
        operator v8::Handle<v8::String>() const {
            return v8::String::New(data(), length());
        }
        operator v8::Handle<v8::Value>() const {
            return operator v8::Handle<v8::String>();
        }

        static bool is(const v8::Handle<v8::Value> &handle) {
            return handle->IsString();
        }
    };
}

#endif
