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

#ifndef GEARBOX_TRYCATCH_H
#define GEARBOX_TRYCATCH_H

#include <gearbox.h>

#define GEARBOX_TRY_CATCH_REPORT_STACKTRACE 1

namespace Gearbox {
    class TryCatch {
        public:
            TryCatch() : m_bHasLocalException(false), m_pPreviousTryCatch(m_pCurrentTryCatch) {
                m_pCurrentTryCatch = this;
            }
            
            virtual ~TryCatch() {
                if(hasCaught()) {
                    if(m_pPreviousTryCatch) {
                        if(!m_TryCatch.HasCaught()) {
                            m_pPreviousTryCatch->m_bHasLocalException = true;
                            #if GEARBOX_TRY_CATCH_REPORT_STACKTRACE
                                m_pPreviousTryCatch->m_LocalStackTrace = m_LocalStackTrace;
                            #else
                                m_pPreviousTryCatch->m_LocalException = m_LocalException;
                                m_pPreviousTryCatch->m_LocalMessage = m_LocalMessage;
                            #endif
                        }
                        else {
                            m_pPreviousTryCatch->m_bHasLocalException = true;
                            #if GEARBOX_TRY_CATCH_REPORT_STACKTRACE
                                m_pPreviousTryCatch->m_LocalStackTrace = m_TryCatch.StackTrace();
                            #else
                                m_pPreviousTryCatch->m_LocalException = m_TryCatch.Exception();
                                m_pPreviousTryCatch->m_LocalMessage = v8::Persistent<v8::Message>::New(m_TryCatch.Message());
                            #endif
                            if(canThrow())
                                m_TryCatch.ReThrow();
                        }
                    }
                    else
                        reportException();
                }
                m_pCurrentTryCatch = m_pPreviousTryCatch;
            }
            
            bool hasCaught() {
                return m_bHasLocalException || m_TryCatch.HasCaught();
            }
            
            void reportException();
            
            static bool canThrow() {
                return m_bCanThrow;
            }
            
            static bool canThrow(bool bCanThrow) {
                bool bCanThrowBefore = m_bCanThrow;
                m_bCanThrow = bCanThrow;
                return bCanThrowBefore;
            }
            
            static Value _throw(Value exception) {
                if(!canThrow() && m_pCurrentTryCatch) {
                    m_pCurrentTryCatch->m_TryCatch.Reset();
                    m_pCurrentTryCatch->m_bHasLocalException = true;
                    #if GEARBOX_TRY_CATCH_REPORT_STACKTRACE
                        m_pCurrentTryCatch->m_LocalStackTrace = exception;
                    #else
                        m_pCurrentTryCatch->m_LocalException = exception;
                    #endif
                }
                if(canThrow())
                    return v8::ThrowException(exception);
                return undefined;
            }
            
        private:
            v8::TryCatch m_TryCatch;
            bool m_bHasLocalException;
            #if GEARBOX_TRY_CATCH_REPORT_STACKTRACE
                Value m_LocalStackTrace;
            #else
                Value m_LocalException;
                v8::Persistent<v8::Message> m_LocalMessage;
            #endif
            
            TryCatch *m_pPreviousTryCatch;
            
            static bool m_bCanThrow;
            static TryCatch *m_pCurrentTryCatch;
    };
    
    static Value Throw(Value exception) {
        return TryCatch::_throw(exception);
    }
}

#define THROW(x) return Throw(x)
#define THROW_ERROR(x) THROW(Error(x))
#define THROW_RANGE_ERROR(x) THROW(RangeError(x))
#define THROW_REFERENCE_ERROR(x) THROW(ReferenceError(x))
#define THROW_SYNTAX_ERROR(x) THROW(SyntaxError(x))
#define THROW_TYPE_ERROR(x) THROW(TypeError(x))

#endif
