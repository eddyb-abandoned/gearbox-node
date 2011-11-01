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

#include <gearbox.h>

using namespace Gearbox;

/** \file src/modules/Io.cc converted from src/modules/Io.gear */

#line 1 "src/modules/Io.gear"
#include <fstream>

static var CError(String prefix = "") {
    return Error(prefix + strerror(errno));
}
#define THROW_CERROR(...) THROW(CError(__VA_ARGS__))

static v8::Handle<v8::Value> _Io_exports_read(const v8::Arguments &args) {
    if(args.Length() >= 1) {
        #line 35 "src/modules/Io.gear"
        Value filePath(args[0]);
        std::ifstream file(*filePath.to<String>(), std::ifstream::in | std::ifstream::binary);
        if(!file.good())
            THROW_CERROR(filePath.to<String>() + ": ");
        
        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);
        
        char *pBuffer = new char [length];
        
        file.read(pBuffer, length);
        String contents(pBuffer, length);
        
        delete [] pBuffer;
        return contents;
    }
    THROW_ERROR("Invalid call to Io.exports.read");
}

static v8::Handle<v8::Value> _Io_exports_write(const v8::Arguments &args) {
    if(args.Length() >= 2) {
        #line 53 "src/modules/Io.gear"
        Value filePath(args[0]), contents(args[1]);
        std::ofstream file(*filePath.to<String>());
        if(!file.good())
            THROW_CERROR(filePath.to<String>() + ": ");
        
        file.write(*contents.to<String>(), contents.length());
        return undefined;
    }
    THROW_ERROR("Invalid call to Io.exports.write");
}


#line 74 "src/modules/Io.cc"
static void _setup_Io(Value exports, Value require, Value module) {
    exports["read"] = Function(_Io_exports_read, "read");
    exports["write"] = Function(_Io_exports_write, "write");
}
static NativeModule _module_Io("Io", _setup_Io);