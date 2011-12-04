#line 1 "src/modules/timers.gear"

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

/** \file src/modules/timers.cc converted from src/modules/timers.gear */

#line 25 "src/modules/timers.gear"

#include <UvCommon.h>

struct _timers_timer_wrap_Timer_wrap /*: public Value::DtorWrap*/ {
    uv_timer_t handle;
    bool unref;
    bool active;

    struct This : public Value {
        This(v8::Handle<v8::Object> &&_this, _timers_timer_wrap_Timer_wrap *wrap) : Value(_this), _wrap(wrap), handle(wrap->handle), unref(wrap->unref), active(wrap->active) {
            _this->SetPointerInInternalField(0, wrap);
        }
        This(v8::Handle<v8::Object> &&_this) : Value(_this), _wrap(static_cast<_timers_timer_wrap_Timer_wrap*>(_this->GetPointerFromInternalField(0))), handle(_wrap->handle), unref(_wrap->unref), active(_wrap->active) {}
        _timers_timer_wrap_Timer_wrap *_wrap;
        uv_timer_t &handle;
        bool &unref;
        bool &active;
    };
};

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_Timer(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This(), new _timers_timer_wrap_Timer_wrap);
    #line 37 "src/modules/timers.gear"
    This.active = false;
                assert(uv_timer_init(uv_default_loop(), &This.handle) == 0);
                
                //FIXME Will This even work? It would never be cleaned up.
                This.handle.data = new decltype(This)(This);
                
                // uv_timer_init adds a loop reference. (That is, it calls uv_ref.) This
                // is not the behavior we want in Node. Timers should not increase the
                // ref count of the loop except when active.
                uv_unref(uv_default_loop());
    return undefined;
}

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_unref(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This());
    #line 50 "src/modules/timers.gear"
    Uv::Handle::unref(This);
    return undefined;
}

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_close(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This());
    #line 52 "src/modules/timers.gear"
    Uv::Handle::close(This);
    return undefined;
}

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_start(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This());
    if(args.Length() >= 2) {
        #line 54 "src/modules/timers.gear"
        Value timeout(args[0]), repeat(args[1]);
        typedef decltype(This) Timer;
                int r = uv_timer_start(&This.handle, [](uv_timer_t *handle, int status) {
        Timer *timer = static_cast<Timer*>(handle->data);
        Uv::Handle::stateChange(*timer);
        (*timer)["ontimeout"](status);
                }, timeout, repeat);
                
                // Error starting the timer.
                //if(r) SetErrno(uv_last_error(uv_default_loop())); //FIXME SetErrno
                
                Uv::Handle::stateChange(This);
                return Integer(r);
        return undefined;
    }
    THROW_ERROR("Invalid call to timers.timer.wrap.Timer.prototype.start");
}

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_stop(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This());
    #line 69 "src/modules/timers.gear"
    int r = uv_timer_stop(&This.handle);
                
                //if(r) SetErrno(uv_last_error(uv_default_loop())); //FIXME SetErrno
                
                Uv::Handle::stateChange(This);
                return Integer(r);
    return undefined;
}

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_again(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This());
    #line 78 "src/modules/timers.gear"
    int r = uv_timer_again(&This.handle);
                
                //if(r) SetErrno(uv_last_error(uv_default_loop())); //FIXME SetErrno
                
                Uv::Handle::stateChange(This);
                return Integer(r);
    return undefined;
}

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_setRepeat(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This());
    if(args.Length() >= 1) {
        #line 86 "src/modules/timers.gear"
        Value repeat(args[0]);
        uv_timer_set_repeat(&This.handle, repeat);
                return Integer(0);
        return undefined;
    }
    THROW_ERROR("Invalid call to timers.timer.wrap.Timer.prototype.setRepeat");
}

static v8::Handle<v8::Value> _timers_timer_wrap_Timer_getRepeat(const v8::Arguments &args) {
    _timers_timer_wrap_Timer_wrap::This This(args.This());
    #line 91 "src/modules/timers.gear"
    int64_t repeat = uv_timer_get_repeat(&This.handle);
                
                //if(repeat < 0) SetErrno(uv_last_error(uv_default_loop())); // FIXME SetErrno
                
                return Integer(repeat);
    return undefined;
}


