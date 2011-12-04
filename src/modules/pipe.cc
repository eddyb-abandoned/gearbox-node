#line 1 "src/modules/pipe.gear"

// Copyright Joyent, Inc. and other Node contributors.
//           (c) 2011 the gearbox-node project authors.
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

using namespace Gearbox;

/** \file src/modules/pipe.cc converted from src/modules/pipe.gear */

#line 25 "src/modules/pipe.gear"

#include <UvCommon.h>
#include <cstring>

struct _pipe_exports_Pipe_wrap /*: public Value::DtorWrap*/ {
    uv_pipe_t handle;
    bool unref;
    size_t slabOffset;

    struct This : public Value {
        This(v8::Handle<v8::Object> &&_this, _pipe_exports_Pipe_wrap *wrap) : Value(_this), _wrap(wrap), handle(wrap->handle), unref(wrap->unref), slabOffset(wrap->slabOffset) {
            _this->SetPointerInInternalField(0, wrap);
        }
        This(v8::Handle<v8::Object> &&_this) : Value(_this), _wrap(static_cast<_pipe_exports_Pipe_wrap*>(_this->GetPointerFromInternalField(0))), handle(_wrap->handle), unref(_wrap->unref), slabOffset(_wrap->slabOffset) {}
        _pipe_exports_Pipe_wrap *_wrap;
        uv_pipe_t &handle;
        bool &unref;
        size_t &slabOffset;
    };
};

static v8::Handle<v8::Value> _pipe_exports_Pipe_Pipe(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This(), new _pipe_exports_Pipe_wrap);
    if(args.Length() >= 1) {
        #line 47 "src/modules/pipe.gear"
        Value ipc(args[0]);
        This.unref = false;
        int r = uv_pipe_init(uv_default_loop(), &This.handle, ipc);
        assert(r == 0); // How do we proxy This error up to javascript?
                        // Suggestion: uv_pipe_init() returns void.
        
        //FIXME Will This even work? It would never be cleaned up.
        This.handle.data = new decltype(This)(This);
        Uv::Stream::updateWriteQueueSize(This);
        return undefined;
    }

    #line 38 "src/modules/pipe.gear"
    This.unref = false;
    int r = uv_pipe_init(uv_default_loop(), &This.handle, false);
    assert(r == 0); // How do we proxy This error up to javascript?
    // Suggestion: uv_pipe_init() returns void.
    
    //FIXME Will This even work? It would never be cleaned up.
    This.handle.data = new decltype(This)(This);
    Uv::Stream::updateWriteQueueSize(This);
    return undefined;
}

static v8::Handle<v8::Value> _pipe_exports_Pipe_unref(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This());
    #line 59 "src/modules/pipe.gear"
    Uv::Handle::unref(This);
    return undefined;
}

static v8::Handle<v8::Value> _pipe_exports_Pipe_close(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This());
    #line 61 "src/modules/pipe.gear"
    Uv::Handle::close(This);
    return undefined;
}

static v8::Handle<v8::Value> _pipe_exports_Pipe_open(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This());
    if(args.Length() >= 1) {
        #line 63 "src/modules/pipe.gear"
        Value fd(args[0]);
        uv_pipe_open(&This.handle, fd);
        return undefined;
    }
    THROW_ERROR("Invalid call to pipe.exports.Pipe.prototype.open");
}

static v8::Handle<v8::Value> _pipe_exports_Pipe_shutdown(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This());
    #line 67 "src/modules/pipe.gear"
    std::cerr << "TODO: Pipe shutdown" << std::endl;
    //TODO
    return undefined;
}

static v8::Handle<v8::Value> _pipe_exports_Pipe_readStart(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This());
    #line 72 "src/modules/pipe.gear"
    Uv::Stream::readStart(This);
    return undefined;
}

static v8::Handle<v8::Value> _pipe_exports_Pipe_readStop(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This());
    #line 74 "src/modules/pipe.gear"
    Uv::Stream::readStop(This);
    return undefined;
}

static v8::Handle<v8::Value> _pipe_exports_Pipe_write(const v8::Arguments &args) {
    _pipe_exports_Pipe_wrap::This This(args.This());
    if(args.Length() >= 4) {
        #line 76 "src/modules/pipe.gear"
        Value buffer(args[0]), offset(args[1]), length(args[2]), sendStream(args[3]);
        return Uv::Stream::write(This, buffer, offset, length, sendStream);
    }

    if(args.Length() >= 3) {
        #line 78 "src/modules/pipe.gear"
        Value buffer(args[0]), offset(args[1]), length(args[2]);
        return Uv::Stream::write(This, buffer, offset, length);
    }

    if(args.Length() >= 2) {
        #line 80 "src/modules/pipe.gear"
        Value buffer(args[0]), offset(args[1]);
        return Uv::Stream::write(This, buffer, offset);
    }

    if(args.Length() >= 1) {
        #line 82 "src/modules/pipe.gear"
        Value buffer(args[0]);
        return Uv::Stream::write(This, buffer);
    }
    THROW_ERROR("Invalid call to pipe.exports.Pipe.prototype.write");
}


#line 157 "src/modules/pipe.cc"
static void _setup_pipe(Value exports, Value require, Value module) {
    v8::Handle<v8::FunctionTemplate> _pipe_exports_Pipe = v8::FunctionTemplate::New(_pipe_exports_Pipe_Pipe);
    _pipe_exports_Pipe->SetClassName(String("Pipe"));
    _pipe_exports_Pipe->InstanceTemplate()->SetInternalFieldCount(1);
    _pipe_exports_Pipe->PrototypeTemplate()->Set("unref", Function(_pipe_exports_Pipe_unref, "unref"));
    _pipe_exports_Pipe->PrototypeTemplate()->Set("close", Function(_pipe_exports_Pipe_close, "close"));
    _pipe_exports_Pipe->PrototypeTemplate()->Set("open", Function(_pipe_exports_Pipe_open, "open"));
    _pipe_exports_Pipe->PrototypeTemplate()->Set("shutdown", Function(_pipe_exports_Pipe_shutdown, "shutdown"));
    _pipe_exports_Pipe->PrototypeTemplate()->Set("readStart", Function(_pipe_exports_Pipe_readStart, "readStart"));
    _pipe_exports_Pipe->PrototypeTemplate()->Set("readStop", Function(_pipe_exports_Pipe_readStop, "readStop"));
    _pipe_exports_Pipe->PrototypeTemplate()->Set("write", Function(_pipe_exports_Pipe_write, "write"));
    exports["Pipe"] = _pipe_exports_Pipe->GetFunction();
}
static NativeModule _module_pipe("pipe", _setup_pipe);