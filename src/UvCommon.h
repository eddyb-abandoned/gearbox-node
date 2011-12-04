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

#ifndef GEARBOX_UV_COMMON_H
#define GEARBOX_UV_COMMON_H

#include <assert.h>
#include <gearbox.h>
#include <modules/buffer.h>
#include <uv.h>

namespace Uv {
    using namespace Gearbox;
    
    template <typename T>
    struct Req : public Object {
        Req() {
            handle.data = this;
        }
        
        T handle;
        void *data_;
    };
    
    template <typename T>
    inline T *unwrapHandle(const v8::Handle<v8::Object> obj) {
        assert(obj->InternalFieldCount() > 0);
        return static_cast<T*>(obj->GetPointerFromInternalField(0));
    }
    
    /// Provides generic functions that work on Uv Handles.
    class Handle {
        /// The Uv Handle.
        uv_handle_t handle;
    public:
#define HANDLE (reinterpret_cast<uv_handle_t*>(&_this.handle))
#define TPL_sT template<typename This> static
        
        // HACK All this is to filter uv_timer_t so we don't need to implement it in timers.gear
        TPL_sT void stateChange(This &_this, uv_timer_t *handle) {
            bool bWasActive = _this.active;
            _this.active = uv_is_active(reinterpret_cast<uv_handle_t*>(handle));
            
            if(!bWasActive && _this.active) {
                // If our state is changing from inactive to active, we
                // increase the loop's reference count.
                uv_ref(uv_default_loop());
            } else if(bWasActive && !_this.active) {
                // If our state is changing from active to inactive, we
                // decrease the loop's reference count.
                uv_unref(uv_default_loop());
            }
        }
        TPL_sT void stateChange(This &_this, void *handle) {}
        TPL_sT void stateChange(This &_this) {
            stateChange(_this, &_this.handle);
        }

        TPL_sT void unref(This &_this) {
            assert(_this.unref == false);
            _this.unref = true;
            
            uv_unref(uv_default_loop());
        }
        
        TPL_sT void close(This &_this) {
            uv_close(HANDLE, onClose);
            
            if(_this.unref) {
                uv_ref(uv_default_loop());
                _this.unref = false;
            }
            
            stateChange(_this);
        }
        
    private:
        static void onClose(uv_handle_t* handle) {
            /*HandleWrap* wrap = static_cast<HandleWrap*>(handle->data);
            
            // The wrap object should still be there.
            assert(wrap->object_.IsEmpty() == false);
            
            wrap->object_->SetPointerInInternalField(0, NULL);
            wrap->object_.Dispose();
            wrap->object_.Clear();
            
            delete wrap;*/ //FIXME proper closing of handles
        }
        
#undef TPL_sT
#undef HANDLE
    };
    
    /// Provides generic functions that work on Uv Streams.
    class Stream {
        typedef Req<uv_shutdown_t> ShutdownReq;
        typedef Req<uv_write_t> WriteReq;
        
        /// The Uv Stream handle.
        /// \note The uv_stream_t or its inheritant should always come first.
        uv_stream_t handle;
        size_t slabOffset;
        
    public:
#define HANDLE (reinterpret_cast<uv_stream_t*>(&_this.handle))
#define TPL_sT template<typename This> static
#define SLAB_SIZE (1024 * 1024)

        TPL_sT Value readStart(This &_this) {
            bool bIpcPipe = HANDLE->type == UV_NAMED_PIPE && reinterpret_cast<uv_pipe_t*>(HANDLE)->ipc;
            
            int r;
            if(bIpcPipe)
                r = uv_read2_start(HANDLE, &onAlloc<This>, [](uv_pipe_t* handle, ssize_t nRead, uv_buf_t buf, uv_handle_type pending) {
                    onRead<This>(reinterpret_cast<uv_stream_t*>(handle), nRead, buf, pending);
                });
            else
                r = uv_read_start(HANDLE, &onAlloc<This>, [](uv_stream_t *handle, ssize_t nRead, uv_buf_t buf) {
                    onRead<This>(handle, nRead, buf, UV_UNKNOWN_HANDLE);
                });
            
            //if (r) SetErrno(uv_last_error(uv_default_loop())); //FIXME
            
            return r;
        }
        TPL_sT Value readStop(This &_this) {
            int r = uv_read_stop(HANDLE);
            
            //if (r) SetErrno(uv_last_error(uv_default_loop())); //FIXME
                
            return r;
        }

        static void updateWriteQueueSize(Value &_this, uv_stream_t *handle) {
            _this["writeQueueSize"] = handle->write_queue_size;
        }
        TPL_sT void updateWriteQueueSize(This &_this) {
            updateWriteQueueSize(_this, HANDLE);
        }
        
        TPL_sT Value write(This &_this, Value &buffer, size_t offset, size_t length, Value sendStream=undefined) {
            assert(buffer.is<Buffer>());
            bool bIpcPipe = HANDLE->type == UV_NAMED_PIPE && reinterpret_cast<uv_pipe_t*>(HANDLE)->ipc;
            
            WriteReq *req = new WriteReq();
            
            (*req)["buffer"] = buffer;
            
            uv_buf_t buf;
            buf.base = reinterpret_cast<char*>(Buffer::data(buffer) + offset);
            buf.len = length;
            
            int r;
            
            if (!bIpcPipe)
                r = uv_write(&req->handle, HANDLE, &buf, 1, &afterWrite);
            else {
                uv_stream_t *sendStreamPtr = NULL;
                
                if(sendStream.is<Object>())
                    sendStreamPtr = unwrapHandle<uv_stream_t>(sendStream);
                
                r = uv_write2(&req->handle, HANDLE, &buf, 1, sendStreamPtr, &afterWrite);
            }
            
            updateWriteQueueSize(_this);
            
            if(r) {
                //SetErrno(uv_last_error(uv_default_loop())); //FIXME
                delete req;
                return null;
            }
            return *req;
        }
        TPL_sT Value write(This &_this, Value &buffer, size_t offset=0) {
            return write(_this, buffer, offset, Buffer::length(buffer));
        }
        
