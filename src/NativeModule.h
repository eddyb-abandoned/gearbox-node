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

#ifndef GEARBOX_NATIVE_MODULE_H
#define GEARBOX_NATIVE_MODULE_H

#include <gearbox.h>

#include <map>

namespace Gearbox {
    class NativeModule {
        public:
            typedef void (*SetupCallback)(Value, Value, Value);
            
            NativeModule(String moduleName, SetupCallback pSetupCallback);
            
            virtual ~NativeModule();
            
            static Value require(String moduleName, Value requireFunc=undefined);
            
            static Value getRequireFunc();
            
        private:
            String m_sModuleName;
            SetupCallback m_pSetupCallback;
            
            Value m_oModule;
            
            Value _require(Value requireFunc);
            
            static v8::Handle<v8::Value> require(const v8::Arguments& args);
            
            static std::map<String, NativeModule*> *m_pModules;
            
            static Value m_RequireFunc;
    };
}

#endif
