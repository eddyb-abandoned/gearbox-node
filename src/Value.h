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

#ifndef GEARBOX_VALUE_H
#define GEARBOX_VALUE_H

#include <gearbox.h>

#include <iostream>

namespace Gearbox {
    
    class Primitive {
        public:
            enum Kind {
                Undefined=0,
                Null,
                False,
                True,
                Integer,
                Number
            };
            Primitive(Kind kind=Undefined, int64_t iValue=0) : m_Kind(kind), m_iValue(0), m_dValue(0) {
                if(kind == Integer)
                    m_iValue = iValue;
                else if(kind == Number)
                    m_dValue = iValue;
            }
            Primitive(Kind kind, double dValue) : m_Kind(kind), m_iValue(0), m_dValue(0) {
                if(kind == Integer)
                    m_iValue = dValue;
                else if(kind == Number)
                    m_dValue = dValue;
            }
            
            operator v8::Handle<v8::Value>() const {
                if(m_Kind == Undefined)
                    return v8::Undefined();
                else if(m_Kind == Null)
                    return v8::Null();
                else if(m_Kind == False)
                    return v8::False();
                else if(m_Kind == True)
                    return v8::True();
                else if(m_Kind == Integer) {
                    if(m_iValue >= 0 && m_iValue <= 0xffffffffL)
                        return v8::Integer::NewFromUnsigned(m_iValue);
                    else if(m_iValue >= -0x80000000L && m_iValue <= 0x7fffffffL)
                        return v8::Integer::New(m_iValue);
                    else
                        return v8::Number::New(m_iValue);
                }
                else if(m_Kind == Number)
                    return v8::Number::New(m_dValue);
                return v8::Undefined();
            }
            
            bool from(const v8::Handle<v8::Value> &hValue) {
                if(hValue.IsEmpty() || hValue->IsUndefined())
                    m_Kind = Undefined;
                else if(hValue->IsNumber()) {
                    m_Kind = Number;
                    m_dValue = hValue->NumberValue();
                }
                else if(hValue->IsUint32() || hValue->IsInt32()) {
                    m_Kind = Integer;
                    m_iValue = hValue->IntegerValue();
                }
                else if(hValue->IsBoolean())
                    m_Kind = hValue->BooleanValue() ? True : False;
                else if(hValue->IsNull())
                    m_Kind = Null;
                else 
                    return true;
                return false;
            }
            
            bool operator==(Primitive that) {
                if(m_Kind <= Null)
                    return m_Kind == that.m_Kind;
                return operator v8::Handle<v8::Value>()->Equals(that);
            }
            
            bool operator!=(Primitive that) {
                return !operator==(that);
            }
            
        private:
            Kind m_Kind;
            int64_t m_iValue;
            double m_dValue;
            friend class Value;
    };
    
    const Primitive undefined = Primitive::Undefined;
    const Primitive null = Primitive::Null;
    
    template <class Node, class Index>
    class Assignable : public Node {
        public:
            Assignable(const Node &parent, const Index &index) : m_Parent(parent)/*, m_Index(index)*/ {
                //m_Parent = parent;
                m_Index = index;
                Node::from(m_Parent.get(m_Index));
            }
            template <class T>
            Node &operator=(const T &_that) {
                Node that(_that);
                m_Parent.set(m_Index, that);
                return *this;
            }
            template <class T>
            Node &operator=(const T &&_that) {
                Node that(_that);
                m_Parent.set(m_Index, that);
                return *this;
            }
            Node &operator=(const Node &that) {
                m_Parent.set(m_Index, that);
                return *this;
            }
            Node &operator=(const Node &&that) {
                m_Parent.set(m_Index, that);
                return *this;
            }
            /*Node &operator=(const Assignable &that) {
                m_Parent.set(m_Index, that);
                return *this;
            }
            Node &operator=(const Assignable &&that) {
                m_Parent.set(m_Index, that);
                return *this;
            }*/
            template <class... Args>
            Node operator()(Args... _args) const {
                return Node::call(m_Parent, _args...);
            }
            
        private:
            const Node &m_Parent;
            Index m_Index;
    };
    
