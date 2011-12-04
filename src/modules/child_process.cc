#line 1 "src/modules/child_process.gear"

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

/** \file src/modules/child_process.cc converted from src/modules/child_process.gear */

#line 25 "src/modules/child_process.gear"

#include <UvCommon.h>
#include <cstring>

struct _child_process_process_wrap_Process_wrap /*: public Value::DtorWrap*/ {
    uv_process_t handle;

    struct This : public Value {
        This(v8::Handle<v8::Object> &&_this, _child_process_process_wrap_Process_wrap *wrap) : Value(_this), _wrap(wrap), handle(wrap->handle) {
            _this->SetPointerInInternalField(0, wrap);
        }
        This(v8::Handle<v8::Object> &&_this) : Value(_this), _wrap(static_cast<_child_process_process_wrap_Process_wrap*>(_this->GetPointerFromInternalField(0))), handle(_wrap->handle) {}
        _child_process_process_wrap_Process_wrap *_wrap;
        uv_process_t &handle;
    };
};

static v8::Handle<v8::Value> _child_process_process_wrap_Process_Process(const v8::Arguments &args) {
    _child_process_process_wrap_Process_wrap::This This(args.This(), new _child_process_process_wrap_Process_wrap);
    #line 36 "src/modules/child_process.gear"
    //FIXME Will This even work? It would never be cleaned up.
                This.handle.data = new decltype(This)(This);
    return undefined;
}

static v8::Handle<v8::Value> _child_process_process_wrap_Process_spawn(const v8::Arguments &args) {
    _child_process_process_wrap_Process_wrap::This This(args.This());
    if(args.Length() >= 1) {
        #line 40 "src/modules/child_process.gear"
        Value jsOptions(args[0]);
        uv_process_options_t options;
                memset(&options, 0, sizeof(uv_process_options_t));
                
                //FIXME This can be wrong
                typedef decltype(This) Process;
                options.exit_cb = [](uv_process_t *handle, int exitStatus, int termSignal) {
                    Process *process = static_cast<Process*>(handle->data);
                    (*process)["onexit"](exitStatus, termSignal);
                    
                    /*Local<Value> argv[2] = {
        Integer::New(exit_status),
        String::New(signo_string(term_signal))
                    };
                    
                    MakeCallback(wrap->object_, "onexit", 2, argv);*/ //FIXME makeCallback
                };
                
                // TODO is This possible to do without mallocing ?
                
                //FIXME node DOES strdup and free afterwards, for the strings;
                // We should be able to store String instances at the function-scope
                // so that we don't need all This extra work, but it's TODO for now
                
                // options.file
                var file_v = jsOptions["file"];
                if(file_v.is<String>())
                    options.file = strdup(file_v.to<String>());
                
                // options.args
                var argv_v = jsOptions["args"];
                if(argv_v.is<Array>()) {
                    int argc = argv_v.length();
                    // Heap allocate to detect errors. +1 is for NULL.
                    options.args = new char*[argc + 1];
                    for(int i = 0; i < argc; i++)
        options.args[i] = strdup(argv_v[i].to<String>());
                    options.args[argc] = NULL;
                }
                
                // options.cwd
                var cwd_v = jsOptions["cwd"];
                if(cwd_v.is<String>() && cwd_v.length())
                    options.cwd = strdup(cwd_v.to<String>());
                
                // options.env
                var env_v = jsOptions["envPairs"];
                if(env_v.is<Array>()) {
                    int envc = env_v.length();
                    options.env = new char*[envc + 1]; // Heap allocated to detect errors.
                    for(int i = 0; i < envc; i++)
        options.env[i] = strdup(env_v[i].to<String>());
                    options.env[envc] = NULL;
                }
                
                // options.stdin_stream
                var stdin_stream_v = jsOptions["stdinStream"];
                if(stdin_stream_v.is<Object>())
                    options.stdin_stream = Uv::unwrapHandle<uv_pipe_t>(stdin_stream_v);
                
                // options.stdout_stream
                var stdout_stream_v = jsOptions["stdoutStream"];
                if(stdout_stream_v.is<Object>())
                    options.stdout_stream = Uv::unwrapHandle<uv_pipe_t>(stdout_stream_v);
                
                // options.stderr_stream
                var stderr_stream_v = jsOptions["stderrStream"];
                if(stderr_stream_v.is<Object>())
                    options.stderr_stream = Uv::unwrapHandle<uv_pipe_t>(stderr_stream_v);
                
                // options.windows_verbatim_arguments
#if defined(_WIN32)
                options.windows_verbatim_arguments = jsOptions["windowsVerbatimArguments"].to<bool>();
#endif
                
                int r = uv_spawn(uv_default_loop(), &This.handle, options);
                
                This["pid"] = This.handle.pid;
                
                if(options.args) {
                    for(int i = 0; options.args[i]; i++)
        free(options.args[i]); //FIXME see above
                    delete [] options.args;
                }
                
                free(options.cwd); //FIXME see above
                free((void*)options.file); //FIXME see above
                
                if(options.env) {
                    for(int i = 0; options.env[i]; i++)
        free(options.env[i]); //FIXME see above
                    delete [] options.env;
                }
                
                //if(r) SetErrno(uv_last_error(uv_default_loop())); //FIXME SetErrno
                
                return Integer(r);
        return undefined;
    }
    THROW_ERROR("Invalid call to child.process.process.wrap.Process.prototype.spawn");
}

