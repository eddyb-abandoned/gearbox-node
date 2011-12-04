#line 1 "src/modules/buffer.gear"

// Copyright Joyent, Inc. and other Node contributors.
//           (c) 2011 the gearbox-node project authors.
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

#ifndef GEARBOX_MODULES_BUFFER_H
#define GEARBOX_MODULES_BUFFER_H

#include <gearbox.h>

#line 25 "src/modules/buffer.gear"

namespace Gearbox {
    enum encoding {ASCII, UTF8, BASE64, UCS2, BINARY, HEX};
    
    class Buffer : public Value {
    public:
        Buffer(size_t size) : Value(getCtor().newInstance(size)) {}
        static bool is(const Value &that) {
            if(!that.is<Object>())
                return false;
            return that.to<v8::Handle<v8::Object>>()->GetIndexedPropertiesExternalArrayDataType() == v8::kExternalUnsignedByteArray;
        }
        
        static uint8_t *data(const Value &that) {
            return static_cast<uint8_t*>(that.to<v8::Handle<v8::Object>>()->GetIndexedPropertiesExternalArrayData());
        }
        
        static size_t length(const Value &that) {
            return that.to<v8::Handle<v8::Object>>()->GetIndexedPropertiesExternalArrayDataLength();
        }
        
    private:
        Value &getCtor() {
            if(m_BufferCtor == undefined)
                m_BufferCtor = Context::getCurrent()->global()["Buffer"];
            return m_BufferCtor;
        }
        
        static Value m_BufferCtor;
    };
}

#endif