#line 152 "src/modules/timers.cc"
static void _setup_timers(Value exports, Value require, Value module) {
    var timer_wrap = Object();
    v8::Handle<v8::FunctionTemplate> _timers_timer_wrap_Timer = v8::FunctionTemplate::New(_timers_timer_wrap_Timer_Timer);
    _timers_timer_wrap_Timer->SetClassName(String("Timer"));
    _timers_timer_wrap_Timer->InstanceTemplate()->SetInternalFieldCount(1);
    _timers_timer_wrap_Timer->PrototypeTemplate()->Set("unref", Function(_timers_timer_wrap_Timer_unref, "unref"));
    _timers_timer_wrap_Timer->PrototypeTemplate()->Set("close", Function(_timers_timer_wrap_Timer_close, "close"));
    _timers_timer_wrap_Timer->PrototypeTemplate()->Set("start", Function(_timers_timer_wrap_Timer_start, "start"));
    _timers_timer_wrap_Timer->PrototypeTemplate()->Set("stop", Function(_timers_timer_wrap_Timer_stop, "stop"));
    _timers_timer_wrap_Timer->PrototypeTemplate()->Set("again", Function(_timers_timer_wrap_Timer_again, "again"));
    _timers_timer_wrap_Timer->PrototypeTemplate()->Set("setRepeat", Function(_timers_timer_wrap_Timer_setRepeat, "setRepeat"));
    _timers_timer_wrap_Timer->PrototypeTemplate()->Set("getRepeat", Function(_timers_timer_wrap_Timer_getRepeat, "getRepeat"));
    timer_wrap["Timer"] = _timers_timer_wrap_Timer->GetFunction();
    Context::getCurrent()->runScript("(function(exports, require, module, timer_wrap){\n//BEGIN lib/timers.js\n//BEGIN *gearbox\n//var Timer = process.binding('timer_wrap').Timer;\nvar Timer = timer_wrap.Timer;\n//END *gearbox\nvar L = require('_linklist');\nvar assert = require('assert').ok;\n\nvar debug;\nif (process.env.NODE_DEBUG && /timer/.test(process.env.NODE_DEBUG)) {\n  debug = function() { require('util').error.apply(this, arguments); };\n} else {\n  debug = function() { };\n}\n\n\n// IDLE TIMEOUTS\n//\n// Because often many sockets will have the same idle timeout we will not\n// use one timeout watcher per item. It is too much overhead.  Instead\n// we'll use a single watcher for all sockets with the same timeout value\n// and a linked list. This technique is described in the libev manual:\n// http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts\n\n// Object containing all lists, timers\n// key = time in milliseconds\n// value = list\nvar lists = {};\n\n\n// the main function - creates lists on demand and the watchers associated\n// with them.\nfunction insert(item, msecs) {\n  item._idleStart = new Date();\n  item._idleTimeout = msecs;\n\n  if (msecs < 0) return;\n\n  var list;\n\n  if (lists[msecs]) {\n    list = lists[msecs];\n  } else {\n    list = new Timer();\n    list.start(msecs, 0);\n\n    L.init(list);\n\n    lists[msecs] = list;\n\n    list.ontimeout = function() {\n      debug('timeout callback ' + msecs);\n\n      var now = new Date();\n      debug('now: ' + now);\n\n      var first;\n      while (first = L.peek(list)) {\n        var diff = now - first._idleStart;\n        if (diff + 1 < msecs) {\n          list.start(msecs - diff, 0);\n          debug(msecs + ' list wait because diff is ' + diff);\n          return;\n        } else {\n          L.remove(first);\n          assert(first !== L.peek(list));\n          if (first._onTimeout) first._onTimeout();\n        }\n      }\n\n      debug(msecs + ' list empty');\n      assert(L.isEmpty(list));\n      list.close();\n      delete lists[msecs];\n    };\n  }\n\n  L.append(list, item);\n  assert(!L.isEmpty(list)); // list is not empty\n}\n\n\nvar unenroll = exports.unenroll = function(item) {\n  L.remove(item);\n\n  var list = lists[item._idleTimeout];\n  // if empty then stop the watcher\n  debug('unenroll');\n  if (list && L.isEmpty(list)) {\n    debug('unenroll: list empty');\n    list.close();\n    delete lists[item._idleTimeout];\n  }\n};\n\n\n// Does not start the time, just sets up the members needed.\nexports.enroll = function(item, msecs) {\n  // if this item was already in a list somewhere\n  // then we should unenroll it from that\n  if (item._idleNext) unenroll(item);\n\n  item._idleTimeout = msecs;\n  L.init(item);\n};\n\n\n// call this whenever the item is active (not idle)\n// it will reset its timeout.\nexports.active = function(item) {\n  var msecs = item._idleTimeout;\n  if (msecs >= 0) {\n    var list = lists[msecs];\n    if (!list || L.isEmpty(list)) {\n      insert(item, msecs);\n    } else {\n      item._idleStart = new Date();\n      L.append(list, item);\n    }\n  }\n};\n\n\n/*\n * DOM-style timers\n */\n\n\nexports.setTimeout = function(callback, after) {\n  var timer, c, args;\n\n  if (after <= 0) {\n    // Use the slow case for after == 0\n    timer = new Timer();\n    timer.ontimeout = callback;\n\n    args = Array.prototype.slice.call(arguments, 2);\n    timer._onTimeout = function() {\n      callback.apply(timer, args);\n      timer.close();\n    }\n\n    timer.start(0, 0);\n  } else {\n    timer = { _idleTimeout: after };\n    timer._idlePrev = timer;\n    timer._idleNext = timer;\n\n    if (arguments.length <= 2) {\n      timer._onTimeout = callback;\n    } else {\n      /*\n       * Sometimes setTimeout is called with arguments, EG\n       *\n       *   setTimeout(callback, 2000, \"hello\", \"world\")\n       *\n       * If that's the case we need to call the callback with\n       * those args. The overhead of an extra closure is not\n       * desired in the normal case.\n       */\n      args = Array.prototype.slice.call(arguments, 2);\n      timer._onTimeout = function() {\n        callback.apply(timer, args);\n      }\n    }\n\n    exports.active(timer);\n  }\n\n  return timer;\n};\n\n\nexports.clearTimeout = function(timer) {\n  if (timer && (timer.ontimeout || timer._onTimeout)) {\n    timer.ontimeout = timer._onTimeout = null;\n    if (timer instanceof Timer) {\n      timer.close(); // for after === 0\n    } else {\n      exports.unenroll(timer);\n    }\n  }\n};\n\n\nexports.setInterval = function(callback, repeat) {\n  var timer = new Timer();\n\n  var args = Array.prototype.slice.call(arguments, 2);\n  timer.ontimeout = function() {\n    callback.apply(timer, args);\n  }\n\n  timer.start(repeat, repeat ? repeat : 1);\n  return timer;\n};\n\n\nexports.clearInterval = function(timer) {\n  if (timer instanceof Timer) {\n    timer.ontimeout = null;\n    timer.close();\n  }\n};\n//END lib/timers.js\n    })", "gear:timers")(exports, require, module, timer_wrap);
}
static NativeModule _module_timers("timers", _setup_timers);