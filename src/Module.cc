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
    std::map<String, Module*> *Module::m_pModules = 0;
    
    Module::Module(String moduleName, Module::SetupCallback pSetupCallback) : m_sModuleName(moduleName), m_pSetupCallback(pSetupCallback) {
        if(!Module::m_pModules)
            Module::m_pModules = new std::map<String, Module*>();
        (*m_pModules)[moduleName] = this;
    }
    
    Module::~Module() {
        if(!Module::m_pModules)
            Module::m_pModules = new std::map<String, Module*>();
        m_pModules->erase(m_sModuleName);
    }
    
    Value Module::require() {
        if(m_Exports == undefined) {
            m_Exports = Object();
            Context moduleContext;
            moduleContext.global()["exports"] = m_Exports;
            m_pSetupCallback(m_Exports);
        }
        return m_Exports;
    }
    
    Value Module::require(String moduleName) {
        if(moduleName[0] == '.' || moduleName[0] == '/')
            return Throw(Error("Can't load relative/absolute path module"));
        
        if(!Module::m_pModules)
            Module::m_pModules = new std::map<String, Module*>();
        
        if(!m_pModules->count(moduleName))
            return Throw(Error("Module isn't already loaded"));
        
        return (*m_pModules)[moduleName]->require();
    }

}
