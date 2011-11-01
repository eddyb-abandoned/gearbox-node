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

#include <gearbox.h>

namespace Gearbox {
    std::map<String, NativeModule*> *NativeModule::m_pModules = 0;
    Value NativeModule::m_RequireFunc;
    
    NativeModule::NativeModule(String moduleName, NativeModule::SetupCallback pSetupCallback) : m_sModuleName(moduleName), m_pSetupCallback(pSetupCallback) {
        if(!NativeModule::m_pModules)
            NativeModule::m_pModules = new std::map<String, NativeModule*>();
        (*m_pModules)[moduleName] = this;
    }
    
    NativeModule::~NativeModule() {
        if(!NativeModule::m_pModules)
            NativeModule::m_pModules = new std::map<String, NativeModule*>(); // Doesn't make too much sense, does it?
        m_pModules->erase(m_sModuleName);
    }
    
    Value NativeModule::_require(Value requireFunc) {
        if(requireFunc == undefined)
            requireFunc = getRequireFunc();
        if(m_oModule == undefined) {
            m_oModule = Object();
            var exports = Object();
            m_oModule["exports"] = exports;
            m_pSetupCallback(exports, requireFunc, m_oModule);
        }
        return m_oModule["exports"];
    }
    
    Value NativeModule::require(String moduleName, Value requireFunc) {
        if(!NativeModule::m_pModules)
            NativeModule::m_pModules = new std::map<String, NativeModule*>();
        
        if(!m_pModules->count(moduleName))
            THROW_ERROR("NativeModule "+moduleName+" doesn't exist");
        
        return (*m_pModules)[moduleName]->_require(requireFunc);
    }
    
    Value NativeModule::getRequireFunc() {
        if(m_RequireFunc == undefined)
            m_RequireFunc = Function(require, "require");
        return m_RequireFunc;
    }
    
    v8::Handle<v8::Value> NativeModule::require(const v8::Arguments& args) {
        if(args.Length() >= 1)
            return require(Value(args[0]).to<String>());
        THROW_ERROR("Invalid call to NativeModule::require");
    }

}
