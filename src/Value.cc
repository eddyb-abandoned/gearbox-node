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
    
    void Value::from(v8::Handle<v8::Value> that) {
        if(that.IsEmpty())
            from(undefined);
        else if(that->IsNumber() || that->IsUint32() || that->IsInt32() ||  that->IsUndefined() || that->IsNull() || that->IsBoolean()) {
            m_hValue.Clear();
            m_pValue.from(that);
        }
        else
            m_hValue = v8::Persistent<v8::Value>::New(that);
    }
    
    void Value::from(Primitive that) {
        m_hValue.Clear();
        m_pValue = that;
    }
    
    String Value::to(Type<String>) const {
        //if(m_hValue.IsEmpty())
        //    return *v8::String::Utf8Value(m_pValue.operator v8::Handle<v8::Value>());
        //return *v8::String::Utf8Value(m_hValue);
        if(m_hValue.IsEmpty()) {
            v8::String::Utf8Value v(m_pValue.operator v8::Handle<v8::Value>());
            return String(*v);
        }
        v8::String::Utf8Value v(m_hValue);
        return String(*v);
        //v8::String::Utf8Value stringValue(m_hValue.IsEmpty() ? m_pValue.operator v8::Handle<v8::Value>() : m_hValue);
        //return String(*stringValue /*? *stringValue : "<string conversion failed>"*/);
    }
    
    int64_t Value::to(Type<int64_t>) const {
        if(m_hValue.IsEmpty()) {
            if(m_pValue.m_Kind == Primitive::Number)return m_pValue.m_dValue;
            else if(m_pValue.m_Kind == Primitive::True)return 1;
            else if(m_pValue.m_Kind == Primitive::Integer)return m_pValue.m_iValue;
            return 0;
        }
        return m_hValue->IntegerValue();
    }
    
    double Value::to(Type<double>) const {
        if(m_hValue.IsEmpty()) {
            if(m_pValue.m_Kind == Primitive::Number)return m_pValue.m_dValue;
            else if(m_pValue.m_Kind == Primitive::True)return 1;
            else if(m_pValue.m_Kind == Primitive::Integer)return m_pValue.m_iValue;
            return 0;
        }
        return m_hValue->NumberValue();
    }
    
    bool Value::to(Type<bool>) const {
        if(m_hValue.IsEmpty()) {
            if(m_pValue.m_Kind == Primitive::Number)return m_pValue.m_dValue;
            else if(m_pValue.m_Kind == Primitive::True)return true;
            else if(m_pValue.m_Kind == Primitive::Integer)return m_pValue.m_iValue;
            return false;
        }
        return m_hValue->BooleanValue();
    }
    
    v8::Handle<v8::Value> Value::to(Type<v8::Handle<v8::Value>>) const {
        if(m_hValue.IsEmpty())
            return m_pValue;
        else
            return m_hValue;
    }
    
    int Value::length() const {
        if(m_hValue.IsEmpty())
            return 0;
        if(m_hValue->IsArray())
            return v8::Handle<v8::Array>::Cast(m_hValue)->Length();
        if(m_hValue->IsString())
            return m_hValue->ToString()->Length();
        return 0;
    }
    
    void Value::weakCallback(v8::Persistent<v8::Value> that, void*) {
        //if(that->IsExternal() || that->ToObject()->HasIndexedPropertiesInExternalArrayData())
         //   printf("TODO: need to delete user-related stuff on disposal\n");
        that.Dispose();
    }
}