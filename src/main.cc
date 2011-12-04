#line 1 "src/main.gear"

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

/** \file src/main.cc converted from src/main.gear */

#line 25 "src/main.gear"

#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <UvCommon.h>

static Value g_Process;

static char pathBuffer[PATH_MAX + 1];

static uv_check_t check_tick_watcher;
static uv_prepare_t prepare_tick_watcher;
static uv_idle_t tick_spinner;
static bool need_tick_cb;

static void tick() {
    // Avoid entering a V8 scope.
    if(!need_tick_cb)
        return;
    
    need_tick_cb = false;
    if(uv_is_active(reinterpret_cast<uv_handle_t*>(&tick_spinner))) {
        uv_idle_stop(&tick_spinner);
        uv_unref(uv_default_loop());
    }
    // HACK Maybe we can do this better.
    g_Process["_tickCallback"]();
    /*Local<Value> cb_v = process->Get(tick_callback_sym);
    if (!cb_v->IsFunction()) return;
    Local<Function> cb = Local<Function>::Cast(cb_v);
    
    TryCatch try_catch;
    
    cb->Call(process, 0, NULL);
    
    if (try_catch.HasCaught()) {
        FatalException(try_catch);
    }*/
}


static void spin(uv_idle_t *handle, int status) {
    assert(reinterpret_cast<uv_idle_t*>(handle) == &tick_spinner);
    assert(status == 0);
    tick();
}

static void init(Value argv);

int main(int argc, char *argv[]) {
    v8::HandleScope handleScope;
    
    // Pass the flags first to v8
    // TODO Pass to v8 only the flags that we do not recognize
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    
    // Create a new context
    Context context;
    TryCatch tryCatch;
    
    // Set the arguments array
    var arguments = Array();
    for(int i = 0; i < argc; i++)
        arguments[i] = argv[i];
    
    uv_idle_init(uv_default_loop(), &tick_spinner);
    uv_unref(uv_default_loop());
    
    init(arguments);
    
    // All our arguments are loaded. We've evaluated all of the scripts. We
    // might even have created TCP servers. Now we enter the main eventloop. If
    // there are no watchers on the loop (except for the ones that were
    // uv_unref'd) then this function exits. As long as there are active
    // watchers, it blocks.
    uv_run(uv_default_loop());
}

static v8::Handle<v8::Value> _init_process_binding(const v8::Arguments &args) {
    #line 107 "src/main.gear"
    THROW_ERROR("process.binding is deprecated in gearbox-node");
    return undefined;
}

static v8::Handle<v8::Value> _init_process__needTickCallback(const v8::Arguments &args) {
    #line 111 "src/main.gear"
    need_tick_cb = true;
            // TODO: This tick_spinner shouldn't be necessary. An ev_prepare should be
            // sufficent, the problem is only in the case of the very last "tick" -
            // there is nothing left to do in the event loop and libev will exit. The
            // ev_prepare callback isn't called before exiting. Thus we start This
            // tick_spinner to keep the event loop alive long enough to handle it.
            if(!uv_is_active(reinterpret_cast<uv_handle_t*>(&tick_spinner))) {
    uv_idle_start(&tick_spinner, spin);
    uv_ref(uv_default_loop());
            }
    return undefined;
}

static v8::Handle<v8::Value> _init_process_cwd(const v8::Arguments &args) {
    #line 124 "src/main.gear"
    if(getcwd(pathBuffer, PATH_MAX) == NULL)
        THROW_ERROR(strerror(errno));
    
    pathBuffer[PATH_MAX] = '\0';
    return String(pathBuffer);
}

static v8::Handle<v8::Value> _init_nativeModule_exists(const v8::Arguments &args) {
    if(args.Length() >= 1) {
        #line 136 "src/main.gear"
        Value id(args[0]);
        return Value(NativeModule::exists(id));
    }
    THROW_ERROR("Invalid call to init.nativeModule.exists");
}

static v8::Handle<v8::Value> _init_nativeModule_require(const v8::Arguments &args) {
    if(args.Length() >= 2) {
        #line 139 "src/main.gear"
        Value id(args[0]), _require(args[1]);
        return NativeModule::require(id, _require);
    }
    THROW_ERROR("Invalid call to init.nativeModule.require");
}


