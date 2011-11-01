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

#include <cstdlib>
#include <iostream>

namespace Gearbox {
    Context *Context::m_pCurrentContext = 0;
    
    static v8::Handle<v8::Value> _exit(const v8::Arguments& args) {
        std::_Exit(Value(args[0]));
        return undefined;
    }
    
    static v8::Handle<v8::Value> _load(const v8::Arguments& args) {
        if(args.Length() >= 1) {
            Value file(args[0]);
            
            TryCatch tryCatch;
            String source = NativeModule::require("Io")["read"](file);
            
            // Report exceptions caught while reading the file
            if(tryCatch.hasCaught())
                return undefined;
            
            Context *pCurrentContext = Context::getCurrent();
            if(!pCurrentContext)
                THROW_ERROR("No Context is in use");
            
            return pCurrentContext->runScript(source, file);
        }
        THROW_ERROR("Invalid call to load");
    }
    
    static v8::Handle<v8::Value> _print(const v8::Arguments& args) {
        if(args.Length()) {
            for(int i = 0; i < args.Length(); i++)
                std::cout << (i ? " " : "") << Value(args[i]).to<String>();
            std::cout << std::endl;
            return undefined;
        }
        THROW_ERROR("Invalid call to print");
    }
    
    Context::Context() {
        // Save the previous context
        m_pPreviousContext = m_pCurrentContext;
        
        // Create the context
        m_hContext = v8::Context::New();
        
        // Enter this context
        m_hContext->Enter();
        
        // We're in this context
        m_pCurrentContext = this;
        
        // Setup the context
        setup();
    }
    
    Context::~Context() {
        // Exit this context
        m_hContext->Exit();
        
        // We're in the previous context
        m_pCurrentContext = m_pPreviousContext;
        
        // Dispose this context
        m_hContext.Dispose();
    }
    
    Value Context::runScript(String source, String name) {
        TryCatch tryCatch;
        
        // Compile the script source
        v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
        
        // Check for any errors that could have happened at compile time
        if(script.IsEmpty() || tryCatch.hasCaught())
            return undefined;
        
        // Exceptions can be thrown, we are inside JavaScript
        bool bCanThrowBefore = TryCatch::canThrow(true);
        
        // Run the script and get the result
        var result = script->Run();
        
        // We are back from JavaScript
        TryCatch::canThrow(bCanThrowBefore);
        
        // Return the result
        return result;
    }
    
    void Context::setup() {
        // Get the global object
        var _global = global();
        
        _global["exit"] = Function(_exit, "exit");
        _global["load"] = Function(_load, "load");
        _global["print"] = Function(_print, "print");
        _global["require"] = NativeModule::getRequireFunc();
        
        _global["global"] = _global;
    }
}
