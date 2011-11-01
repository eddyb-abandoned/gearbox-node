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

#include <gearbox.h>
#include "buffer.h"

using namespace Gearbox;

/** \file src/modules/buffer.cc converted from src/modules/buffer.gear */

#line 1 "src/modules/buffer.gear"
#include <assert.h>

typedef void (*freeCallback)(uint8_t *data, void *hint);

enum encoding parseEncoding(String encodingString, enum encoding _default) {
    if(!encodingString.length())
        return _default;
    
    if(encodingString.caseCompare("utf8") || encodingString.caseCompare("utf-8"))
        return UTF8;
    else if(encodingString.caseCompare("ascii"))
        return ASCII;
    else if(encodingString.caseCompare("base64"))
        return BASE64;
    else if(encodingString.caseCompare("ucs2") || encodingString.caseCompare("ucs-2"))
        return UCS2;
    else if(encodingString.caseCompare("binary"))
        return BINARY;
    else if(encodingString.caseCompare("hex"))
        return HEX;
    else if(encodingString.caseCompare("raw")) {
        std::cerr << "'raw' (array of integers) has been removed. Use 'binary'.\n";
        return BINARY;
    } else if(encodingString.caseCompare("raws")) {
        std::cerr << "'raws' encoding has been renamed to 'binary'. Please update your code.\n";
        return BINARY;
    }
    return _default;
}

Value encodeString(const void *buffer, size_t length, enum encoding encoding) {
    if(!length)
        return "";

    if(encoding == BINARY) {
        const uint8_t *cbuffer = static_cast<const uint8_t*>(buffer);
        uint16_t *buffer16 = new uint16_t[length];
        for(size_t i = 0; i < length; i++)
            // XXX is the following line platform independent?
            buffer16[i] = cbuffer[i];
        var chunk = v8::String::New(buffer16, length);
        delete [] buffer16; // TODO use ExternalTwoByteString?
        return chunk;
    }

    // utf-8 or ascii encoding
    return String(static_cast<const char*>(buffer), length);
}

/*struct SlowBuffer {
        typedef void (*freeCallback)(uint8_t *data, void *hint);
        
        //SlowBuffer();
        
        static void replace(Value &_this, uint8_t *data, size_t length, freeCallback callback, void *hint) {
            BUFFER(_this);
            if(buffer->m_pCallback)
                buffer->m_pCallback(buffer->m_pData, buffer->m_pCallbackHint);
            else if(buffer->m_nLength) {
                delete [] buffer->m_pData;
                v8::V8::AdjustAmountOfExternalAllocatedMemory(-(sizeof(SlowBuffer) + buffer->m_nLength));
            }
            
            buffer->m_nLength = length;
            buffer->m_pCallback = callback;
            buffer->m_pCallbackHint = hint;
            
            if(buffer->m_pCallback) // If it's got a callback, use provided data.
                buffer->m_pData = data;
            else if(length) {
                buffer->m_pData = new uint8_t [buffer->m_nLength];
                if(data)
                    memcpy(buffer->m_pData, data, buffer->m_nLength);
                v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(SlowBuffer) + buffer->m_nLength);
            } else
                buffer->m_pData = 0;
            
            _this.to<v8::Handle<v8::Object>>()->SetIndexedPropertiesToExternalArrayData(buffer->m_pData, v8::kExternalUnsignedByteArray, buffer->m_nLength);
            _this["length"] = buffer->m_nLength;
        }

        size_t m_nLength;
        uint8_t *m_pData;
        freeCallback m_pCallback;
        void *m_pCallbackHint;
};*/

// Returns number of bytes written.
ssize_t decodeWrite(uint8_t *_buffer, size_t length, Value val, enum encoding encoding) {
    // XXX
    // A lot of improvement can be made here. See:
    // http://code.google.com/p/v8/issues/detail?id=270
    // http://groups.google.com/group/v8-dev/browse_thread/thread/dba28a81d9215291/ece2b50a3b4022c
    // http://groups.google.com/group/v8-users/browse_thread/thread/1f83b0ba1f0a611
    
    if(val.is<Array>()) {
        std::cerr << "'raw' encoding (array of integers) has been removed. Use 'binary'.\n";
        assert(0);
        return -1;
    }
    
    char *buffer = reinterpret_cast<char*>(_buffer);
    
    v8::Handle<v8::String> str = val;
    
    if(encoding == UTF8) {
        str->WriteUtf8(buffer, length, NULL, v8::String::HINT_MANY_WRITES_EXPECTED);
        return length;
    }
    
    if(encoding == UCS2) {
        str->Write(reinterpret_cast<uint16_t*>(buffer), 0, length, v8::String::HINT_MANY_WRITES_EXPECTED);
        return length;
    }
    
    if(encoding == ASCII) {
        str->WriteAscii(buffer, 0, length, v8::String::HINT_MANY_WRITES_EXPECTED);
        return length;
    }
    
    // THIS IS AWFUL!!! FIXME
    
    assert(encoding == BINARY);
    
    uint16_t *buffer16 = new uint16_t [length];
    
    str->Write(buffer16, 0, length, v8::String::HINT_MANY_WRITES_EXPECTED);
    
    for(size_t i = 0; i < length; i++)
        buffer[i] = buffer16[i];
    
    delete [] buffer16;
    
    return length;
}

static size_t base64DecodedSize(const String &str) {
    size_t size = str.length();
    const char *const end = *str + size;
    const size_t remainder = size % 4;
    
    size = (size / 4) * 3;
    if(remainder) {
        // Special case: 1-byte input cannot be decoded
        if(size == 0 && remainder == 1)
            size = 0;
        // Non-padded input, add 1 or 2 extra bytes
        else
            size += 1 + (remainder == 3);
    }
    
    // Check for trailing padding (1 or 2 bytes)
    if(size > 0) {
        if(end[-1] == '=')
            size--;
        if(str[-2] == '=')
            size--;
    }
    
    return size;
}

static const char *base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "0123456789+/";
static const int8_t unbase64_table[] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-1,-1,-2,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};
#define unbase64(x) unbase64_table[uint8_t(x)]

#define SLICE_ARGS                                    \
ssize_t start = _start;                           \
ssize_t end = _end;                               \
if(start < 0 || end < 0)                          \
    THROW_TYPE_ERROR("Bad argument.");            \
if(start > end)                                   \
    THROW_ERROR("Must have start <= end");        \
