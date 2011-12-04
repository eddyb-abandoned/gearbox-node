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

/** \file src/modules/console.cc converted from src/modules/console.js */



#line 30 "src/modules/console.cc"
static void _setup_console(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\nvar util = require('util');\n\nexports.log = function() {\n  process.stdout.write(util.format.apply(this, arguments) + '\\n');\n};\n\n\nexports.info = exports.log;\n\n\nexports.warn = function() {\n  process.stderr.write(util.format.apply(this, arguments) + '\\n');\n};\n\n\nexports.error = exports.warn;\n\n\nexports.dir = function(object) {\n  process.stdout.write(util.inspect(object) + '\\n');\n};\n\n\nvar times = {};\nexports.time = function(label) {\n  times[label] = Date.now();\n};\n\n\nexports.timeEnd = function(label) {\n  var duration = Date.now() - times[label];\n  exports.log('%s: %dms', label, duration);\n};\n\n\nexports.trace = function(label) {\n  // TODO probably can to do this better with V8's debug object once that is\n  // exposed.\n  var err = new Error;\n  err.name = 'Trace';\n  err.message = label || '';\n  Error.captureStackTrace(err, arguments.callee);\n  console.error(err.stack);\n};\n\n\nexports.assert = function(expression) {\n  if (!expression) {\n    var arr = Array.prototype.slice.call(arguments, 1);\n    require('assert').ok(false, util.format.apply(this, arr));\n  }\n};\n})", "gear:console")(exports, require, module);
}
static NativeModule _module_console("console", _setup_console);