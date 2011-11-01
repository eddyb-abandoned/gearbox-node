// Copyright (C) 2011 by Ben Noordhuis <info@bnoordhuis.nl>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <gearbox.h>

using namespace Gearbox;

/** \file src/modules/punycode.cc converted from src/modules/punycode.js */

#line 1 "src/modules/punycode.js"


#line 30 "src/modules/punycode.js"
static void _setup_punycode(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\nexports.encode = encode;\nexports.decode = decode;\n\nvar TMIN = 1;\nvar TMAX = 26;\nvar BASE = 36;\nvar SKEW = 38;\nvar DAMP = 700; // initial bias scaler\nvar INITIAL_N = 128;\nvar INITIAL_BIAS = 72;\nvar MAX_INTEGER = Math.pow(2, 53);\n\nfunction adapt_bias(delta, n_points, is_first) {\n  // scale back, then increase delta\n  delta /= is_first ? DAMP : 2;\n  delta += ~~(delta / n_points);\n\n  var s = (BASE - TMIN);\n  var t = ~~((s * TMAX) / 2); // threshold=455\n\n  for (var k = 0; delta > t; k += BASE) {\n    delta = ~~(delta / s);\n  }\n\n  var a = (BASE - TMIN + 1) * delta;\n  var b = (delta + SKEW);\n\n  return k + ~~(a / b);\n}\n\nfunction next_smallest_codepoint(codepoints, n) {\n  var m = 0x110000; // unicode upper bound + 1\n\n  for (var i = 0, len = codepoints.length; i < len; ++i) {\n    var c = codepoints[i];\n    if (c >= n && c < m) {\n      m = c;\n    }\n  }\n\n  // sanity check - should not happen\n  if (m >= 0x110000) {\n    throw new Error('Next smallest code point not found.');\n  }\n\n  return m;\n}\n\nfunction encode_digit(d) {\n  return d + (d < 26 ? 97 : 22);\n}\n\nfunction decode_digit(d) {\n  if (d >= 48 && d <= 57) {\n    return d - 22; // 0..9\n  }\n  if (d >= 65 && d <= 90) {\n    return d - 65; // A..Z\n  }\n  if (d >= 97 && d <= 122) {\n    return d - 97; // a..z\n  }\n  throw new Error('Illegal digit #' + d);\n}\n\nfunction threshold(k, bias) {\n  if (k <= bias + TMIN) {\n    return TMIN;\n  }\n  if (k >= bias + TMAX) {\n    return TMAX;\n  }\n  return k - bias;\n}\n\nfunction encode_int(bias, delta) {\n  var result = [];\n\n  for (var k = BASE, q = delta;; k += BASE) {\n    var t = threshold(k, bias);\n    if (q < t) {\n      result.push(encode_digit(q));\n      break;\n    }\n    else {\n      result.push(encode_digit(t + ((q - t) % (BASE - t))));\n      q = ~~((q - t) / (BASE - t));\n    }\n  }\n\n  return result;\n}\n\nfunction encode(input) {\n  if (typeof input != 'string') {\n    throw new Error('Argument must be a string.');\n  }\n\n  input = input.split('').map(function(c) {\n    return c.charCodeAt(0);\n  });\n\n  var output = [];\n  var non_basic = [];\n\n  for (var i = 0, len = input.length; i < len; ++i) {\n    var c = input[i];\n    if (c < 128) {\n      output.push(c);\n    }\n    else {\n      non_basic.push(c);\n    }\n  }\n\n  var b, h;\n  b = h = output.length;\n\n  if (b) {\n    output.push(45); // delimiter '-'\n  }\n\n  var n = INITIAL_N;\n  var bias = INITIAL_BIAS;\n  var delta = 0;\n\n  for (var len = input.length; h < len; ++n, ++delta) {\n    var m = next_smallest_codepoint(non_basic, n);\n    delta += (m - n) * (h + 1);\n    n = m;\n\n    for (var i = 0; i < len; ++i) {\n      var c = input[i];\n      if (c < n) {\n        if (++delta == MAX_INTEGER) {\n          throw new Error('Delta overflow.');\n        }\n      }\n      else if (c == n) {\n        // TODO append in-place?\n        // i.e. -> output.push.apply(output, encode_int(bias, delta));\n        output = output.concat(encode_int(bias, delta));\n        bias = adapt_bias(delta, h + 1, b == h);\n        delta = 0;\n        h++;\n      }\n    }\n  }\n\n  return String.fromCharCode.apply(String, output);\n}\n\nfunction decode(input) {\n  if (typeof input != 'string') {\n    throw new Error('Argument must be a string.');\n  }\n\n  // find basic code points/delta separator\n  var b = 1 + input.lastIndexOf('-');\n\n  input = input.split('').map(function(c) {\n    return c.charCodeAt(0);\n  });\n\n  // start with a copy of the basic code points\n  var output = input.slice(0, b ? (b - 1) : 0);\n\n  var n = INITIAL_N;\n  var bias = INITIAL_BIAS;\n\n  for (var i = 0, len = input.length; b < len; ++i) {\n    var org_i = i;\n\n    for (var k = BASE, w = 1;; k += BASE) {\n      var d = decode_digit(input[b++]);\n\n      // TODO overflow check\n      i += d * w;\n\n      var t = threshold(k, bias);\n      if (d < t) {\n        break;\n      }\n\n      // TODO overflow check\n      w *= BASE - t;\n    }\n\n    var x = 1 + output.length;\n    bias = adapt_bias(i - org_i, x, org_i == 0);\n    // TODO overflow check\n    n += ~~(i / x);\n    i %= x;\n\n    output.splice(i, 0, n);\n  }\n\n  return String.fromCharCode.apply(String, output);\n}\n})", "gear:punycode")(exports, require, module);
}
static NativeModule _module_punycode("punycode", _setup_punycode);