#line 159 "src/main.cc"
void init(Value argv) {
    var process = Object();
    process["binding"] = Function(_init_process_binding, "binding");
    process["_needTickCallback"] = Function(_init_process__needTickCallback, "_needTickCallback");
    process["cwd"] = Function(_init_process_cwd, "cwd");
    process["env"] = Value(Object());
    process["argv"] = Value(argv);
    var nativeModule = Object();
    nativeModule["exists"] = Function(_init_nativeModule_exists, "exists");
    nativeModule["require"] = Function(_init_nativeModule_require, "require");
    #line 144 "src/main.gear"

        g_Process = process;
        Context::getCurrent()->runScript("(function(argv, process, nativeModule){\n//BEGIN src/node.js\n// Hello, and welcome to hacking node.js!\n//\n// This file is invoked by node::Load in src/node.cc, and responsible for\n// bootstrapping the node.js core. Special caution is given to the performance\n// of the startup process, so many dependencies are invoked lazily.\nglobal = this;\n\nvar EventEmitter;\n\nfunction startup() {\n  EventEmitter = NativeModule.require('events').EventEmitter;\n  process.__proto__ = EventEmitter.prototype;\n  process.EventEmitter = EventEmitter; // process.EventEmitter is deprecated\n\n  startup.globalVariables();\n  startup.globalTimeouts();\n  startup.globalConsole();\n\n  startup.processAssert();\n  startup.processNextTick();\n  startup.processStdio();\n  startup.processKillAndExit();\n  startup.processSignalHandlers();\n\n  startup.processChannel();\n\n  startup.removedMethods();\n\n  startup.resolveArgv0();\n\n  // There are various modes that Node can run in. The most common two\n  // are running from a script and running the REPL - but there are a few\n  // others like the debugger or running --eval arguments. Here we decide\n  // which mode we run in.\n\n  if (NativeModule.exists('_third_party_main')) {\n    // To allow people to extend Node in different ways, this hook allows\n    // one to drop a file lib/_third_party_main.js into the build\n    // directory which will be executed instead of Node's normal loading.\n    process.nextTick(function() {\n      NativeModule.require('_third_party_main');\n    });\n\n  } else if (process.argv[1] == 'debug') {\n    // Start the debugger agent\n    var d = NativeModule.require('_debugger');\n    d.start();\n\n  } else if (process._eval != null) {\n    // User passed '-e' or '--eval' arguments to Node.\n    var Module = NativeModule.require('module');\n    var path = NativeModule.require('path');\n    var cwd = process.cwd();\n\n    var module = new Module('eval');\n    module.filename = path.join(cwd, 'eval');\n    module.paths = Module._nodeModulePaths(cwd);\n    module._compile('eval(process._eval)', 'eval');\n\n  } else if (process.argv[1]) {\n    // make process.argv[1] into a full path\n    var path = NativeModule.require('path');\n    process.argv[1] = path.resolve(process.argv[1]);\n\n    // If this is a worker in cluster mode, start up the communiction\n    // channel.\n    if (process.env.NODE_WORKER_ID) {\n      var cluster = NativeModule.require('cluster');\n      cluster._startWorker();\n    }\n\n    var Module = NativeModule.require('module');\n    // REMOVEME: nextTick should not be necessary. This hack to get\n    // test/simple/test-exception-handler2.js working.\n    // Main entry point into most programs:\n    process.nextTick(Module.runMain);\n\n  } else {\n    var Module = NativeModule.require('module');\n\n    // If stdin is a TTY.\n    if (NativeModule.require('tty').isatty(0)) {\n      // REPL\n      var repl = Module.requireRepl().start('> ', null, null, true);\n\n    } else {\n      // Read all of stdin - execute it.\n      process.stdin.resume();\n      process.stdin.setEncoding('utf8');\n\n      var code = '';\n      process.stdin.on('data', function(d) {\n        code += d;\n      });\n\n      process.stdin.on('end', function() {\n        new Module()._compile(code, '[stdin]');\n      });\n    }\n  }\n}\n\nstartup.globalVariables = function() {\n  global.process = process;\n  global.global = global;\n  global.GLOBAL = global;\n  global.root = global;\n  global.Buffer = NativeModule.require('buffer').Buffer;\n};\n\nstartup.globalTimeouts = function() {\n  global.setTimeout = function() {\n    var t = NativeModule.require('timers');\n    return t.setTimeout.apply(this, arguments);\n  };\n\n  global.setInterval = function() {\n    var t = NativeModule.require('timers');\n    return t.setInterval.apply(this, arguments);\n  };\n\n  global.clearTimeout = function() {\n    var t = NativeModule.require('timers');\n    return t.clearTimeout.apply(this, arguments);\n  };\n\n  global.clearInterval = function() {\n    var t = NativeModule.require('timers');\n    return t.clearInterval.apply(this, arguments);\n  };\n};\n\nstartup.globalConsole = function() {\n  global.__defineGetter__('console', function() {\n    return NativeModule.require('console');\n  });\n};\n\n\nstartup._lazyConstants = null;\n\nstartup.lazyConstants = function() {\n  if (!startup._lazyConstants) {\n//BEGIN *gearbox\n    //startup._lazyConstants = process.binding('constants');\n    startup._lazyConstants = require('constants');\n//END *gearbox\n  }\n  return startup._lazyConstants;\n};\n\nvar assert;\nstartup.processAssert = function() {\n  // Note that calls to assert() are pre-processed out by JS2C for the\n  // normal build of node. They persist only in the node_g build.\n  // Similarly for debug().\n  assert = process.assert = function(x, msg) {\n    if (!x) throw new Error(msg || 'assertion error');\n  };\n};\n\nstartup.processNextTick = function() {\n  var nextTickQueue = [];\n\n  process._tickCallback = function() {\n    var l = nextTickQueue.length;\n    if (l === 0) return;\n\n    var q = nextTickQueue;\n    nextTickQueue = [];\n\n    try {\n      for (var i = 0; i < l; i++) q[i]();\n    }\n    catch (e) {\n      if (i + 1 < l) {\n        nextTickQueue = q.slice(i + 1).concat(nextTickQueue);\n      }\n      if (nextTickQueue.length) {\n        process._needTickCallback();\n      }\n      throw e; // process.nextTick error, or 'error' event on first tick\n    }\n  };\n\n  process.nextTick = function(callback) {\n    nextTickQueue.push(callback);\n    process._needTickCallback();\n  };\n};\n\nfunction errnoException(errorno, syscall) {\n  // TODO make this more compatible with ErrnoException from src/node.cc\n  // Once all of Node is using this function the ErrnoException from\n  // src/node.cc should be removed.\n  var e = new Error(syscall + ' ' + errorno);\n  e.errno = e.code = errorno;\n  e.syscall = syscall;\n  return e;\n}\n\nfunction createWritableStdioStream(fd) {\n  var stream;\n//BEGIN *gearbox\n  //var tty_wrap = process.binding('tty_wrap');\n  var tty = NativeModule.require('tty'), tty_wrap = tty;\n//END *gearbox\n\n  // Note stream._type is used for test-module-load-list.js\n\n  switch (tty_wrap.guessHandleType(fd)) {\n    case 'TTY':\n//BEGIN *gearbox\n      //var tty = NativeModule.require('tty');\n//END *gearbox\n      stream = new tty.WriteStream(fd);\n      stream._type = 'tty';\n\n      // Hack to have stream not keep the event loop alive.\n      // See https://github.com/joyent/node/issues/1726\n      if (stream._handle && stream._handle.unref) {\n        stream._handle.unref();\n      }\n      break;\n\n    case 'FILE':\n      var fs = NativeModule.require('fs');\n      stream = new fs.SyncWriteStream(fd);\n      stream._type = 'fs';\n      break;\n\n    case 'PIPE':\n      var net = NativeModule.require('net');\n      stream = new net.Stream(fd);\n\n      // FIXME Should probably have an option in net.Stream to create a\n      // stream from an existing fd which is writable only. But for now\n      // we'll just add this hack and set the `readable` member to false.\n      // Test: ./node test/fixtures/echo.js < /etc/passwd\n      stream.readable = false;\n      stream._type = 'pipe';\n\n      // FIXME Hack to have stream not keep the event loop alive.\n      // See https://github.com/joyent/node/issues/1726\n      if (stream._handle && stream._handle.unref) {\n        stream._handle.unref();\n      }\n      break;\n\n    default:\n      // Probably an error on in uv_guess_handle()\n      throw new Error('Implement me. Unknown stream file type!');\n  }\n\n  // For supporting legacy API we put the FD here.\n  stream.fd = fd;\n\n  stream._isStdio = true;\n\n  return stream;\n}\n\nstartup.processStdio = function() {\n  var stdin, stdout, stderr;\n\n  process.__defineGetter__('stdout', function() {\n    if (stdout) return stdout;\n    stdout = createWritableStdioStream(1);\n    stdout.end = stdout.destroy = stdout.destroySoon = function() {\n      throw new Error('process.stdout cannot be closed');\n    };\n    return stdout;\n  });\n\n  process.__defineGetter__('stderr', function() {\n    if (stderr) return stderr;\n    stderr = createWritableStdioStream(2);\n    stderr.end = stderr.destroy = stderr.destroySoon = function() {\n      throw new Error('process.stderr cannot be closed');\n    };\n    return stderr;\n  });\n\n  process.__defineGetter__('stdin', function() {\n    if (stdin) return stdin;\n\n//BEGIN *gearbox\n    //var tty_wrap = process.binding('tty_wrap');\n    var tty = NativeModule.require('tty'), tty_wrap = tty;\n//END *gearbox\n    var fd = 0;\n\n    switch (tty_wrap.guessHandleType(fd)) {\n      case 'TTY':\n//BEGIN *gearbox\n        //var tty = NativeModule.require('tty');\n//END *gearbox\n        stdin = new tty.ReadStream(fd);\n        break;\n\n      case 'FILE':\n        var fs = NativeModule.require('fs');\n        stdin = new fs.ReadStream(null, {fd: fd});\n        break;\n\n      case 'PIPE':\n        var net = NativeModule.require('net');\n        stdin = new net.Stream(fd);\n        stdin.readable = true;\n        break;\n\n      default:\n        // Probably an error on in uv_guess_handle()\n        throw new Error('Implement me. Unknown stdin file type!');\n    }\n    \n    if (stdin._handle && stdin._handle.unref) {\n      stdin._handle.unref();\n    }\n\n    // For supporting legacy API we put the FD here.\n    stdin.fd = fd;\n\n    return stdin;\n  });\n\n  process.openStdin = function() {\n    process.stdin.resume();\n    return process.stdin;\n  };\n};\n\nstartup.processKillAndExit = function() {\n  process.exit = function(code) {\n    process.emit('exit', code || 0);\n    process.reallyExit(code || 0);\n  };\n\n  process.kill = function(pid, sig) {\n    var r;\n    \n    // preserve null signal\n    if (0 === sig) {\n      r = process._kill(pid, 0);\n    } else {\n      sig = sig || 'SIGTERM';\n      if (startup.lazyConstants()[sig]) {\n        r = process._kill(pid, startup.lazyConstants()[sig]);\n      } else {\n        throw new Error('Unknown signal: ' + sig);\n      }\n    }\n\n    if (r) {\n      throw errnoException(errno, 'kill');\n    }\n  };\n};\n\nstartup.processSignalHandlers = function() {\n  // Load events module in order to access prototype elements on process like\n  // process.addListener.\n  var signalWatchers = {};\n  var addListener = process.addListener;\n  var removeListener = process.removeListener;\n\n  function isSignal(event) {\n    return event.slice(0, 3) === 'SIG' && startup.lazyConstants()[event];\n  }\n\n  // Wrap addListener for the special signal types\n  process.on = process.addListener = function(type, listener) {\n    var ret = addListener.apply(this, arguments);\n    if (isSignal(type)) {\n      if (!signalWatchers.hasOwnProperty(type)) {\n        var b = process.binding('signal_watcher');\n        var w = new b.SignalWatcher(startup.lazyConstants()[type]);\n        w.callback = function() { process.emit(type); };\n        signalWatchers[type] = w;\n        w.start();\n\n      } else if (this.listeners(type).length === 1) {\n        signalWatchers[type].start();\n      }\n    }\n\n    return ret;\n  };\n\n  process.removeListener = function(type, listener) {\n    var ret = removeListener.apply(this, arguments);\n    if (isSignal(type)) {\n      assert(signalWatchers.hasOwnProperty(type));\n\n      if (this.listeners(type).length === 0) {\n        signalWatchers[type].stop();\n      }\n    }\n\n    return ret;\n  };\n};\n\n\nstartup.processChannel = function() {\n  // If we were spawned with env NODE_CHANNEL_FD then load that up and\n  // start parsing data from that stream.\n  if (process.env.NODE_CHANNEL_FD) {\n    var fd = parseInt(process.env.NODE_CHANNEL_FD);\n    assert(fd >= 0);\n    var cp = NativeModule.require('child_process');\n\n    // Load tcp_wrap to avoid situation where we might immediately receive\n    // a message.\n    // FIXME is this really necessary?\n    process.binding('tcp_wrap')\n\n    cp._forkChild(fd);\n    assert(process.send);\n  }\n}\n\nstartup._removedProcessMethods = {\n  'assert': 'process.assert() use require(\"assert\").ok() instead',\n  'debug': 'process.debug() use console.error() instead',\n  'error': 'process.error() use console.error() instead',\n  'watchFile': 'process.watchFile() has moved to fs.watchFile()',\n  'unwatchFile': 'process.unwatchFile() has moved to fs.unwatchFile()',\n  'mixin': 'process.mixin() has been removed.',\n  'createChildProcess': 'childProcess API has changed. See doc/api.txt.',\n  'inherits': 'process.inherits() has moved to util.inherits()',\n  '_byteLength': 'process._byteLength() has moved to Buffer.byteLength'\n};\n\nstartup.removedMethods = function() {\n  for (var method in startup._removedProcessMethods) {\n    var reason = startup._removedProcessMethods[method];\n    process[method] = startup._removedMethod(reason);\n  }\n};\n\nstartup._removedMethod = function(reason) {\n  return function() {\n    throw new Error(reason);\n  };\n};\n\nstartup.resolveArgv0 = function() {\n  var cwd = process.cwd();\n  var isWindows = process.platform === 'win32';\n\n  // Make process.argv[0] into a full path, but only touch argv[0] if it's\n  // not a system $PATH lookup.\n  // TODO: Make this work on Windows as well.  Note that \"node\" might\n  // execute cwd\\node.exe, or some %PATH%\\node.exe on Windows,\n  // and that every directory has its own cwd, so d:node.exe is valid.\n  var argv0 = process.argv[0];\n  if (!isWindows && argv0.indexOf('/') !== -1 && argv0.charAt(0) !== '/') {\n    var path = NativeModule.require('path');\n    process.argv[0] = path.join(cwd, process.argv[0]);\n  }\n};\n\n//BEGIN *gearbox\n// Below you find a minimal module system, which is used to load the node\n// core modules found in lib/*.js. All core modules are compiled into the\n// node binary, so they can be loaded faster.\n\n\nvar NativeModule = {};\n\nNativeModule.require = function(id) {\n    if(id == 'native_module')\n        return NativeModule;\n    \n    return nativeModule.require(id, NativeModule.require);\n};\n\nNativeModule.exists = function(id) {\n    return nativeModule.exists(id);\n};\n\n/*\nvar Script = process.binding('evals').NodeScript;\nvar runInThisContext = Script.runInThisContext;\n\nfunction NativeModule(id) {\n  this.filename = id + '.js';\n  this.id = id;\n  this.exports = {};\n  this.loaded = false;\n}\n\nNativeModule._source = process.binding('natives');\nNativeModule._cache = {};\n\nNativeModule.require = function(id) {\n  if (id == 'native_module') {\n    return NativeModule;\n  }\n\n  var cached = NativeModule.getCached(id);\n  if (cached) {\n    return cached.exports;\n  }\n\n  if (!NativeModule.exists(id)) {\n    throw new Error('No such native module ' + id);\n  }\n\n  process.moduleLoadList.push('NativeModule ' + id);\n\n  var nativeModule = new NativeModule(id);\n\n  nativeModule.compile();\n  nativeModule.cache();\n\n  return nativeModule.exports;\n};\n\nNativeModule.getCached = function(id) {\n  return NativeModule._cache[id];\n}\n\nNativeModule.exists = function(id) {\n  return (id in NativeModule._source);\n}\n\nNativeModule.getSource = function(id) {\n  return NativeModule._source[id];\n}\n\nNativeModule.wrap = function(script) {\n  return NativeModule.wrapper[0] + script + NativeModule.wrapper[1];\n};\n\nNativeModule.wrapper = [\n  '(function (exports, require, module, __filename, __dirname) { ',\n  '\\n});'\n];\n\nNativeModule.prototype.compile = function() {\n  var source = NativeModule.getSource(this.id);\n  source = NativeModule.wrap(source);\n\n  var fn = runInThisContext(source, this.filename, true);\n  fn(this.exports, NativeModule.require, this, this.filename);\n\n  this.loaded = true;\n};\n\nNativeModule.prototype.cache = function() {\n  NativeModule._cache[this.id] = this;\n};\n*/\n//END *gearbox\n\nstartup();\n//END src/node.js\n    })", "gear:init")(argv, process, nativeModule);
}
