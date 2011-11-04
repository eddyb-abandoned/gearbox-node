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

/** \file src/modules/_linklist.cc converted from src/modules/_linklist.js */

#line 1 "src/modules/_linklist.js"


#line 31 "src/modules/_linklist.js"
static void _setup__linklist(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\nfunction init(list) {\n  list._idleNext = list;\n  list._idlePrev = list;\n}\nexports.init = init;\n\n\n// show the most idle item\nfunction peek(list) {\n  if (list._idlePrev == list) return null;\n  return list._idlePrev;\n}\nexports.peek = peek;\n\n\n// remove the most idle item from the list\nfunction shift(list) {\n  var first = list._idlePrev;\n  remove(first);\n  return first;\n}\nexports.shift = shift;\n\n\n// remove a item from its list\nfunction remove(item) {\n  if (item._idleNext) {\n    item._idleNext._idlePrev = item._idlePrev;\n  }\n\n  if (item._idlePrev) {\n    item._idlePrev._idleNext = item._idleNext;\n  }\n\n  item._idleNext = null;\n  item._idlePrev = null;\n}\nexports.remove = remove;\n\n\n// remove a item from its list and place at the end.\nfunction append(list, item) {\n  remove(item);\n  item._idleNext = list._idleNext;\n  list._idleNext._idlePrev = item;\n  item._idlePrev = list;\n  list._idleNext = item;\n}\nexports.append = append;\n\n\nfunction isEmpty(list) {\n  return list._idleNext === list;\n}\nexports.isEmpty = isEmpty;\n})", "gear:_linklist")(exports, require, module);
}
static NativeModule _module__linklist("_linklist", _setup__linklist);