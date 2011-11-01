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

/** \file src/modules/sys.cc converted from src/modules/sys.js */

#line 1 "src/modules/sys.js"


#line 31 "src/modules/sys.js"
static void _setup_sys(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\nvar util = require('util');\n\nvar sysWarning;\nif (!sysWarning) {\n  sysWarning = 'The \"sys\" module is now called \"util\". ' +\n               'It should have a similar interface.';\n  util.error(sysWarning);\n}\n\nexports.print = util.print;\nexports.puts = util.puts;\nexports.debug = util.debug;\nexports.error = util.error;\nexports.inspect = util.inspect;\nexports.p = util.p;\nexports.log = util.log;\nexports.exec = util.exec;\nexports.pump = util.pump;\nexports.inherits = util.inherits;\n})", "gear:sys")(exports, require, module);
}
static NativeModule _module_sys("sys", _setup_sys);