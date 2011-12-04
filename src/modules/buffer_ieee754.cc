// Copyright (c) 2008, Fair Oaks Labs, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
//  * Neither the name of Fair Oaks Labs, Inc. nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//
// Modifications to writeIEEE754 to support negative zeroes made by Brian White

#include <gearbox.h>

using namespace Gearbox;

/** \file src/modules/buffer_ieee754.cc converted from src/modules/buffer_ieee754.js */



#line 41 "src/modules/buffer_ieee754.cc"
static void _setup_buffer_ieee754(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\nexports.readIEEE754 = function(buffer, offset, isBE, mLen, nBytes) {\n  var e, m,\n      eLen = nBytes * 8 - mLen - 1,\n      eMax = (1 << eLen) - 1,\n      eBias = eMax >> 1,\n      nBits = -7,\n      i = isBE ? 0 : (nBytes - 1),\n      d = isBE ? 1 : -1,\n      s = buffer[offset + i];\n\n  i += d;\n\n  e = s & ((1 << (-nBits)) - 1);\n  s >>= (-nBits);\n  nBits += eLen;\n  for (; nBits > 0; e = e * 256 + buffer[offset + i], i += d, nBits -= 8);\n\n  m = e & ((1 << (-nBits)) - 1);\n  e >>= (-nBits);\n  nBits += mLen;\n  for (; nBits > 0; m = m * 256 + buffer[offset + i], i += d, nBits -= 8);\n\n  if (e === 0) {\n    e = 1 - eBias;\n  } else if (e === eMax) {\n    return m ? NaN : ((s ? -1 : 1) * Infinity);\n  } else {\n    m = m + Math.pow(2, mLen);\n    e = e - eBias;\n  }\n  return (s ? -1 : 1) * m * Math.pow(2, e - mLen);\n};\n\nexports.writeIEEE754 = function(buffer, value, offset, isBE, mLen, nBytes) {\n  var e, m, c,\n      eLen = nBytes * 8 - mLen - 1,\n      eMax = (1 << eLen) - 1,\n      eBias = eMax >> 1,\n      rt = (mLen === 23 ? Math.pow(2, -24) - Math.pow(2, -77) : 0),\n      i = isBE ? (nBytes - 1) : 0,\n      d = isBE ? -1 : 1,\n      s = value < 0 || (value === 0 && 1 / value < 0) ? 1 : 0;\n\n  value = Math.abs(value);\n\n  if (isNaN(value) || value === Infinity) {\n    m = isNaN(value) ? 1 : 0;\n    e = eMax;\n  } else {\n    e = Math.floor(Math.log(value) / Math.LN2);\n    if (value * (c = Math.pow(2, -e)) < 1) {\n      e--;\n      c *= 2;\n    }\n    if (e + eBias >= 1) {\n      value += rt / c;\n    } else {\n      value += rt * Math.pow(2, 1 - eBias);\n    }\n    if (value * c >= 2) {\n      e++;\n      c /= 2;\n    }\n\n    if (e + eBias >= eMax) {\n      m = 0;\n      e = eMax;\n    } else if (e + eBias >= 1) {\n      m = (value * c - 1) * Math.pow(2, mLen);\n      e = e + eBias;\n    } else {\n      m = value * Math.pow(2, eBias - 1) * Math.pow(2, mLen);\n      e = 0;\n    }\n  }\n\n  for (; mLen >= 8; buffer[offset + i] = m & 0xff, i += d, m /= 256, mLen -= 8);\n\n  e = (e << mLen) | m;\n  eLen += mLen;\n  for (; eLen > 0; buffer[offset + i] = e & 0xff, i += d, e /= 256, eLen -= 8);\n\n  buffer[offset + i - d] |= s * 128;\n};\n})", "gear:buffer_ieee754")(exports, require, module);
}
static NativeModule _module_buffer_ieee754("buffer_ieee754", _setup_buffer_ieee754);