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
    bool TryCatch::m_bCanThrow = false;
    TryCatch *TryCatch::m_pCurrentTryCatch = 0;
    
    void TryCatch::reportException() {
        #if GEARBOX_TRY_CATCH_REPORT_STACKTRACE
            String stackTrace;
            if(!m_TryCatch.HasCaught())
                stackTrace = m_LocalStackTrace;
            else
                stackTrace = Value(m_TryCatch.StackTrace());
            
            std::cerr << stackTrace << std::endl;
            
            m_TryCatch.Reset();
            m_bHasLocalException = false;
            m_LocalStackTrace = undefined;
        #else
            String exception;
            if(!m_TryCatch.HasCaught())
                exception = m_LocalException;
            else
                exception = Value(m_TryCatch.Exception());
            
            v8::Handle<v8::Message> message;
            if(!m_TryCatch.HasCaught())
                message = m_LocalMessage;
            else
                message = m_TryCatch.Message();
            
            if(message.IsEmpty())
                std::cerr << *exception << std::endl;
            else {
                // Print (filename):(line number): (message).
                String filename = Value(message->GetScriptResourceName());
                std::cerr << *filename << ':' << message->GetLineNumber() << ": " << *exception << std::endl;
                
                // Print line of source code.
                String sourceline = Value(message->GetSourceLine());
                std::cerr << *sourceline << std::endl;
                
                // Print wavy underline
                int start = message->GetStartColumn(), end = message->GetEndColumn();
                for(int i = 0; i < start; i++)
                    std::cerr << ' ';
                for(int i = start; i < end; i++)
                    std::cerr << '^';
                std::cerr << std::endl;
            }
            
            m_TryCatch.Reset();
            m_bHasLocalException = false;
            m_LocalException = undefined;
            if(!m_LocalMessage.IsEmpty()) {
                m_LocalMessage.Dispose();
                m_LocalMessage.Clear();
            }
        #endif
    }
}