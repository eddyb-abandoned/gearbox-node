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
            typedef void (*SetupCallback)(Value exports, Value require, Value module);
            
            NativeModule(String moduleName, SetupCallback pSetupCallback) : m_sModuleName(moduleName), m_pSetupCallback(pSetupCallback) {
                fixMap();
                (*m_pModules)[moduleName] = this;
            }
            
            virtual ~NativeModule() {
                m_pModules->erase(m_sModuleName);
            }
            
            static Value require(String moduleName, Value requireFunc) {
                if(!exists(moduleName))
                    THROW_ERROR("NativeModule "+moduleName+" doesn't exist");
                
                return (*m_pModules)[moduleName]->_require(requireFunc);
            }
            
            static bool exists(String moduleName) {
                fixMap();
                
                return m_pModules->count(moduleName);
            }
            
        private:
            String m_sModuleName;
            SetupCallback m_pSetupCallback;
            
            Value m_oModule;
            
            Value _require(Value requireFunc);
            
            static void fixMap() {
                if(!m_pModules)
                    m_pModules = new std::map<String, NativeModule*>();
            }
            
            static std::map<String, NativeModule*> *m_pModules;
    };
}

#endif