if(size_t(end) > This.nLength)                    \
    THROW_ERROR("end cannot be longer than length");

struct _buffer_binding_SlowBuffer_wrap /*: public Value::DtorWrap*/ {
    size_t nLength;
    uint8_t *pData;
    freeCallback pCallback;
    void *pCallbackHint;

    struct This : public Value {
        This(v8::Handle<v8::Object> &&_this, _buffer_binding_SlowBuffer_wrap *wrap) : Value(_this), _wrap(wrap), nLength(wrap->nLength), pData(wrap->pData), pCallback(wrap->pCallback), pCallbackHint(wrap->pCallbackHint) {
            _this->SetPointerInInternalField(0, wrap);
        }
        This(v8::Handle<v8::Object> &&_this) : Value(_this), _wrap(static_cast<_buffer_binding_SlowBuffer_wrap*>(_this->GetPointerFromInternalField(0))), nLength(_wrap->nLength), pData(_wrap->pData), pCallback(_wrap->pCallback), pCallbackHint(_wrap->pCallbackHint) {}
        _buffer_binding_SlowBuffer_wrap *_wrap;
        size_t &nLength;
        uint8_t *&pData;
        freeCallback &pCallback;
        void *&pCallbackHint;
    };
};

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_SlowBuffer(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This(), new _buffer_binding_SlowBuffer_wrap);
    if(args.Length() >= 1) {
        #line 255 "src/modules/buffer.gear"
        Value length(args[0]);
        This.nLength = length;
        if(This.nLength) {
            This.pData = new uint8_t [This.nLength];
            v8::V8::AdjustAmountOfExternalAllocatedMemory(length);
        } else
            This.pData = 0;
        
        This.to<v8::Handle<v8::Object>>()->SetIndexedPropertiesToExternalArrayData(This.pData, v8::kExternalUnsignedByteArray, This.nLength);
        This["length"] = This.nLength;
        return undefined;
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_binarySlice(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 267 "src/modules/buffer.gear"
        Value _start(args[0]), _end(args[1]);
        SLICE_ARGS
        return encodeString(This.pData + start, end - start, BINARY);
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.binarySlice");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_asciiSlice(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 270 "src/modules/buffer.gear"
        Value _start(args[0]), _end(args[1]);
        SLICE_ARGS
        return String(This.pData + start, end - start);
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.asciiSlice");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_utf8Slice(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 273 "src/modules/buffer.gear"
        Value _start(args[0]), _end(args[1]);
        SLICE_ARGS
        return String(This.pData + start, end - start);
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.utf8Slice");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_ucs2Slice(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 276 "src/modules/buffer.gear"
        Value _start(args[0]), _end(args[1]);
        SLICE_ARGS
        return v8::String::New(reinterpret_cast<uint16_t*>(This.pData + start), (end - start) / 2);
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.ucs2Slice");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_base64Slice(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 279 "src/modules/buffer.gear"
        Value _start(args[0]), _end(args[1]);
        SLICE_ARGS
        int n = end - start;
        int out_len = (n + 2 - ((n + 2) % 3)) / 3 * 4;
        char *out = new char[out_len];
        
        uint8_t bitbuf[3];
        int i = start; // data() index
        int j = 0; // out index
        char c;
        bool b1_oob, b2_oob;
        
        while (i < end) {
            bitbuf[0] = This.pData[i++];
            
            if (i < end) {
                bitbuf[1] = This.pData[i];
                b1_oob = false;
            }  else {
                bitbuf[1] = 0;
                b1_oob = true;
            }
            i++;
            
            if (i < end) {
                bitbuf[2] = This.pData[i];
                b2_oob = false;
            }  else {
                bitbuf[2] = 0;
                b2_oob = true;
            }
            i++;
            
            c = bitbuf[0] >> 2;
            assert(c < 64);
            out[j++] = base64_table[int(c)];
            assert(j < out_len);
            
            c = ((bitbuf[0] & 0x03) << 4) | (bitbuf[1] >> 4);
            assert(c < 64);
            out[j++] = base64_table[int(c)];
            assert(j < out_len);
            
            if (b1_oob)
                out[j++] = '=';
            else {
                c = ((bitbuf[1] & 0x0F) << 2) | (bitbuf[2] >> 6);
                assert(c < 64);
                out[j++] = base64_table[int(c)];
            }
            assert(j < out_len);
            
            if (b2_oob)
                out[j++] = '=';
            else {
                c = bitbuf[2] & 0x3F;
                assert(c < 64);
                out[j++]  = base64_table[int(c)];
            }
            assert(j <= out_len);
        }
        
        String string(out, out_len);
        delete [] out;
        return string;
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.base64Slice");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_fill(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 3) {
        #line 345 "src/modules/buffer.gear"
        Value value(args[0]), _start(args[1]), _end(args[2]);
        SLICE_ARGS
        memset(This.pData + start, value.to<uint8_t>(), end - start);
        return undefined;
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.fill");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_copy(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 3) {
        #line 349 "src/modules/buffer.gear"
        Value target(args[0]), _targetStart(args[1]), _sourceStart(args[2]);
        if(!target.is<Buffer>())
            THROW_TYPE_ERROR("Target should be a Buffer");
        
        uint8_t *targetData = Buffer::data(target);
        ssize_t targetLength = Buffer::length(target);
        ssize_t targetStart = _targetStart, sourceStart = _sourceStart;
        ssize_t sourceEnd = args[3]->IsInt32() ? args[3]->Int32Value() : This.nLength; //FIXME optArgs
        
        if(sourceEnd < sourceStart)
            THROW_ERROR("sourceEnd < sourceStart");
        
        // Copy 0 bytes; we're done
        if(sourceEnd == sourceStart)
            return Integer(0);
        
        if(targetStart < 0 || targetStart >= targetLength)
            THROW_ERROR("targetStart out of bounds");
        
        if(sourceStart < 0 || sourceStart >= This.nLength)
            THROW_ERROR("sourceStart out of bounds");
        
        if(sourceEnd < 0 || sourceEnd > This.nLength)
            THROW_ERROR("sourceEnd out of bounds");
        
        ssize_t toCopy = MIN(sourceEnd - sourceStart,
                         MIN(targetLength - targetStart, This.nLength - sourceStart));
        
        // Need to use slightly slower memmove if the ranges might overlap
        memmove(targetData + targetStart, This.pData + sourceStart, toCopy);
        
        return Integer(toCopy);
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.copy");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_binaryWrite(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 383 "src/modules/buffer.gear"
        Value string(args[0]), _offset(args[1]);
        size_t length = string.length(), offset = _offset;
        
        if(length > 0 && offset >= This.nLength)
            THROW_TYPE_ERROR("Offset is out of bounds");
        
        size_t maxLength = args[2]->IsUndefined() ? This.nLength - offset : args[2]->Uint32Value(); //FIXME optArgs
        maxLength = MIN(length, MIN(This.nLength - offset, maxLength));
        return Integer(decodeWrite(This.pData + offset, maxLength, string, BINARY));
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.binaryWrite");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_asciiWrite(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 393 "src/modules/buffer.gear"
        Value string(args[0]), _offset(args[1]);
        v8::Handle<v8::String> s = string;
        size_t length = string.length(), offset = _offset;
        
        if(length > 0 && offset >= This.nLength)
            THROW_TYPE_ERROR("Offset is out of bounds");
        
        size_t maxLength = args[2]->IsUndefined() ? This.nLength - offset : args[2]->Uint32Value(); //FIXME optArgs
        maxLength = MIN(length, MIN(This.nLength - offset, maxLength));
        return Integer(s->WriteAscii(reinterpret_cast<char*>(This.pData + offset), 0, maxLength,
                                    (v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION)));
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.asciiWrite");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_utf8Write(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 405 "src/modules/buffer.gear"
        Value string(args[0]), _offset(args[1]);
        v8::Handle<v8::String> s = string;
        
        size_t length = string.length();
        if(length == 0)
            return Integer(0);
        
        size_t offset = _offset;
        
        if(length > 0 && offset >= This.nLength)
            THROW_TYPE_ERROR("Offset is out of bounds");
        
        size_t maxLength = args[2]->IsUndefined() ? This.nLength - offset : args[2]->Uint32Value(); //FIXME optArgs
        maxLength = MIN(This.nLength - offset, maxLength);
        return Integer(s->WriteUtf8(reinterpret_cast<char*>(This.pData + offset), maxLength, NULL,
                                    (v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION)));
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.utf8Write");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_ucs2Write(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 422 "src/modules/buffer.gear"
        Value string(args[0]), _offset(args[1]);
        v8::Handle<v8::String> s = string;
        size_t length = string.length(), offset = _offset;
        
        if(length > 0 && offset >= This.nLength)
            THROW_TYPE_ERROR("Offset is out of bounds");
        
        size_t maxLength = args[2]->IsUndefined() ? This.nLength - offset : args[2]->Uint32Value(); //FIXME optArgs
        maxLength = MIN(This.nLength - offset, maxLength) / 2;
        
        char *p = reinterpret_cast<char*>(This.pData + offset);
        
        int charsWritten;
        return Integer(s->Write(reinterpret_cast<uint16_t*>(This.pData + offset), 0, maxLength,
                                    (v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION)));
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.ucs2Write");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_base64Write(const v8::Arguments &args) {
    _buffer_binding_SlowBuffer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 438 "src/modules/buffer.gear"
        Value _string(args[0]), _offset(args[1]);
        assert(unbase64('/') == 63);
        assert(unbase64('+') == 62);
        assert(unbase64('T') == 19);
        assert(unbase64('Z') == 25);
        assert(unbase64('t') == 45);
        assert(unbase64('z') == 51);
        
        assert(unbase64(' ') == -2);
        assert(unbase64('\n') == -2);
        assert(unbase64('\r') == -2);
        
        size_t offset = _offset;
        // Handle zero-length buffers graciously
        if(offset == 0 && This.nLength == 0)
            return Integer(0);
        
        if(offset >= This.nLength)
            THROW_TYPE_ERROR("Offset is out of bounds");
        
        String string = _string;
        if(base64DecodedSize(string) > This.nLength - offset)
            THROW_TYPE_ERROR("Buffer too small");
        
        char a, b, c, d;
        char *start = reinterpret_cast<char*>(This.pData + offset);
        char *dst = start;
        const char *src = *string;
        const char *const srcEnd = src + string.length();
        
        while(src < srcEnd) {
            size_t remaining = srcEnd - src;
            
            while(unbase64(*src) < 0 && src < srcEnd)
                src++, remaining--;
            if(remaining == 0 || *src == '=')
                break;
            a = unbase64(*src++);
            
            while(unbase64(*src) < 0 && src < srcEnd)
                src++, remaining--;
            if(remaining <= 1 || *src == '=')
                break;
            b = unbase64(*src++);
            *dst++ = (a << 2) | ((b & 0x30) >> 4);
            
            while(unbase64(*src) < 0 && src < srcEnd)
                src++, remaining--;
            if(remaining <= 2 || *src == '=')
                break;
            c = unbase64(*src++);
            *dst++ = ((b & 0x0F) << 4) | ((c & 0x3C) >> 2);
            
            while(unbase64(*src) < 0 && src < srcEnd)
                src++, remaining--;
            if(remaining <= 3 || *src == '=')
                break;
            d = unbase64(*src++);
            *dst++ = ((c & 0x03) << 6) | (d & 0x3F);
        }
        
        return Integer(dst - start);
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.prototype.base64Write");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_byteLength(const v8::Arguments &args) {
    if(args.Length() >= 2) {
        #line 502 "src/modules/buffer.gear"
        Value string(args[0]), _enc(args[1]);
        enum encoding enc = parseEncoding(_enc, UTF8);
        
        if(enc == UTF8)
            return Integer(string.to<v8::Handle<v8::String>>()->Utf8Length());
        if(enc == BASE64)
            return Integer(base64DecodedSize(string));
        if(enc == UCS2)
            return Integer(string.length() * 2);
        if(enc == HEX)
            return Integer(string.length() / 2);
        return Integer(string.length());
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.byteLength");
}

static v8::Handle<v8::Value> _buffer_binding_SlowBuffer_makeFastBuffer(const v8::Arguments &args) {
    if(args.Length() >= 4) {
        #line 516 "src/modules/buffer.gear"
        Value _buffer(args[0]), fastBuffer(args[1]), offset(args[2]), length(args[3]);
        if(!_buffer.is<Buffer>())
            THROW_TYPE_ERROR("First argument must be a Buffer");
        
        v8::Handle<v8::Object> buffer = _buffer;
        fastBuffer.to<v8::Handle<v8::Object>>()->SetIndexedPropertiesToExternalArrayData(
            static_cast<uint8_t*>(buffer->GetIndexedPropertiesExternalArrayData()) + offset.to<size_t>(),
            v8::kExternalUnsignedByteArray,length);
        return undefined;
    }
    THROW_ERROR("Invalid call to buffer.binding.SlowBuffer.makeFastBuffer");
}


#line 622 "src/modules/buffer.cc"
static void _setup_buffer(Value exports, Value require, Value module) {
    var binding = Object();
    v8::Handle<v8::FunctionTemplate> _buffer_binding_SlowBuffer = v8::FunctionTemplate::New(_buffer_binding_SlowBuffer_SlowBuffer);
    _buffer_binding_SlowBuffer->SetClassName(String("SlowBuffer"));
    _buffer_binding_SlowBuffer->InstanceTemplate()->SetInternalFieldCount(1);
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("binarySlice", Function(_buffer_binding_SlowBuffer_binarySlice, "binarySlice"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("asciiSlice", Function(_buffer_binding_SlowBuffer_asciiSlice, "asciiSlice"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("utf8Slice", Function(_buffer_binding_SlowBuffer_utf8Slice, "utf8Slice"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("ucs2Slice", Function(_buffer_binding_SlowBuffer_ucs2Slice, "ucs2Slice"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("base64Slice", Function(_buffer_binding_SlowBuffer_base64Slice, "base64Slice"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("fill", Function(_buffer_binding_SlowBuffer_fill, "fill"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("copy", Function(_buffer_binding_SlowBuffer_copy, "copy"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("binaryWrite", Function(_buffer_binding_SlowBuffer_binaryWrite, "binaryWrite"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("asciiWrite", Function(_buffer_binding_SlowBuffer_asciiWrite, "asciiWrite"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("utf8Write", Function(_buffer_binding_SlowBuffer_utf8Write, "utf8Write"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("ucs2Write", Function(_buffer_binding_SlowBuffer_ucs2Write, "ucs2Write"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("base64Write", Function(_buffer_binding_SlowBuffer_base64Write, "base64Write"));
    _buffer_binding_SlowBuffer->PrototypeTemplate()->Set("length", Value(0));
    binding["SlowBuffer"] = _buffer_binding_SlowBuffer->GetFunction();
    binding["SlowBuffer"]["byteLength"] = Function(_buffer_binding_SlowBuffer_byteLength, "byteLength");
    binding["SlowBuffer"]["makeFastBuffer"] = Function(_buffer_binding_SlowBuffer_makeFastBuffer, "makeFastBuffer");
    Context::getCurrent()->runScript("(function(exports, require, module, binding){\n//BEGIN lib/buffer.js\n//BEGIN #gearbox\n//var SlowBuffer = process.binding('buffer').SlowBuffer;\nvar SlowBuffer = binding.SlowBuffer;\n//END #gearbox\nvar assert = require('assert');\n\nexports.INSPECT_MAX_BYTES = 50;\n\n\nfunction toHex(n) {\n  if (n < 16) return '0' + n.toString(16);\n  return n.toString(16);\n}\n\n\nSlowBuffer.prototype.inspect = function() {\n  var out = [],\n      len = this.length;\n  for (var i = 0; i < len; i++) {\n    out[i] = toHex(this[i]);\n    if (i == exports.INSPECT_MAX_BYTES) {\n      out[i + 1] = '...';\n      break;\n    }\n  }\n  return '<SlowBuffer ' + out.join(' ') + '>';\n};\n\n\nSlowBuffer.prototype.hexSlice = function(start, end) {\n  var len = this.length;\n\n  if (!start || start < 0) start = 0;\n  if (!end || end < 0 || end > len) end = len;\n\n  var out = '';\n  for (var i = start; i < end; i++) {\n    out += toHex(this[i]);\n  }\n  return out;\n};\n\n\n\nSlowBuffer.prototype.toString = function(encoding, start, end) {\n  encoding = String(encoding || 'utf8').toLowerCase();\n  start = +start || 0;\n  if (typeof end == 'undefined') end = this.length;\n\n  // Fastpath empty strings\n  if (+end == start) {\n    return '';\n  }\n\n  switch (encoding) {\n    case 'hex':\n      return this.hexSlice(start, end);\n\n    case 'utf8':\n    case 'utf-8':\n      return this.utf8Slice(start, end);\n\n    case 'ascii':\n      return this.asciiSlice(start, end);\n\n    case 'binary':\n      return this.binarySlice(start, end);\n\n    case 'base64':\n      return this.base64Slice(start, end);\n\n    case 'ucs2':\n    case 'ucs-2':\n      return this.ucs2Slice(start, end);\n\n    default:\n      throw new Error('Unknown encoding');\n  }\n};\n\n\nSlowBuffer.prototype.hexWrite = function(string, offset, length) {\n  offset = +offset || 0;\n  var remaining = this.length - offset;\n  if (!length) {\n    length = remaining;\n  } else {\n    length = +length;\n    if (length > remaining) {\n      length = remaining;\n    }\n  }\n\n  // must be an even number of digits\n  var strLen = string.length;\n  if (strLen % 2) {\n    throw new Error('Invalid hex string');\n  }\n  if (length > strLen / 2) {\n    length = strLen / 2;\n  }\n  for (var i = 0; i < length; i++) {\n    var byte = parseInt(string.substr(i * 2, 2), 16);\n    if (isNaN(byte)) throw new Error('Invalid hex string');\n    this[offset + i] = byte;\n  }\n  SlowBuffer._charsWritten = i * 2;\n  return i;\n};\n\n\nSlowBuffer.prototype.write = function(string, offset, length, encoding) {\n  // Support both (string, offset, length, encoding)\n  // and the legacy (string, encoding, offset, length)\n  if (isFinite(offset)) {\n    if (!isFinite(length)) {\n      encoding = length;\n      length = undefined;\n    }\n  } else {  // legacy\n    var swap = encoding;\n    encoding = offset;\n    offset = length;\n    length = swap;\n  }\n\n  offset = +offset || 0;\n  var remaining = this.length - offset;\n  if (!length) {\n    length = remaining;\n  } else {\n    length = +length;\n    if (length > remaining) {\n      length = remaining;\n    }\n  }\n  encoding = String(encoding || 'utf8').toLowerCase();\n\n  switch (encoding) {\n    case 'hex':\n      return this.hexWrite(string, offset, length);\n\n    case 'utf8':\n    case 'utf-8':\n      return this.utf8Write(string, offset, length);\n\n    case 'ascii':\n      return this.asciiWrite(string, offset, length);\n\n    case 'binary':\n      return this.binaryWrite(string, offset, length);\n\n    case 'base64':\n      return this.base64Write(string, offset, length);\n\n    case 'ucs2':\n    case 'ucs-2':\n      return this.ucs2Write(string, offset, length);\n\n    default:\n      throw new Error('Unknown encoding');\n  }\n};\n\n\n// slice(start, end)\nSlowBuffer.prototype.slice = function(start, end) {\n  if (end === undefined) end = this.length;\n\n  if (end > this.length) {\n    throw new Error('oob');\n  }\n  if (start > end) {\n    throw new Error('oob');\n  }\n\n  return new Buffer(this, end - start, +start);\n};\n\n\nfunction coerce(length) {\n  // Coerce length to a number (possibly NaN), round up\n  // in case it's fractional (e.g. 123.456) then do a\n  // double negate to coerce a NaN to 0. Easy, right?\n  length = ~~Math.ceil(+length);\n  return length < 0 ? 0 : length;\n}\n\n\n// Buffer\n\nfunction Buffer(subject, encoding, offset) {\n  if (!(this instanceof Buffer)) {\n    return new Buffer(subject, encoding, offset);\n  }\n\n  var type;\n\n  // Are we slicing?\n  if (typeof offset === 'number') {\n    this.length = coerce(encoding);\n    this.parent = subject;\n    this.offset = offset;\n  } else {\n    // Find the length\n    switch (type = typeof subject) {\n      case 'number':\n        this.length = coerce(subject);\n        break;\n\n      case 'string':\n        this.length = Buffer.byteLength(subject, encoding);\n        break;\n\n      case 'object': // Assume object is an array\n        this.length = coerce(subject.length);\n        break;\n\n      default:\n        throw new Error('First argument needs to be a number, ' +\n                        'array or string.');\n    }\n\n    if (this.length > Buffer.poolSize) {\n      // Big buffer, just alloc one.\n      this.parent = new SlowBuffer(this.length);\n      this.offset = 0;\n\n    } else {\n      // Small buffer.\n      if (!pool || pool.length - pool.used < this.length) allocPool();\n      this.parent = pool;\n      this.offset = pool.used;\n      pool.used += this.length;\n    }\n\n    // Treat array-ish objects as a byte array.\n    if (isArrayIsh(subject)) {\n      for (var i = 0; i < this.length; i++) {\n        this.parent[i + this.offset] = subject[i];\n      }\n    } else if (type == 'string') {\n      // We are a string\n      this.length = this.write(subject, 0, encoding);\n    }\n  }\n\n  SlowBuffer.makeFastBuffer(this.parent, this, this.offset, this.length);\n}\n\nfunction isArrayIsh(subject) {\n  return Array.isArray(subject) || Buffer.isBuffer(subject) ||\n         subject && typeof subject === 'object' &&\n         typeof subject.length === 'number';\n}\n\nexports.SlowBuffer = SlowBuffer;\nexports.Buffer = Buffer;\n\nBuffer.poolSize = 8 * 1024;\nvar pool;\n\nfunction allocPool() {\n  pool = new SlowBuffer(Buffer.poolSize);\n  pool.used = 0;\n}\n\n\n// Static methods\nBuffer.isBuffer = function isBuffer(b) {\n  return b instanceof Buffer || b instanceof SlowBuffer;\n};\n\n\n// Inspect\nBuffer.prototype.inspect = function inspect() {\n  var out = [],\n      len = this.length;\n\n  for (var i = 0; i < len; i++) {\n    out[i] = toHex(this.parent[i + this.offset]);\n    if (i == exports.INSPECT_MAX_BYTES) {\n      out[i + 1] = '...';\n      break;\n    }\n  }\n\n  return '<Buffer ' + out.join(' ') + '>';\n};\n\n\nBuffer.prototype.get = function get(i) {\n  if (i < 0 || i >= this.length) throw new Error('oob');\n  return this.parent[this.offset + i];\n};\n\n\nBuffer.prototype.set = function set(i, v) {\n  if (i < 0 || i >= this.length) throw new Error('oob');\n  return this.parent[this.offset + i] = v;\n};\n\n\n// write(string, offset = 0, length = buffer.length-offset, encoding = 'utf8')\nBuffer.prototype.write = function(string, offset, length, encoding) {\n  // Support both (string, offset, length, encoding)\n  // and the legacy (string, encoding, offset, length)\n  if (isFinite(offset)) {\n    if (!isFinite(length)) {\n      encoding = length;\n      length = undefined;\n    }\n  } else {  // legacy\n    var swap = encoding;\n    encoding = offset;\n    offset = length;\n    length = swap;\n  }\n\n  offset = +offset || 0;\n  var remaining = this.length - offset;\n  if (!length) {\n    length = remaining;\n  } else {\n    length = +length;\n    if (length > remaining) {\n      length = remaining;\n    }\n  }\n  encoding = String(encoding || 'utf8').toLowerCase();\n\n  var ret;\n  switch (encoding) {\n    case 'hex':\n      ret = this.parent.hexWrite(string, this.offset + offset, length);\n      break;\n\n    case 'utf8':\n    case 'utf-8':\n      ret = this.parent.utf8Write(string, this.offset + offset, length);\n      break;\n\n    case 'ascii':\n      ret = this.parent.asciiWrite(string, this.offset + offset, length);\n      break;\n\n    case 'binary':\n      ret = this.parent.binaryWrite(string, this.offset + offset, length);\n      break;\n\n    case 'base64':\n      // Warning: maxLength not taken into account in base64Write\n      ret = this.parent.base64Write(string, this.offset + offset, length);\n      break;\n\n    case 'ucs2':\n    case 'ucs-2':\n      ret = this.parent.ucs2Write(string, this.offset + offset, length);\n      break;\n\n    default:\n      throw new Error('Unknown encoding');\n  }\n\n  Buffer._charsWritten = SlowBuffer._charsWritten;\n\n  return ret;\n};\n\n\n// toString(encoding, start=0, end=buffer.length)\nBuffer.prototype.toString = function(encoding, start, end) {\n  encoding = String(encoding || 'utf8').toLowerCase();\n\n  if (typeof start == 'undefined' || start < 0) {\n    start = 0;\n  } else if (start > this.length) {\n    start = this.length;\n  }\n\n  if (typeof end == 'undefined' || end > this.length) {\n    end = this.length;\n  } else if (end < 0) {\n    end = 0;\n  }\n\n  start = start + this.offset;\n  end = end + this.offset;\n\n  switch (encoding) {\n    case 'hex':\n      return this.parent.hexSlice(start, end);\n\n    case 'utf8':\n    case 'utf-8':\n      return this.parent.utf8Slice(start, end);\n\n    case 'ascii':\n      return this.parent.asciiSlice(start, end);\n\n    case 'binary':\n      return this.parent.binarySlice(start, end);\n\n    case 'base64':\n      return this.parent.base64Slice(start, end);\n\n    case 'ucs2':\n    case 'ucs-2':\n      return this.parent.ucs2Slice(start, end);\n\n    default:\n      throw new Error('Unknown encoding');\n  }\n};\n\n\n// byteLength\nBuffer.byteLength = SlowBuffer.byteLength;\n\n\n// fill(value, start=0, end=buffer.length)\nBuffer.prototype.fill = function fill(value, start, end) {\n  value || (value = 0);\n  start || (start = 0);\n  end || (end = this.length);\n\n  if (typeof value === 'string') {\n    value = value.charCodeAt(0);\n  }\n  if (!(typeof value === 'number') || isNaN(value)) {\n    throw new Error('value is not a number');\n  }\n\n  if (end < start) throw new Error('end < start');\n\n  // Fill 0 bytes; we're done\n  if (end === start) return 0;\n  if (this.length == 0) return 0;\n\n  if (start < 0 || start >= this.length) {\n    throw new Error('start out of bounds');\n  }\n\n  if (end < 0 || end > this.length) {\n    throw new Error('end out of bounds');\n  }\n\n  return this.parent.fill(value,\n                          start + this.offset,\n                          end + this.offset);\n};\n\n\n// copy(targetBuffer, targetStart=0, sourceStart=0, sourceEnd=buffer.length)\nBuffer.prototype.copy = function(target, target_start, start, end) {\n  var source = this;\n  start || (start = 0);\n  end || (end = this.length);\n  target_start || (target_start = 0);\n\n  if (end < start) throw new Error('sourceEnd < sourceStart');\n\n  // Copy 0 bytes; we're done\n  if (end === start) return 0;\n  if (target.length == 0 || source.length == 0) return 0;\n\n  if (target_start < 0 || target_start >= target.length) {\n    throw new Error('targetStart out of bounds');\n  }\n\n  if (start < 0 || start >= source.length) {\n    throw new Error('sourceStart out of bounds');\n  }\n\n  if (end < 0 || end > source.length) {\n    throw new Error('sourceEnd out of bounds');\n  }\n\n  // Are we oob?\n  if (end > this.length) {\n    end = this.length;\n  }\n\n  if (target.length - target_start < end - start) {\n    end = target.length - target_start + start;\n  }\n\n  return this.parent.copy(target.parent,\n                          target_start + target.offset,\n                          start + this.offset,\n                          end + this.offset);\n};\n\n\n// slice(start, end)\nBuffer.prototype.slice = function(start, end) {\n  if (end === undefined) end = this.length;\n  if (end > this.length) throw new Error('oob');\n  if (start > end) throw new Error('oob');\n\n  return new Buffer(this.parent, end - start, +start + this.offset);\n};\n\n\n// Legacy methods for backwards compatibility.\n\nBuffer.prototype.utf8Slice = function(start, end) {\n  return this.toString('utf8', start, end);\n};\n\nBuffer.prototype.binarySlice = function(start, end) {\n  return this.toString('binary', start, end);\n};\n\nBuffer.prototype.asciiSlice = function(start, end) {\n  return this.toString('ascii', start, end);\n};\n\nBuffer.prototype.utf8Write = function(string, offset) {\n  return this.write(string, offset, 'utf8');\n};\n\nBuffer.prototype.binaryWrite = function(string, offset) {\n  return this.write(string, offset, 'binary');\n};\n\nBuffer.prototype.asciiWrite = function(string, offset) {\n  return this.write(string, offset, 'ascii');\n};\n\nBuffer.prototype.readUInt8 = function(offset, noAssert) {\n  var buffer = this;\n\n  if (!noAssert) {\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  return buffer[offset];\n};\n\nfunction readUInt16(buffer, offset, isBigEndian, noAssert) {\n  var val = 0;\n\n\n  if (!noAssert) {\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 1 < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  if (isBigEndian) {\n    val = buffer[offset] << 8;\n    val |= buffer[offset + 1];\n  } else {\n    val = buffer[offset];\n    val |= buffer[offset + 1] << 8;\n  }\n\n  return val;\n}\n\nBuffer.prototype.readUInt16LE = function(offset, noAssert) {\n  return readUInt16(this, offset, false, noAssert);\n};\n\nBuffer.prototype.readUInt16BE = function(offset, noAssert) {\n  return readUInt16(this, offset, true, noAssert);\n};\n\nfunction readUInt32(buffer, offset, isBigEndian, noAssert) {\n  var val = 0;\n\n  if (!noAssert) {\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 3 < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  if (isBigEndian) {\n    val = buffer[offset + 1] << 16;\n    val |= buffer[offset + 2] << 8;\n    val |= buffer[offset + 3];\n    val = val + (buffer[offset] << 24 >>> 0);\n  } else {\n    val = buffer[offset + 2] << 16;\n    val |= buffer[offset + 1] << 8;\n    val |= buffer[offset];\n    val = val + (buffer[offset + 3] << 24 >>> 0);\n  }\n\n  return val;\n}\n\nBuffer.prototype.readUInt32LE = function(offset, noAssert) {\n  return readUInt32(this, offset, false, noAssert);\n};\n\nBuffer.prototype.readUInt32BE = function(offset, noAssert) {\n  return readUInt32(this, offset, true, noAssert);\n};\n\n\n/*\n * Signed integer types, yay team! A reminder on how two's complement actually\n * works. The first bit is the signed bit, i.e. tells us whether or not the\n * number should be positive or negative. If the two's complement value is\n * positive, then we're done, as it's equivalent to the unsigned representation.\n *\n * Now if the number is positive, you're pretty much done, you can just leverage\n * the unsigned translations and return those. Unfortunately, negative numbers\n * aren't quite that straightforward.\n *\n * At first glance, one might be inclined to use the traditional formula to\n * translate binary numbers between the positive and negative values in two's\n * complement. (Though it doesn't quite work for the most negative value)\n * Mainly:\n *  - invert all the bits\n *  - add one to the result\n *\n * Of course, this doesn't quite work in Javascript. Take for example the value\n * of -128. This could be represented in 16 bits (big-endian) as 0xff80. But of\n * course, Javascript will do the following:\n *\n * > ~0xff80\n * -65409\n *\n * Whoh there, Javascript, that's not quite right. But wait, according to\n * Javascript that's perfectly correct. When Javascript ends up seeing the\n * constant 0xff80, it has no notion that it is actually a signed number. It\n * assumes that we've input the unsigned value 0xff80. Thus, when it does the\n * binary negation, it casts it into a signed value, (positive 0xff80). Then\n * when you perform binary negation on that, it turns it into a negative number.\n *\n * Instead, we're going to have to use the following general formula, that works\n * in a rather Javascript friendly way. I'm glad we don't support this kind of\n * weird numbering scheme in the kernel.\n *\n * (BIT-MAX - (unsigned)val + 1) * -1\n *\n * The astute observer, may think that this doesn't make sense for 8-bit numbers\n * (really it isn't necessary for them). However, when you get 16-bit numbers,\n * you do. Let's go back to our prior example and see how this will look:\n *\n * (0xffff - 0xff80 + 1) * -1\n * (0x007f + 1) * -1\n * (0x0080) * -1\n */\nBuffer.prototype.readInt8 = function(offset, noAssert) {\n  var buffer = this;\n  var neg;\n\n  if (!noAssert) {\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  neg = buffer[offset] & 0x80;\n  if (!neg) {\n    return (buffer[offset]);\n  }\n\n  return ((0xff - buffer[offset] + 1) * -1);\n};\n\nfunction readInt16(buffer, offset, isBigEndian, noAssert) {\n  var neg;\n\n  if (!noAssert) {\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 1 < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  val = readUInt16(buffer, offset, isBigEndian, noAssert);\n  neg = val & 0x8000;\n  if (!neg) {\n    return val;\n  }\n\n  return (0xffff - val + 1) * -1;\n}\n\nBuffer.prototype.readInt16LE = function(offset, noAssert) {\n  return readInt16(this, offset, false, noAssert);\n};\n\nBuffer.prototype.readInt16BE = function(offset, noAssert) {\n  return readInt16(this, offset, true, noAssert);\n};\n\nfunction readInt32(buffer, offset, isBigEndian, noAssert) {\n  var neg;\n\n  if (!noAssert) {\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 3 < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  val = readUInt32(buffer, offset, isBigEndian, noAssert);\n  neg = val & 0x80000000;\n  if (!neg) {\n    return (val);\n  }\n\n  return (0xffffffff - val + 1) * -1;\n}\n\nBuffer.prototype.readInt32LE = function(offset, noAssert) {\n  return readInt32(this, offset, false, noAssert);\n};\n\nBuffer.prototype.readInt32BE = function(offset, noAssert) {\n  return readInt32(this, offset, true, noAssert);\n};\n\nfunction readFloat(buffer, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset + 3 < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  return require('buffer_ieee754').readIEEE754(buffer, offset, isBigEndian,\n      23, 4);\n}\n\nBuffer.prototype.readFloatLE = function(offset, noAssert) {\n  return readFloat(this, offset, false, noAssert);\n};\n\nBuffer.prototype.readFloatBE = function(offset, noAssert) {\n  return readFloat(this, offset, true, noAssert);\n};\n\nfunction readDouble(buffer, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset + 7 < buffer.length,\n        'Trying to read beyond buffer length');\n  }\n\n  return require('buffer_ieee754').readIEEE754(buffer, offset, isBigEndian,\n      52, 8);\n}\n\nBuffer.prototype.readDoubleLE = function(offset, noAssert) {\n  return readDouble(this, offset, false, noAssert);\n};\n\nBuffer.prototype.readDoubleBE = function(offset, noAssert) {\n  return readDouble(this, offset, true, noAssert);\n};\n\n\n/*\n * We have to make sure that the value is a valid integer. This means that it is\n * non-negative. It has no fractional component and that it does not exceed the\n * maximum allowed value.\n *\n *      value           The number to check for validity\n *\n *      max             The maximum value\n */\nfunction verifuint(value, max) {\n  assert.ok(typeof (value) == 'number',\n      'cannot write a non-number as a number');\n\n  assert.ok(value >= 0,\n      'specified a negative value for writing an unsigned value');\n\n  assert.ok(value <= max, 'value is larger than maximum value for type');\n\n  assert.ok(Math.floor(value) === value, 'value has a fractional component');\n}\n\nBuffer.prototype.writeUInt8 = function(value, offset, noAssert) {\n  var buffer = this;\n\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset < buffer.length,\n        'trying to write beyond buffer length');\n\n    verifuint(value, 0xff);\n  }\n\n  buffer[offset] = value;\n};\n\nfunction writeUInt16(buffer, value, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 1 < buffer.length,\n        'trying to write beyond buffer length');\n\n    verifuint(value, 0xffff);\n  }\n\n  if (isBigEndian) {\n    buffer[offset] = (value & 0xff00) >>> 8;\n    buffer[offset + 1] = value & 0x00ff;\n  } else {\n    buffer[offset + 1] = (value & 0xff00) >>> 8;\n    buffer[offset] = value & 0x00ff;\n  }\n}\n\nBuffer.prototype.writeUInt16LE = function(value, offset, noAssert) {\n  writeUInt16(this, value, offset, false, noAssert);\n};\n\nBuffer.prototype.writeUInt16BE = function(value, offset, noAssert) {\n  writeUInt16(this, value, offset, true, noAssert);\n};\n\nfunction writeUInt32(buffer, value, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 3 < buffer.length,\n        'trying to write beyond buffer length');\n\n    verifuint(value, 0xffffffff);\n  }\n\n  if (isBigEndian) {\n    buffer[offset] = (value >>> 24) & 0xff;\n    buffer[offset + 1] = (value >>> 16) & 0xff;\n    buffer[offset + 2] = (value >>> 8) & 0xff;\n    buffer[offset + 3] = value & 0xff;\n  } else {\n    buffer[offset + 3] = (value >>> 24) & 0xff;\n    buffer[offset + 2] = (value >>> 16) & 0xff;\n    buffer[offset + 1] = (value >>> 8) & 0xff;\n    buffer[offset] = value & 0xff;\n  }\n}\n\nBuffer.prototype.writeUInt32LE = function(value, offset, noAssert) {\n  writeUInt32(this, value, offset, false, noAssert);\n};\n\nBuffer.prototype.writeUInt32BE = function(value, offset, noAssert) {\n  writeUInt32(this, value, offset, true, noAssert);\n};\n\n\n/*\n * We now move onto our friends in the signed number category. Unlike unsigned\n * numbers, we're going to have to worry a bit more about how we put values into\n * arrays. Since we are only worrying about signed 32-bit values, we're in\n * slightly better shape. Unfortunately, we really can't do our favorite binary\n * & in this system. It really seems to do the wrong thing. For example:\n *\n * > -32 & 0xff\n * 224\n *\n * What's happening above is really: 0xe0 & 0xff = 0xe0. However, the results of\n * this aren't treated as a signed number. Ultimately a bad thing.\n *\n * What we're going to want to do is basically create the unsigned equivalent of\n * our representation and pass that off to the wuint* functions. To do that\n * we're going to do the following:\n *\n *  - if the value is positive\n *      we can pass it directly off to the equivalent wuint\n *  - if the value is negative\n *      we do the following computation:\n *         mb + val + 1, where\n *         mb   is the maximum unsigned value in that byte size\n *         val  is the Javascript negative integer\n *\n *\n * As a concrete value, take -128. In signed 16 bits this would be 0xff80. If\n * you do out the computations:\n *\n * 0xffff - 128 + 1\n * 0xffff - 127\n * 0xff80\n *\n * You can then encode this value as the signed version. This is really rather\n * hacky, but it should work and get the job done which is our goal here.\n */\n\n/*\n * A series of checks to make sure we actually have a signed 32-bit number\n */\nfunction verifsint(value, max, min) {\n  assert.ok(typeof (value) == 'number',\n      'cannot write a non-number as a number');\n\n  assert.ok(value <= max, 'value larger than maximum allowed value');\n\n  assert.ok(value >= min, 'value smaller than minimum allowed value');\n\n  assert.ok(Math.floor(value) === value, 'value has a fractional component');\n}\n\nfunction verifIEEE754(value, max, min) {\n  assert.ok(typeof (value) == 'number',\n      'cannot write a non-number as a number');\n\n  assert.ok(value <= max, 'value larger than maximum allowed value');\n\n  assert.ok(value >= min, 'value smaller than minimum allowed value');\n}\n\nBuffer.prototype.writeInt8 = function(value, offset, noAssert) {\n  var buffer = this;\n\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset < buffer.length,\n        'Trying to write beyond buffer length');\n\n    verifsint(value, 0x7f, -0xf0);\n  }\n\n  if (value >= 0) {\n    buffer.writeUInt8(value, offset, noAssert);\n  } else {\n    buffer.writeUInt8(0xff + value + 1, offset, noAssert);\n  }\n};\n\nfunction writeInt16(buffer, value, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 1 < buffer.length,\n        'Trying to write beyond buffer length');\n\n    verifsint(value, 0x7fff, -0xf000);\n  }\n\n  if (value >= 0) {\n    writeUInt16(buffer, value, offset, isBigEndian, noAssert);\n  } else {\n    writeUInt16(buffer, 0xffff + value + 1, offset, isBigEndian, noAssert);\n  }\n}\n\nBuffer.prototype.writeInt16LE = function(value, offset, noAssert) {\n  writeInt16(this, value, offset, false, noAssert);\n};\n\nBuffer.prototype.writeInt16BE = function(value, offset, noAssert) {\n  writeInt16(this, value, offset, true, noAssert);\n};\n\nfunction writeInt32(buffer, value, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 3 < buffer.length,\n        'Trying to write beyond buffer length');\n\n    verifsint(value, 0x7fffffff, -0xf0000000);\n  }\n\n  if (value >= 0) {\n    writeUInt32(buffer, value, offset, isBigEndian, noAssert);\n  } else {\n    writeUInt32(buffer, 0xffffffff + value + 1, offset, isBigEndian, noAssert);\n  }\n}\n\nBuffer.prototype.writeInt32LE = function(value, offset, noAssert) {\n  writeInt32(this, value, offset, false, noAssert);\n};\n\nBuffer.prototype.writeInt32BE = function(value, offset, noAssert) {\n  writeInt32(this, value, offset, true, noAssert);\n};\n\nfunction writeFloat(buffer, value, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 3 < buffer.length,\n        'Trying to write beyond buffer length');\n\n    verifIEEE754(value, 3.4028234663852886e+38, -3.4028234663852886e+38);\n  }\n\n  require('buffer_ieee754').writeIEEE754(buffer, value, offset, isBigEndian,\n      23, 4);\n}\n\nBuffer.prototype.writeFloatLE = function(value, offset, noAssert) {\n  writeFloat(this, value, offset, false, noAssert);\n};\n\nBuffer.prototype.writeFloatBE = function(value, offset, noAssert) {\n  writeFloat(this, value, offset, true, noAssert);\n};\n\nfunction writeDouble(buffer, value, offset, isBigEndian, noAssert) {\n  if (!noAssert) {\n    assert.ok(value !== undefined && value !== null,\n        'missing value');\n\n    assert.ok(typeof (isBigEndian) === 'boolean',\n        'missing or invalid endian');\n\n    assert.ok(offset !== undefined && offset !== null,\n        'missing offset');\n\n    assert.ok(offset + 7 < buffer.length,\n        'Trying to write beyond buffer length');\n\n    verifIEEE754(value, 1.7976931348623157E+308, -1.7976931348623157E+308);\n  }\n\n  require('buffer_ieee754').writeIEEE754(buffer, value, offset, isBigEndian,\n      52, 8);\n}\n\nBuffer.prototype.writeDoubleLE = function(value, offset, noAssert) {\n  writeDouble(this, value, offset, false, noAssert);\n};\n\nBuffer.prototype.writeDoubleBE = function(value, offset, noAssert) {\n  writeDouble(this, value, offset, true, noAssert);\n};\n//END lib/buffer.js\n    })", "gear:buffer")(exports, require, module, binding);
}
static NativeModule _module_buffer("buffer", _setup_buffer);