// Copyright Joyent, Inc. and other Node contributors.
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

/** \file src/modules/string_decoder.cc converted from src/modules/string_decoder.js */



#line 30 "src/modules/string_decoder.cc"
static void _setup_string_decoder(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\nvar StringDecoder = exports.StringDecoder = function(encoding) {\n  this.encoding = (encoding || 'utf8').toLowerCase().replace(/[-_]/, '');\n  if (this.encoding === 'utf8') {\n    this.charBuffer = new Buffer(4);\n    this.charReceived = 0;\n    this.charLength = 0;\n  }\n};\n\n\nStringDecoder.prototype.write = function(buffer) {\n  // If not utf8...\n  if (this.encoding !== 'utf8') {\n    return buffer.toString(this.encoding);\n  }\n\n  var charStr = '';\n  // if our last write ended with an incomplete multibyte character\n  if (this.charLength) {\n    // determine how many remaining bytes this buffer has to offer for this char\n    var i = (buffer.length >= this.charLength - this.charReceived) ?\n                this.charLength - this.charReceived :\n                buffer.length;\n\n    // add the new bytes to the char buffer\n    buffer.copy(this.charBuffer, this.charReceived, 0, i);\n    this.charReceived += i;\n\n    if (this.charReceived < this.charLength) {\n      // still not enough chars in this buffer? wait for more ...\n      return '';\n    }\n\n    // get the character that was split\n    charStr = this.charBuffer.slice(0, this.charLength).toString();\n    this.charReceived = this.charLength = 0;\n\n    // if there are no more bytes in this buffer, just emit our char\n    if (i == buffer.length) return charStr;\n\n    // otherwise cut off the characters end from the beginning of this buffer\n    buffer = buffer.slice(i, buffer.length);\n  }\n\n\n  // determine how many bytes we have to check at the end of this buffer\n  var i = (buffer.length >= 3) ? 3 : buffer.length;\n\n  // Figure out if one of the last i bytes of our buffer announces an\n  // incomplete char.\n  for (; i > 0; i--) {\n    var c = buffer[buffer.length - i];\n\n    // See http://en.wikipedia.org/wiki/UTF-8#Description\n\n    // 110XXXXX\n    if (i == 1 && c >> 5 == 0x06) {\n      this.charLength = 2;\n      break;\n    }\n\n    // 1110XXXX\n    if (i <= 2 && c >> 4 == 0x0E) {\n      this.charLength = 3;\n      break;\n    }\n\n    // 11110XXX\n    if (i <= 3 && c >> 3 == 0x1E) {\n      this.charLength = 4;\n      break;\n    }\n  }\n\n  if (!this.charLength) {\n    // no incomplete char at the end of this buffer, emit the whole thing\n    return charStr + buffer.toString();\n  }\n\n  // buffer the incomplete character bytes we got\n  buffer.copy(this.charBuffer, 0, buffer.length - i, buffer.length);\n  this.charReceived = i;\n\n  if (buffer.length - i > 0) {\n    // buffer had more bytes before the incomplete char, emit them\n    return charStr + buffer.toString('utf8', 0, buffer.length - i);\n  }\n\n  // or just emit the charStr\n  return charStr;\n};\n})", "gear:string_decoder")(exports, require, module);
}
static NativeModule _module_string_decoder("string_decoder", _setup_string_decoder);