    private:
        static inline uint8_t *newSlab() {
            m_Slab =  Buffer(SLAB_SIZE);
            assert(Buffer::length(m_Slab) == SLAB_SIZE);
            m_nSlabUsed = 0;
            return Buffer::data(m_Slab);
        }
        TPL_sT uv_buf_t onAlloc(uv_handle_t *handle, size_t suggestedSize) {
            This &_this = *static_cast<This*>(handle->data);
            
            uint8_t *slab = NULL;
            
            // No slab currently. Create a new one.
            if(m_Slab == undefined)
                slab = newSlab();
            // If less than 64kb is remaining on the slab allocate a new one.
            else if(SLAB_SIZE - m_nSlabUsed < 64 * 1024)
                slab = newSlab();
            else {
                // Use existing slab.
                slab = Buffer::data(m_Slab);
                assert(Buffer::length(m_Slab) == SLAB_SIZE);
                assert(SLAB_SIZE >= m_nSlabUsed);
            }
            //wrap->object_->SetHiddenValue(slab_sym, slab_obj);
            _this["slab"] = m_Slab;
            
            uv_buf_t buf;
            buf.base = reinterpret_cast<char*>(slab + m_nSlabUsed);
            buf.len = MIN(SLAB_SIZE - m_nSlabUsed, suggestedSize);
            
            _this.slabOffset = m_nSlabUsed;
            m_nSlabUsed += buf.len;
            
            m_HandleThatLastAlloced = reinterpret_cast<uv_stream_t*>(handle);
            
            return buf;
        }
        
        TPL_sT void onRead(uv_stream_t *handle, ssize_t nRead, uv_buf_t buf, uv_handle_type pending) {
            // Remove the reference to the slab to avoid memory leaks;
            This &_this = *static_cast<This*>(handle->data);
            var slab = _this["slab"];
            _this["slab"] = null;
            //Local<Value> slab_v = wrap->object_->GetHiddenValue(slab_sym);
            //wrap->object_->SetHiddenValue(slab_sym, v8::Null());
            
            if(nRead < 0)  {
                // EOF or Error
                if(m_HandleThatLastAlloced == handle)
                    m_nSlabUsed -= buf.len;
                
                //SetErrno(uv_last_error(uv_default_loop())); //FIXME
                //MakeCallback(wrap->object_, "onread", 0, NULL); //FIXME makeCallback
                _this["onread"]();
                return;
            }
            
            assert(nRead <= buf.len);
            
            if(m_HandleThatLastAlloced == handle)
                m_nSlabUsed -= (buf.len - nRead);
            
            if(nRead > 0) {
                /*int argc = 3;
                Local<Value> argv[4] = {
                    slab_v,
                    Integer::New(wrap->slab_offset_),
                    Integer::New(nread)
                };*/
                
                
                if(pending == UV_TCP) {
                    std::cerr<<"TODO: UV_TCP onRead pending"<<std::endl;
                    /*
                    // Instantiate the client javascript object and handle.
                    Local<Object> pending_obj = TCPWrap::Instantiate();
                    
                    // Unwrap the client javascript object.
                    assert(pending_obj->InternalFieldCount() > 0);
                    TCPWrap* pending_wrap =
                    static_cast<TCPWrap*>(pending_obj->GetPointerFromInternalField(0));
                    
                    int r = uv_accept(handle, pending_wrap->GetStream());
                    assert(r == 0);
                    
                    argv[3] = pending_obj;
                    argc++;*/
                } else
                    // We only support sending UV_TCP right now.
                    assert(pending == UV_UNKNOWN_HANDLE);
                
                _this["onread"](slab, _this.slabOffset, nRead);
                //MakeCallback(wrap->object_, "onread", argc, argv); //FIXME makeCallback
            }
        }
        static void afterWrite(uv_write_t *handle, int status) {
            WriteReq *req = static_cast<WriteReq*>(handle->data);
            Value &_this = *static_cast<Value*>(handle->handle->data);
            //if(status)
            //  SetErrno(uv_last_error(uv_default_loop())); //FIXME

            updateWriteQueueSize(_this, handle->handle);

            /*Local<Value> argv[4] = {
                Integer::New(status),
                Local<Value>::New(wrap->object_),
                Local<Value>::New(req_wrap->object_),
                req_wrap->object_->GetHiddenValue(buffer_sym),
            };
            MakeCallback(req_wrap->object_, "oncomplete", 4, argv);*/ //FIXME makeCallback
            (*req)["oncomplete"](status, _this, *req, (*req)["buffer"]);
            delete req;
        }
        
        static uv_stream_t *m_HandleThatLastAlloced;
        static Value m_Slab;
        static size_t m_nSlabUsed;
        
#undef TPL_sT
#undef HANDLE
#undef SLAB_SIZE
    };
}

#endif