static v8::Handle<v8::Value> _child_process_process_wrap_Process_kill(const v8::Arguments &args) {
    _child_process_process_wrap_Process_wrap::This This(args.This());
    if(args.Length() >= 1) {
        #line 139 "src/modules/child_process.gear"
        Value signal(args[0]);
        int r = uv_process_kill(&This.handle, signal);
                
                //if(r) SetErrno(uv_last_error(uv_default_loop())); //FIXME SetErrno
                
                Uv::Handle::stateChange(This);
                return Integer(r);
        return undefined;
    }
    THROW_ERROR("Invalid call to child.process.process.wrap.Process.prototype.kill");
}


#line 180 "src/modules/child_process.cc"
static void _setup_child_process(Value exports, Value require, Value module) {
    var process_wrap = Object();
    v8::Handle<v8::FunctionTemplate> _child_process_process_wrap_Process = v8::FunctionTemplate::New(_child_process_process_wrap_Process_Process);
    _child_process_process_wrap_Process->SetClassName(String("Process"));
    _child_process_process_wrap_Process->InstanceTemplate()->SetInternalFieldCount(1);
    _child_process_process_wrap_Process->PrototypeTemplate()->Set("spawn", Function(_child_process_process_wrap_Process_spawn, "spawn"));
    _child_process_process_wrap_Process->PrototypeTemplate()->Set("kill", Function(_child_process_process_wrap_Process_kill, "kill"));
    process_wrap["Process"] = _child_process_process_wrap_Process->GetFunction();
    Context::getCurrent()->runScript("(function(exports, require, module, process_wrap){\n//BEGIN lib/child_process.js\nvar EventEmitter = require('events').EventEmitter;\nvar net = require('net');\n//BEGIN *gearbox\n//var Process = process.binding('process_wrap').Process;\nvar Process = process_wrap.Process;\n//END *gearbox\nvar inherits = require('util').inherits;\nvar constants; // if (!constants) constants = process.binding('constants');\n\nvar LF = '\\n'.charCodeAt(0);\nvar Pipe;\n\n\n// constructors for lazy loading\nfunction createPipe(ipc) {\n  // Lazy load\n  if (!Pipe) {\n//BEGIN *gearbox\n    //Pipe = process.binding('pipe_wrap').Pipe;\n    Pipe = require('pipe').Pipe;\n//END *gearbox\n  }\n\n  return new Pipe(ipc);\n}\n\nfunction createSocket(pipe, readable) {\n  var s = new net.Socket({ handle: pipe });\n\n  if (readable) {\n    s.writable = false;\n    s.readable = true;\n    s.resume();\n  } else {\n    s.writable = true;\n    s.readable = false;\n  }\n\n  return s;\n}\n\nfunction mergeOptions(target, overrides) {\n  if (overrides) {\n    var keys = Object.keys(overrides);\n    for (var i = 0, len = keys.length; i < len; i++) {\n      var k = keys[i];\n      if (overrides[k] !== undefined) {\n        target[k] = overrides[k];\n      }\n    }\n  }\n  return target;\n}\n\n\nfunction setupChannel(target, channel) {\n  var isWindows = process.platform === 'win32';\n  target._channel = channel;\n\n  var jsonBuffer = '';\n\n  if (isWindows) {\n    var setSimultaneousAccepts = function(handle) {\n      var simultaneousAccepts = (process.env.NODE_MANY_ACCEPTS\n        && process.env.NODE_MANY_ACCEPTS != '0') ? true : false;\n\n      if (handle._simultaneousAccepts != simultaneousAccepts) {\n        handle.setSimultaneousAccepts(simultaneousAccepts);\n        handle._simultaneousAccepts = simultaneousAccepts;\n      }\n    }\n  }\n\n  channel.onread = function(pool, offset, length, recvHandle) {\n    if (recvHandle && setSimultaneousAccepts) {\n      // Update simultaneous accepts on Windows\n      setSimultaneousAccepts(recvHandle);\n    }\n\n    if (pool) {\n      jsonBuffer += pool.toString('ascii', offset, offset + length);\n\n      var i, start = 0;\n      while ((i = jsonBuffer.indexOf('\\n', start)) >= 0) {\n        var json = jsonBuffer.slice(start, i);\n        var message = JSON.parse(json);\n\n        target.emit('message', message, recvHandle);\n        start = i+1;\n      }\n      jsonBuffer = jsonBuffer.slice(start);\n\n    } else {\n      channel.close();\n      target._channel = null;\n    }\n  };\n\n  target.send = function(message, sendHandle) {\n    if (!target._channel) throw new Error(\"channel closed\");\n\n    // For overflow protection don't write if channel queue is too deep.\n    if (channel.writeQueueSize > 1024 * 1024) {\n      return false;\n    }\n\n    var buffer = Buffer(JSON.stringify(message) + '\\n');\n\n    if (sendHandle && setSimultaneousAccepts) {\n      // Update simultaneous accepts on Windows\n      setSimultaneousAccepts(sendHandle);\n    }\n\n    var writeReq = channel.write(buffer, 0, buffer.length, sendHandle);\n\n    if (!writeReq) {\n      throw new Error(errno + \" cannot write to IPC channel.\");\n    }\n\n    writeReq.oncomplete = nop;\n\n    return true;\n  };\n\n  channel.readStart();\n}\n\n\nfunction nop() { }\n\n\nexports.fork = function(modulePath, args, options) {\n  if (!options) options = {};\n\n  args = args ? args.slice(0) : [];\n  args.unshift(modulePath);\n\n  if (options.stdinStream) {\n    throw new Error(\"stdinStream not allowed for fork()\");\n  }\n\n  if (options.customFds) {\n    throw new Error(\"customFds not allowed for fork()\");\n  }\n\n  // Leave stdin open for the IPC channel. stdout and stderr should be the\n  // same as the parent's.\n  options.customFds = [ -1, 1, 2 ];\n\n  // Just need to set this - child process won't actually use the fd.\n  // For backwards compat - this can be changed to 'NODE_CHANNEL' before v0.6.\n  if (!options.env) options.env = { };\n  options.env.NODE_CHANNEL_FD = 42;\n\n  // stdin is the IPC channel.\n  options.stdinStream = createPipe(true);\n\n  var child = spawn(process.execPath, args, options);\n\n  setupChannel(child, options.stdinStream);\n\n  child.on('exit', function() {\n    if (child._channel) {\n      child._channel.close();\n    }\n  });\n\n  return child;\n};\n\n\nexports._forkChild = function() {\n  // set process.send()\n  var p = createPipe(true);\n  p.open(0);\n  setupChannel(process, p);\n};\n\n\nexports.exec = function(command /*, options, callback */) {\n  var file, args, options, callback;\n\n  if (typeof arguments[1] === 'function') {\n    options = undefined;\n    callback = arguments[1];\n  } else {\n    options = arguments[1];\n    callback = arguments[2];\n  }\n\n  if (process.platform === 'win32') {\n    file = 'cmd.exe';\n    args = ['/s', '/c', '\"' + command + '\"'];\n    // Make a shallow copy before patching so we don't clobber the user's\n    // options object.\n    options = mergeOptions({}, options);\n    options.windowsVerbatimArguments = true;\n  } else {\n    file = '/bin/sh';\n    args = ['-c', command];\n  }\n  return exports.execFile(file, args, options, callback);\n};\n\n\nexports.execFile = function(file /* args, options, callback */) {\n  var args, optionArg, callback;\n  var options = {\n    encoding: 'utf8',\n    timeout: 0,\n    maxBuffer: 200 * 1024,\n    killSignal: 'SIGTERM',\n    setsid: false,\n    cwd: null,\n    env: null\n  };\n\n  // Parse the parameters.\n\n  if (typeof arguments[arguments.length - 1] === 'function') {\n    callback = arguments[arguments.length - 1];\n  }\n\n  if (Array.isArray(arguments[1])) {\n    args = arguments[1];\n    if (typeof arguments[2] === 'object') optionArg = arguments[2];\n  } else {\n    args = [];\n    if (typeof arguments[1] === 'object') optionArg = arguments[1];\n  }\n\n  // Merge optionArg into options\n  mergeOptions(options, optionArg);\n\n  var child = spawn(file, args, {\n    cwd: options.cwd,\n    env: options.env,\n    windowsVerbatimArguments: !!options.windowsVerbatimArguments\n  });\n\n  var stdout = '';\n  var stderr = '';\n  var killed = false;\n  var exited = false;\n  var timeoutId;\n\n  var err;\n\n  function exithandler(code, signal) {\n    if (exited) return;\n    exited = true;\n\n    if (timeoutId) {\n      clearTimeout(timeoutId);\n      timeoutId = null;\n    }\n\n    if (!callback) return;\n\n    if (err) {\n      callback(err, stdout, stderr);\n    } else if (code === 0 && signal === null) {\n      callback(null, stdout, stderr);\n    } else {\n      var e = new Error('Command failed: ' + stderr);\n      e.killed = child.killed || killed;\n      e.code = code;\n      e.signal = signal;\n      callback(e, stdout, stderr);\n    }\n  }\n\n  function kill() {\n    killed = true;\n    child.kill(options.killSignal);\n    process.nextTick(function() {\n      exithandler(null, options.killSignal);\n    });\n  }\n\n  if (options.timeout > 0) {\n    timeoutId = setTimeout(function() {\n      kill();\n      timeoutId = null;\n    }, options.timeout);\n  }\n\n  child.stdout.setEncoding(options.encoding);\n  child.stderr.setEncoding(options.encoding);\n\n  child.stdout.addListener('data', function(chunk) {\n    stdout += chunk;\n    if (stdout.length > options.maxBuffer) {\n      err = new Error('maxBuffer exceeded.');\n      kill();\n    }\n  });\n\n  child.stderr.addListener('data', function(chunk) {\n    stderr += chunk;\n    if (stderr.length > options.maxBuffer) {\n      err = new Error('maxBuffer exceeded.');\n      kill();\n    }\n  });\n\n  child.addListener('exit', exithandler);\n\n  return child;\n};\n\n\nvar spawn = exports.spawn = function(file, args, options) {\n  args = args ? args.slice(0) : [];\n  args.unshift(file);\n\n  var env = (options ? options.env : null) || process.env;\n  var envPairs = [];\n  var keys = Object.keys(env);\n  for (var key in env) {\n    envPairs.push(key + '=' + env[key]);\n  }\n\n  var child = new ChildProcess();\n\n  child.spawn({\n    file: file,\n    args: args,\n    cwd: options ? options.cwd : null,\n    windowsVerbatimArguments: !!(options && options.windowsVerbatimArguments),\n    envPairs: envPairs,\n    customFds: options ? options.customFds : null,\n    stdinStream: options ? options.stdinStream : null\n  });\n\n  return child;\n};\n\n\nfunction maybeExit(subprocess) {\n  subprocess._closesGot++;\n\n  if (subprocess._closesGot == subprocess._closesNeeded) {\n    subprocess.emit('exit', subprocess.exitCode, subprocess.signalCode);\n  }\n}\n\n\nfunction ChildProcess() {\n  var self = this;\n\n  this._closesNeeded = 1;\n  this._closesGot = 0;\n\n  this.signalCode = null;\n  this.exitCode = null;\n  this.killed = false;\n\n  this._internal = new Process();\n  this._internal.onexit = function(exitCode, signalCode) {\n    //\n    // follow 0.4.x behaviour:\n    //\n    // - normally terminated processes don't touch this.signalCode\n    // - signaled processes don't touch this.exitCode\n    //\n    if (signalCode) {\n      self.signalCode = signalCode;\n    } else {\n      self.exitCode = exitCode;\n    }\n\n    if (self.stdin) {\n      self.stdin.destroy();\n    }\n\n    self._internal.close();\n    self._internal = null;\n\n    maybeExit(self);\n  };\n}\ninherits(ChildProcess, EventEmitter);\n\n\nfunction setStreamOption(name, index, options) {\n  // Skip if we already have options.stdinStream\n  if (options[name]) return;\n\n  if (options.customFds &&\n      typeof options.customFds[index] == 'number' &&\n      options.customFds[index] !== -1) {\n    if (options.customFds[index] === index) {\n      options[name] = null;\n    } else {\n      throw new Error('customFds not yet supported');\n    }\n  } else {\n    options[name] = createPipe();\n  }\n}\n\n\nChildProcess.prototype.spawn = function(options) {\n  var self = this;\n\n  setStreamOption('stdinStream', 0, options);\n  setStreamOption('stdoutStream', 1, options);\n  setStreamOption('stderrStream', 2, options);\n\n  var r = this._internal.spawn(options);\n\n  if (r) {\n    if (options.stdinStream) {\n      options.stdinStream.close();\n    }\n\n    if (options.stdoutStream) {\n      options.stdoutStream.close();\n    }\n\n    if (options.stderrStream) {\n      options.stderrStream.close();\n    }\n\n    this._internal.close();\n    this._internal = null;\n    throw errnoException(errno, 'spawn');\n  }\n\n  this.pid = this._internal.pid;\n\n  if (options.stdinStream) {\n    this.stdin = createSocket(options.stdinStream, false);\n  }\n\n  if (options.stdoutStream) {\n    this.stdout = createSocket(options.stdoutStream, true);\n    this._closesNeeded++;\n    this.stdout.on('close', function() {\n      maybeExit(self);\n    });\n  }\n\n  if (options.stderrStream) {\n    this.stderr = createSocket(options.stderrStream, true);\n    this._closesNeeded++;\n    this.stderr.on('close', function() {\n      maybeExit(self);\n    });\n  }\n\n  return r;\n};\n\n\nfunction errnoException(errorno, syscall) {\n  // TODO make this more compatible with ErrnoException from src/node.cc\n  // Once all of Node is using this function the ErrnoException from\n  // src/node.cc should be removed.\n  var e = new Error(syscall + ' ' + errorno);\n  e.errno = e.code = errorno;\n  e.syscall = syscall;\n  return e;\n}\n\n\nChildProcess.prototype.kill = function(sig) {\n  if (!constants) {\n//BEGIN *gearbox\n    //constants = process.binding('constants');\n    constants = require('constants');\n//END *gearbox\n  }\n\n  sig = sig || 'SIGTERM';\n  var signal = constants[sig];\n\n  if (!signal) {\n    throw new Error('Unknown signal: ' + sig);\n  }\n\n  if (this._internal) {\n    this.killed = true;\n    var r = this._internal.kill(signal);\n    // TODO: raise error if r == -1?\n  }\n};\n//END lib/child_process.js\n    })", "gear:child_process")(exports, require, module, process_wrap);
}
static NativeModule _module_child_process("child_process", _setup_child_process);