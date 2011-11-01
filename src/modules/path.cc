// Copyright Joyent, Inc. and other Node contributors.
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

/** \file src/modules/path.cc converted from src/modules/path.js */

#line 1 "src/modules/path.js"


#line 31 "src/modules/path.js"
static void _setup_path(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\n\nvar isWindows = process.platform === 'win32';\n\n\n// resolves . and .. elements in a path array with directory names there\n// must be no slashes, empty elements, or device names (c:\\) in the array\n// (so also no leading and trailing slashes - it does not distinguish\n// relative and absolute paths)\nfunction normalizeArray(parts, allowAboveRoot) {\n  // if the path tries to go above the root, `up` ends up > 0\n  var up = 0;\n  for (var i = parts.length - 1; i >= 0; i--) {\n    var last = parts[i];\n    if (last == '.') {\n      parts.splice(i, 1);\n    } else if (last === '..') {\n      parts.splice(i, 1);\n      up++;\n    } else if (up) {\n      parts.splice(i, 1);\n      up--;\n    }\n  }\n\n  // if the path is allowed to go above the root, restore leading ..s\n  if (allowAboveRoot) {\n    for (; up--; up) {\n      parts.unshift('..');\n    }\n  }\n\n  return parts;\n}\n\n\nif (isWindows) {\n  // Regex to split a windows path into three parts: [*, device, slash,\n  // tail] windows-only\n  var splitDeviceRe =\n      /^([a-zA-Z]:|[\\\\\\/]{2}[^\\\\\\/]+[\\\\\\/][^\\\\\\/]+)?([\\\\\\/])?([\\s\\S]*?)$/;\n\n  // Regex to split the tail part of the above into [*, dir, basename, ext]\n  var splitTailRe = /^([\\s\\S]+[\\\\\\/](?!$)|[\\\\\\/])?((?:[\\s\\S]+?)?(\\.[^.]*)?)$/;\n\n  // Function to split a filename into [root, dir, basename, ext]\n  // windows version\n  var splitPath = function(filename) {\n    // Separate device+slash from tail\n    var result = splitDeviceRe.exec(filename),\n        device = (result[1] || '') + (result[2] || ''),\n        tail = result[3] || '';\n    // Split the tail into dir, basename and extension\n    var result2 = splitTailRe.exec(tail),\n        dir = result2[1] || '',\n        basename = result2[2] || '',\n        ext = result2[3] || '';\n    return [device, dir, basename, ext];\n  };\n\n  // path.resolve([from ...], to)\n  // windows version\n  exports.resolve = function() {\n    var resolvedDevice = '',\n        resolvedTail = '',\n        resolvedAbsolute = false;\n\n    for (var i = arguments.length - 1; i >= -1; i--) {\n      var path = (i >= 0) ? arguments[i] : process.cwd();\n\n      // Skip empty and invalid entries\n      if (typeof path !== 'string' || !path) {\n        continue;\n      }\n\n      var result = splitDeviceRe.exec(path),\n          device = result[1] || '',\n          isUnc = device && device.charAt(1) !== ':',\n          isAbsolute = !!result[2] || isUnc, // UNC paths are always absolute\n          tail = result[3];\n\n      if (device &&\n          resolvedDevice &&\n          device.toLowerCase() !== resolvedDevice.toLowerCase()) {\n        // This path points to another device so it is not applicable\n        continue;\n      }\n\n      if (!resolvedDevice) {\n        resolvedDevice = device;\n      }\n      if (!resolvedAbsolute) {\n        resolvedTail = tail + '\\\\' + resolvedTail;\n        resolvedAbsolute = isAbsolute;\n      }\n\n      if (resolvedDevice && resolvedAbsolute) {\n        break;\n      }\n    }\n\n    if (!resolvedAbsolute && resolvedDevice) {\n      // If we still don't have an absolute path,\n      // prepend the current path for the device found.\n\n      // TODO\n      // Windows stores the current directories for 'other' drives\n      // as hidden environment variables like =C:=c:\\windows (literally)\n      // var deviceCwd = os.getCwdForDrive(resolvedDevice);\n      var deviceCwd = '';\n\n      // If there is no cwd set for the drive, it is at root\n      resolvedTail = deviceCwd + '\\\\' + resolvedTail;\n      resolvedAbsolute = true;\n    }\n\n    // Replace slashes (in UNC share name) by backslashes\n    resolvedDevice = resolvedDevice.replace(/\\//g, '\\\\');\n\n    // At this point the path should be resolved to a full absolute path,\n    // but handle relative paths to be safe (might happen when process.cwd()\n    // fails)\n\n    // Normalize the tail path\n\n    function f(p) {\n      return !!p;\n    }\n\n    resolvedTail = normalizeArray(resolvedTail.split(/[\\\\\\/]+/).filter(f),\n                                  !resolvedAbsolute).join('\\\\');\n\n    return (resolvedDevice + (resolvedAbsolute ? '\\\\' : '') + resolvedTail) ||\n           '.';\n  };\n\n  // windows version\n  exports.normalize = function(path) {\n    var result = splitDeviceRe.exec(path),\n        device = result[1] || '',\n        isUnc = device && device.charAt(1) !== ':',\n        isAbsolute = !!result[2] || isUnc, // UNC paths are always absolute\n        tail = result[3],\n        trailingSlash = /[\\\\\\/]$/.test(tail);\n\n    // Normalize the tail path\n    tail = normalizeArray(tail.split(/[\\\\\\/]+/).filter(function(p) {\n      return !!p;\n    }), !isAbsolute).join('\\\\');\n\n    if (!tail && !isAbsolute) {\n      tail = '.';\n    }\n    if (tail && trailingSlash) {\n      tail += '\\\\';\n    }\n\n    return device + (isAbsolute ? '\\\\' : '') + tail;\n  };\n\n  // windows version\n  exports.join = function() {\n    function f(p) {\n      return p && typeof p === 'string';\n    }\n\n    var paths = Array.prototype.slice.call(arguments, 0).filter(f);\n    var joined = paths.join('\\\\');\n\n    // Make sure that the joined path doesn't start with two slashes\n    // - it will be mistaken for an unc path by normalize() -\n    // unless the paths[0] also starts with two slashes\n    if (/^[\\\\\\/]{2}/.test(joined) && !/^[\\\\\\/]{2}/.test(paths[0])) {\n      joined = joined.slice(1);\n    }\n\n    return exports.normalize(joined);\n  };\n\n  // path.relative(from, to)\n  // it will solve the relative path from 'from' to 'to', for instance:\n  // from = 'C:\\\\orandea\\\\test\\\\aaa'\n  // to = 'C:\\\\orandea\\\\impl\\\\bbb'\n  // The output of the function should be: '..\\\\..\\\\impl\\\\bbb'\n  // windows version\n  exports.relative = function(from, to) {\n    from = exports.resolve(from);\n    to = exports.resolve(to);\n\n    // windows is not case sensitive\n    var lowerFrom = from.toLowerCase();\n    var lowerTo = to.toLowerCase();\n\n    function trim(arr) {\n      var start = 0;\n      for (; start < arr.length; start++) {\n        if (arr[start] !== '') break;\n      }\n\n      var end = arr.length - 1;\n      for (; end >= 0; end--) {\n        if (arr[end] !== '') break;\n      }\n\n      if (start > end) return [];\n      return arr.slice(start, end - start + 1);\n    }\n\n    var fromParts = trim(from.split('\\\\'));\n    var toParts = trim(to.split('\\\\'));\n\n    var lowerFromParts = trim(lowerFrom.split('\\\\'));\n    var lowerToParts = trim(lowerTo.split('\\\\'));\n\n    var length = Math.min(lowerFromParts.length, lowerToParts.length);\n    var samePartsLength = length;\n    for (var i = 0; i < length; i++) {\n      if (lowerFromParts[i] !== lowerToParts[i]) {\n        samePartsLength = i;\n        break;\n      }\n    }\n\n    if (samePartsLength == 0) {\n      return to;\n    }\n\n    var outputParts = [];\n    for (var i = samePartsLength; i < lowerFromParts.length; i++) {\n      outputParts.push('..');\n    }\n\n    outputParts = outputParts.concat(toParts.slice(samePartsLength));\n\n    return outputParts.join('\\\\');\n  };\n\n\n} else /* posix */ {\n\n  // Split a filename into [root, dir, basename, ext], unix version\n  // 'root' is just a slash, or nothing.\n  var splitPathRe = /^(\\/?)([\\s\\S]+\\/(?!$)|\\/)?((?:[\\s\\S]+?)?(\\.[^.]*)?)$/;\n  var splitPath = function(filename) {\n    var result = splitPathRe.exec(filename);\n    return [result[1] || '', result[2] || '', result[3] || '', result[4] || ''];\n  };\n\n  // path.resolve([from ...], to)\n  // posix version\n  exports.resolve = function() {\n    var resolvedPath = '',\n        resolvedAbsolute = false;\n\n    for (var i = arguments.length - 1; i >= -1 && !resolvedAbsolute; i--) {\n      var path = (i >= 0) ? arguments[i] : process.cwd();\n\n      // Skip empty and invalid entries\n      if (typeof path !== 'string' || !path) {\n        continue;\n      }\n\n      resolvedPath = path + '/' + resolvedPath;\n      resolvedAbsolute = path.charAt(0) === '/';\n    }\n\n    // At this point the path should be resolved to a full absolute path, but\n    // handle relative paths to be safe (might happen when process.cwd() fails)\n\n    // Normalize the path\n    resolvedPath = normalizeArray(resolvedPath.split('/').filter(function(p) {\n      return !!p;\n    }), !resolvedAbsolute).join('/');\n\n    return ((resolvedAbsolute ? '/' : '') + resolvedPath) || '.';\n  };\n\n  // path.normalize(path)\n  // posix version\n  exports.normalize = function(path) {\n    var isAbsolute = path.charAt(0) === '/',\n        trailingSlash = path.slice(-1) === '/';\n\n    // Normalize the path\n    path = normalizeArray(path.split('/').filter(function(p) {\n      return !!p;\n    }), !isAbsolute).join('/');\n\n    if (!path && !isAbsolute) {\n      path = '.';\n    }\n    if (path && trailingSlash) {\n      path += '/';\n    }\n\n    return (isAbsolute ? '/' : '') + path;\n  };\n\n\n  // posix version\n  exports.join = function() {\n    var paths = Array.prototype.slice.call(arguments, 0);\n    return exports.normalize(paths.filter(function(p, index) {\n      return p && typeof p === 'string';\n    }).join('/'));\n  };\n\n\n  // path.relative(from, to)\n  // posix version\n  exports.relative = function(from, to) {\n    from = exports.resolve(from).substr(1);\n    to = exports.resolve(to).substr(1);\n\n    function trim(arr) {\n      var start = 0;\n      for (; start < arr.length; start++) {\n        if (arr[start] !== '') break;\n      }\n\n      var end = arr.length - 1;\n      for (; end >= 0; end--) {\n        if (arr[end] !== '') break;\n      }\n\n      if (start > end) return [];\n      return arr.slice(start, end - start + 1);\n    }\n\n    var fromParts = trim(from.split('/'));\n    var toParts = trim(to.split('/'));\n\n    var length = Math.min(fromParts.length, toParts.length);\n    var samePartsLength = length;\n    for (var i = 0; i < length; i++) {\n      if (fromParts[i] !== toParts[i]) {\n        samePartsLength = i;\n        break;\n      }\n    }\n\n    var outputParts = [];\n    for (var i = samePartsLength; i < fromParts.length; i++) {\n      outputParts.push('..');\n    }\n\n    outputParts = outputParts.concat(toParts.slice(samePartsLength));\n\n    return outputParts.join('/');\n  };\n\n}\n\n\nexports.dirname = function(path) {\n  var result = splitPath(path),\n      root = result[0],\n      dir = result[1];\n\n  if (!root && !dir) {\n    // No dirname whatsoever\n    return '.';\n  }\n\n  if (dir) {\n    // It has a dirname, strip trailing slash\n    dir = dir.substring(0, dir.length - 1);\n  }\n\n  return root + dir;\n};\n\n\nexports.basename = function(path, ext) {\n  var f = splitPath(path)[2];\n  // TODO: make this comparison case-insensitive on windows?\n  if (ext && f.substr(-1 * ext.length) === ext) {\n    f = f.substr(0, f.length - ext.length);\n  }\n  return f;\n};\n\n\nexports.extname = function(path) {\n  return splitPath(path)[3];\n};\n\n\nexports.exists = function(path, callback) {\n  process.binding('fs').stat(path, function(err, stats) {\n    if (callback) callback(err ? false : true);\n  });\n};\n\n\nexports.existsSync = function(path) {\n  try {\n    process.binding('fs').stat(path);\n    return true;\n  } catch (e) {\n    return false;\n  }\n};\n})", "gear:path")(exports, require, module);
}
static NativeModule _module_path("path", _setup_path);