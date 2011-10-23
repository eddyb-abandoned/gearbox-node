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

#ifndef GEARBOX_CONTEXT_H
#define GEARBOX_CONTEXT_H

#include <gearbox.h>

namespace Gearbox {
    class Context {
        public:
            Context();
            
            virtual ~Context();
            
            Value global() {
                return m_hContext->Global();
            }
            
            Value runScript(String source, String name);
            
            static Context *getCurrent() {
                return m_pCurrentContext;
            }
            
        private:
            virtual void setup();
            
            v8::Persistent<v8::Context> m_hContext;
            
            Context *m_pPreviousContext;
            
            static Context *m_pCurrentContext;
    };
}

#endif
