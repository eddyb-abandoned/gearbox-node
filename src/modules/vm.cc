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

using namespace Gearbox;

/** \file src/modules/vm.cc converted from src/modules/vm.gear */

#line 1 "src/modules/vm.gear"

static v8::Handle<v8::Value> _vm_exports_Script_Script(const v8::Arguments &args) {
    Value This(args.This());
    #line 28 "src/modules/vm.gear"
    
    return undefined;
}

static v8::Handle<v8::Value> _vm_exports_runInThisContext(const v8::Arguments &args) {
    if(args.Length() >= 2) {
        #line 29 "src/modules/vm.gear"
        Value source(args[0]), name(args[1]);
        return Context::getCurrent()->runScript(source, name);
    }
    THROW_ERROR("Invalid call to vm.exports.runInThisContext");
}

static v8::Handle<v8::Value> _vm_exports_runInNewContext(const v8::Arguments &args) {
    if(args.Length() >= 3) {
        #line 32 "src/modules/vm.gear"
        Value source(args[0]), sandbox(args[1]), name(args[2]);
        Context context;
        context.setSandbox(sandbox);
        return context.runScript(source, name);
    }
    THROW_ERROR("Invalid call to vm.exports.runInNewContext");
}


#line 59 "src/modules/vm.cc"
static void _setup_vm(Value exports, Value require, Value module) {
    v8::Handle<v8::FunctionTemplate> _vm_exports_Script = v8::FunctionTemplate::New(_vm_exports_Script_Script);
    _vm_exports_Script->SetClassName(String("Script"));
    exports["Script"] = _vm_exports_Script->GetFunction();
    exports["runInThisContext"] = Function(_vm_exports_runInThisContext, "runInThisContext");
    exports["runInNewContext"] = Function(_vm_exports_runInNewContext, "runInNewContext");
    Context::getCurrent()->runScript("(function(exports, require, module){\nexports.createScript = function(code, ctx, name) {\n  return new exports.Script(code, ctx, name);\n};\n    })", "gear:vm")(exports, require, module);
}
static NativeModule _module_vm("vm", _setup_vm);