    /** A class for every kind of JavaScript value (Objects, Arrays, Functions, Numbers, Strings) */
    class Value {
        public:
            /** Default constructor */
            Value() : m_bIsPersistent(true) {}
            /** Constructors */
            template <class T>
            Value(const T &that) : m_bIsPersistent(true) {
                from(that);
            }
            template <class T>
            Value(const T &&that) : m_bIsPersistent(true) {
                from(that);
            }
            Value(Value &&that) : m_bIsPersistent(that.m_bIsPersistent) {
                if(that.m_hValue.IsEmpty())
                    from(that.m_pValue);
                else {
                    m_hValue = that.m_hValue;
                    that.m_hValue.Clear();
                }
            }
            
            //HACK These are here until we have some sort of ConstValue or Arguments class.
            // The reason is that we shouldn't call Persistent::New or attempt to turn the
            // v8 Value into a Primitive, because it's not stored anywhere.
            Value(const v8::Local<v8::Value> &that) : m_bIsPersistent(false), m_hValue(that) {}
            Value(const v8::Local<v8::Value> &&that) : m_bIsPersistent(false), m_hValue(that) {}
            
            /** Default destructor */
            virtual ~Value() {
                if(m_bIsPersistent && !m_hValue.IsEmpty())
                    v8::Persistent<v8::Value>(m_hValue).MakeWeak(0, weakCallback);
            }
            
            /** Copy operators */
            template <class T>
            Value &operator=(const T &that) {
                if(m_bIsPersistent && !m_hValue.IsEmpty()) {
                    v8::Persistent<v8::Value>(m_hValue).MakeWeak(0, weakCallback);
                    m_hValue.Clear();
                }
                from(that);
                return *this;
            }
            template <class T>
            Value &operator=(const T &&that) {
                if(m_bIsPersistent && !m_hValue.IsEmpty()) {
                    v8::Persistent<v8::Value>(m_hValue).MakeWeak(0, weakCallback);
                    m_hValue.Clear();
                }
                from(that);
                return *this;
            }
            Value &operator=(Value &&that) {
                if(m_bIsPersistent && !m_hValue.IsEmpty()) {
                    v8::Persistent<v8::Value>(m_hValue).MakeWeak(0, weakCallback);
                    m_hValue.Clear();
                }
                if(that.m_hValue.IsEmpty())
                    from(that.m_pValue);
                else {
                    m_hValue = that.m_hValue;
                    that.m_hValue.Clear();
                }
                return *this;
            }
            
            /** Instantiation tools */
            void from(const Value &that) {
                if(that.m_hValue.IsEmpty())
                    from(that.m_pValue);
                else
                    from(that.m_hValue);
            }
            void from(const v8::Handle<v8::Value> &that) {
                if(m_pValue.from(that))
                    m_hValue = v8::Persistent<v8::Value>::New(that);
                /*if(that.IsEmpty() || that->IsUndefined())
                    from(undefined);
                else if(!that->IsUint32() || that->IsInt32() ||  that->IsNumber() || that->IsUndefined() || that->IsNull() || that->IsBoolean())
                    m_pValue.from(that);
                else
                    m_hValue = v8::Persistent<v8::Value>::New(that);*/
            }
            template <class T>
            void from(const v8::Handle<T> &that) {
                from(v8::Handle<v8::Value>(that));
            }
            void from(const String &that) {
                from(that.operator v8::Handle<v8::Value>());
            }
            void from(const Primitive &that) {
                m_pValue = that;
            }
            void from(const char *that) {
                from(v8::String::New(that));
            }
            void from(char *that) {
                from(v8::String::New(that));
            }
#ifdef __LP64__
            void from(unsigned long long int that) {
                from(Primitive(Primitive::Integer, int64_t(that)));
            }
            void from(long long int that) {
                from(Primitive(Primitive::Integer, int64_t(that)));
            }
#endif
            void from(uint64_t that) {
                from(Primitive(Primitive::Integer, int64_t(that)));
            }
            void from(int64_t that) {
                from(Primitive(Primitive::Integer, that));
            }
            void from(int that) {
                from(int64_t(that));
            }
            void from(unsigned int that) {
                from(int64_t(that));
            }
            void from(double that) {
                from(Primitive(Primitive::Number, that));
            }
            void from(bool that) {
                from(Primitive(that ? Primitive::True : Primitive::False));
            }
            template <class T>
            void from(T *that)=delete;/* {
                //static_assert(sizeof(T)<0, "Value::from<T*> is deprecated!");
                from(v8::External::New(that));
            }*/
            
            /** Conversion tools, used to get primitive values */
            template <class T>
            T to() const {
                return to(Type<T>());
            }
            
#define _TO(T) T to(Type<T>) const
            template <class T> _TO(T) {
                static_assert(sizeof(T)<0, "Value::to<T> called with no specialization for T!");
            }
            
