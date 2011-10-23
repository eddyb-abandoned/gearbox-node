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

#ifndef GEARBOX_H
#define GEARBOX_H

#include <v8.h>
#include "shell.h"

namespace Gearbox {
    
    /** Structure that wraps a type as a template parameter */
    template <class T>
    class Type {};
    
}

#include "String.h"
#include "Value.h"

#include "Context.h"
#include "Module.h"
#include "TryCatch.h"

namespace Gearbox {
    
    static void PrintTrace() { 
        v8::Message::PrintCurrentStackTrace(stdout);
    }

    static Primitive Integer(int64_t val) {
        return Primitive(Primitive::Integer, val);
    }
    
    static Primitive Number(double val) {
        return Primitive(Primitive::Number, val);
    }
    
    class Object : public Value {
        public:
            Object() : Value(v8::Object::New()) {}
            static bool is(Value &that) {
                return that.to<v8::Handle<v8::Value>>()->IsObject();
            }
    };
    
    class Array : public Value {
        public:
            Array(int length=0) : Value(v8::Array::New(length)) {}
            static bool is(Value &that) {
                return that.to<v8::Handle<v8::Value>>()->IsArray();
            }
            template <typename T>
            static T** get(Value &that) {
                if(!is(that))
                    return 0;
                size_t len = that.length();
                T **array = new T* [len];
                for(size_t i = 0; i < len; i++)
                    array[i] = that[uint32_t(i)].to<T*>();
                return array;
            }
    };
    
    static Value Function(v8::InvocationCallback _function, String name) {
        v8::Handle<v8::Function> function = v8::FunctionTemplate::New(_function)->GetFunction();
        function->SetName(name);
        return function;
    }
    
#define _DEF_ERROR(x) static Value x(String message) {return v8::Exception:: x(message);}
    _DEF_ERROR(Error)
    _DEF_ERROR(RangeError)
    _DEF_ERROR(ReferenceError)
    _DEF_ERROR(SyntaxError)
    _DEF_ERROR(TypeError)
#undef _DEF_ERROR
    
    template <class T>
    static Value Internal(T that) {
        return Value(that, Value::Internal);
    }
    
    typedef Value var;
    
}

#endif