            _TO(v8::Handle<v8::Value>) {
                if(m_hValue.IsEmpty())
                    return m_pValue;
                else
                    return m_hValue;
            }
            _TO(v8::Handle<v8::Data>) {
                return to<v8::Handle<v8::Value>>();
            }
            template <class T> _TO(v8::Handle<T>) {
                return v8::Handle<T>::Cast(to<v8::Handle<v8::Value>>());
            }
            _TO(Primitive) {
                if(m_hValue.IsEmpty())
                    return m_pValue;
                Primitive value;
                value.from(m_hValue);
                return value;
            }
            _TO(String) {
                if(m_hValue.IsEmpty()) {
                    v8::String::Utf8Value v(m_pValue.operator v8::Handle<v8::Value>());
                    return String(*v);
                }
                v8::String::Utf8Value v(m_hValue);
                return String(*v);
            }
            _TO(int64_t) {
                if(m_hValue.IsEmpty()) {
                    if(m_pValue.m_Kind == Primitive::Integer)return m_pValue.m_iValue;
                    else if(m_pValue.m_Kind == Primitive::Number)return m_pValue.m_dValue;
                    else if(m_pValue.m_Kind == Primitive::True)return 1;
                    return 0;
                }
                return m_hValue->IntegerValue();
            }
#ifdef __LP64__
            _TO(long long int) {
                return to<int64_t>();
            }
            _TO(unsigned long long int) {
                return to<int64_t>();
            }
#endif
            _TO(uint64_t) {
                return to<int64_t>();
            }
            _TO(uint32_t) {
                return to<int64_t>();
            }
            _TO(uint16_t) {
                return to<int64_t>();
            }
            _TO(int16_t) {
                return to<int64_t>();
            }
            _TO(char) {
                return to<int64_t>();
            }
            _TO(uint8_t) {
                return to<int64_t>();
            }
            _TO(int8_t) {
                return to<int64_t>();
            }
            _TO(int) {
                return to<int64_t>();
            }
            _TO(double) {
                if(m_hValue.IsEmpty()) {
                    if(m_pValue.m_Kind == Primitive::Number)return m_pValue.m_dValue;
                    else if(m_pValue.m_Kind == Primitive::Integer)return m_pValue.m_iValue;
                    else if(m_pValue.m_Kind == Primitive::True)return 1;
                    return 0;
                }
                return m_hValue->NumberValue();
            }
            _TO(float) {
                return to<double>();
            }
            long double to(Type<long double>) const {
                return to<double>();
            }
            _TO(bool) {
                if(m_hValue.IsEmpty()) {
                    if(m_pValue.m_Kind == Primitive::True)return true;
                    else if(m_pValue.m_Kind == Primitive::Integer)return m_pValue.m_iValue;
                    else if(m_pValue.m_Kind == Primitive::Number)return m_pValue.m_dValue;
                    return false;
                }
                return m_hValue->BooleanValue();
            }
            
            template <class T>
            _TO(T*) {
                static_assert(sizeof(T)<0, "Value::to<T*> is deprecated!");
                /*if(m_hValue.IsEmpty() || !m_hValue->IsExternal() || !v8::External::Unwrap(m_hValue)) {
                    std::cerr << "Empty/NULL External!" << std::endl;
                    return 0;
                }
                return reinterpret_cast<T*>(v8::External::Unwrap(m_hValue));*/
            }
#undef _TO
            
            /** Compare operators */
            bool operator==(Value that) {
                if(m_hValue.IsEmpty() && that.m_hValue.IsEmpty())
                    return m_pValue == that.m_pValue;
                else
                    return to<v8::Handle<v8::Value>>()->Equals(that);
            }
            bool operator!=(Value that) {
                return !operator==(that);
            }
            
            /** Length, for Arrays and Strings */
            int64_t length() const {
                if(m_hValue.IsEmpty())
                    return 0;
                if(m_hValue->IsArray())
                    return v8::Handle<v8::Array>::Cast(m_hValue)->Length();
                if(m_hValue->IsString())
                    return m_hValue->ToString()->Length();
                return 0;
            }
            
            /** Access to Object or Array elements */
            Assignable<Value, uint32_t> operator[](uint32_t idx) const {
                return Assignable<Value, uint32_t>(*this, idx);
            }
            Assignable<Value, uint32_t> operator[](int32_t idx) const {
                return Assignable<Value, uint32_t>(*this, idx);
            }
            Assignable<Value, uint32_t> operator[](size_t idx) const {
                return Assignable<Value, uint32_t>(*this, idx);
            }
            Assignable<Value, v8::Handle<v8::String>> operator[](const char *idx) const {
                return Assignable<Value, v8::Handle<v8::String>>(*this, v8::String::NewSymbol(idx));
            }
            Assignable<Value, v8::Handle<v8::String>> operator[](const String &idx) const {
                return Assignable<Value, v8::Handle<v8::String>>(*this, v8::String::NewSymbol(idx, idx.length()));
            }
            Value get(uint32_t idx) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsObject())
                    return undefined;
                return m_hValue->ToObject()->Get(idx);
            }
            Value get(const String &idx) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsObject())
                    return undefined;
                return m_hValue->ToObject()->Get(idx.operator v8::Handle<v8::Value>());
            }
            Value get(const v8::Handle<v8::String> &idx) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsObject())
                    return undefined;
                return m_hValue->ToObject()->Get(idx);
            }
            void set(uint32_t idx, const Value &val) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsObject())
                    return;
                m_hValue->ToObject()->Set(idx, val);
            }
            void set(const String &idx, const Value &val) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsObject())
                    return;
                m_hValue->ToObject()->Set(idx.operator v8::Handle<v8::Value>(), val);
            }
            void set(const v8::Handle<v8::String> &idx, const Value &val) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsObject())
                    return;
                m_hValue->ToObject()->Set(idx, val);
            }
            
            /** Returns true if this Value is an instance of class T */
            template <class T>
            bool is() const {
                return T::is(*this);
            }
            
            /** Convert operator */
            template <class T>
            operator T() const {
                return to<T>();
            }
            
            /** Call operator for Functions */
            template <class... Args>
            Value operator()(Args... _args) const {
                return call(v8::Context::GetCurrent()->Global(), _args...);
            }
            
            template <class... Args>
            Value call(Value _this, Args... _args) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsFunction())
                    return undefined;
                
                v8::Handle<v8::Value> args[sizeof...(_args)];
                placeArgs(args, _args...);
                
                // Exceptions can be thrown, we are inside JavaScript.
                bool bCanThrowBefore = tryCatchCanThrow(true);
                
                Value result = v8::Handle<v8::Function>::Cast(m_hValue)->Call(_this, sizeof...(_args), args);
                
                // We are back from JavaScript.
                tryCatchCanThrow(bCanThrowBefore);
                
                return result;
            }
            
            /** New Instance for Functions */
            template <class... Args>
            Value newInstance(Args... _args) const {
                if(m_hValue.IsEmpty() || !m_hValue->IsFunction())
                    return undefined;
                
                v8::Handle<v8::Value> args[sizeof...(_args)];
                placeArgs(args, _args...);
                
                // Exceptions can be thrown, we are inside JavaScript.
                bool bCanThrowBefore = tryCatchCanThrow(true);
                
                Value result = v8::Handle<v8::Function>::Cast(m_hValue)->NewInstance(sizeof...(_args), args);
                
                // We are back from JavaScript.
                tryCatchCanThrow(bCanThrowBefore);
                
                return result;
            }
            
        private:
            template <class... Last>
            static void placeArgs(v8::Handle<v8::Value> *pValues, Value first, Last... last) {
                *pValues = first;
                placeArgs(pValues + 1, last...);
            }
            
            static void placeArgs(v8::Handle<v8::Value> *pValues) {}
            
            /// FIXME Hack to get TryCatch::canThrow into call and newInstance.
            static bool tryCatchCanThrow(bool);
            
            static void weakCallback(v8::Persistent<v8::Value> that, void*) {
                //if(/*that->IsExternal() || */that->ToObject()->HasIndexedPropertiesInExternalArrayData() && that->ToObject()->InternalFieldCount())
                //    delete [] (uint8_t*)that->ToObject()->GetIndexedPropertiesExternalArrayData();
                //    printf("TODO: need to delete user-related stuff on disposal\n");
                that.Dispose();
            }
            
            const bool m_bIsPersistent;
            v8::Handle<v8::Value> m_hValue;
            Primitive m_pValue;
            //v8::Persistent<v8::Value> m_hValue;
    };
}

#include "TryCatch.h"

namespace Gearbox {
    inline bool Value::tryCatchCanThrow(bool bCanThrow) {
        return TryCatch::canThrow(bCanThrow);
    }
}

#endif
