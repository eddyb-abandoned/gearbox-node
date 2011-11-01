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

/** \file src/modules/url.cc converted from src/modules/url.js */

#line 1 "src/modules/url.js"


#line 31 "src/modules/url.js"
static void _setup_url(Value exports, Value require, Value module) {
    Context::getCurrent()->runScript("(function(exports, require, module){\n\nvar punycode = require('punycode');\n\nexports.parse = urlParse;\nexports.resolve = urlResolve;\nexports.resolveObject = urlResolveObject;\nexports.format = urlFormat;\n\n// Reference: RFC 3986, RFC 1808, RFC 2396\n\n// define these here so at least they only have to be\n// compiled once on the first module load.\nvar protocolPattern = /^([a-z0-9+]+:)/i,\n    portPattern = /:[0-9]+$/,\n    // RFC 2396: characters reserved for delimiting URLs.\n    delims = ['<', '>', '\"', '`', ' ', '\\r', '\\n', '\\t'],\n    // RFC 2396: characters not allowed for various reasons.\n    unwise = ['{', '}', '|', '\\\\', '^', '~', '[', ']', '`'].concat(delims),\n    // Allowed by RFCs, but cause of XSS attacks.  Always escape these.\n    autoEscape = ['\\''],\n    // Characters that are never ever allowed in a hostname.\n    // Note that any invalid chars are also handled, but these\n    // are the ones that are *expected* to be seen, so we fast-path\n    // them.\n    nonHostChars = ['%', '/', '?', ';', '#']\n      .concat(unwise).concat(autoEscape),\n    nonAuthChars = ['/', '@', '?', '#'].concat(delims),\n    hostnameMaxLen = 255,\n    hostnamePartPattern = /^[a-zA-Z0-9][a-z0-9A-Z_-]{0,62}$/,\n    hostnamePartStart = /^([a-zA-Z0-9][a-z0-9A-Z_-]{0,62})(.*)$/,\n    // protocols that can allow \"unsafe\" and \"unwise\" chars.\n    unsafeProtocol = {\n      'javascript': true,\n      'javascript:': true\n    },\n    // protocols that never have a hostname.\n    hostlessProtocol = {\n      'javascript': true,\n      'javascript:': true\n    },\n    // protocols that always have a path component.\n    pathedProtocol = {\n      'http': true,\n      'https': true,\n      'ftp': true,\n      'gopher': true,\n      'file': true,\n      'http:': true,\n      'ftp:': true,\n      'gopher:': true,\n      'file:': true\n    },\n    // protocols that always contain a // bit.\n    slashedProtocol = {\n      'http': true,\n      'https': true,\n      'ftp': true,\n      'gopher': true,\n      'file': true,\n      'http:': true,\n      'https:': true,\n      'ftp:': true,\n      'gopher:': true,\n      'file:': true\n    },\n    querystring = require('querystring');\n\nfunction urlParse(url, parseQueryString, slashesDenoteHost) {\n  if (url && typeof(url) === 'object' && url.href) return url;\n\n  if (typeof url !== 'string') {\n    throw new TypeError(\"Parameter 'url' must be a string, not \" + typeof url);\n  }\n\n  var out = {},\n      rest = url;\n\n  // cut off any delimiters.\n  // This is to support parse stuff like \"<http://foo.com>\"\n  for (var i = 0, l = rest.length; i < l; i++) {\n    if (delims.indexOf(rest.charAt(i)) === -1) break;\n  }\n  if (i !== 0) rest = rest.substr(i);\n\n\n  var proto = protocolPattern.exec(rest);\n  if (proto) {\n    proto = proto[0];\n    var lowerProto = proto.toLowerCase();\n    out.protocol = lowerProto;\n    rest = rest.substr(proto.length);\n  }\n\n  // figure out if it's got a host\n  // user@server is *always* interpreted as a hostname, and url\n  // resolution will treat //foo/bar as host=foo,path=bar because that's\n  // how the browser resolves relative URLs.\n  if (slashesDenoteHost || proto || rest.match(/^\\/\\/[^@\\/]+@[^@\\/]+/)) {\n    var slashes = rest.substr(0, 2) === '//';\n    if (slashes && !(proto && hostlessProtocol[proto])) {\n      rest = rest.substr(2);\n      out.slashes = true;\n    }\n  }\n\n  if (!hostlessProtocol[proto] &&\n      (slashes || (proto && !slashedProtocol[proto]))) {\n    // there's a hostname.\n    // the first instance of /, ?, ;, or # ends the host.\n    // don't enforce full RFC correctness, just be unstupid about it.\n\n    // If there is an @ in the hostname, then non-host chars *are* allowed\n    // to the left of the first @ sign, unless some non-auth character\n    // comes *before* the @-sign.\n    // URLs are obnoxious.\n    var atSign = rest.indexOf('@');\n    if (atSign !== -1) {\n      // there *may be* an auth\n      var hasAuth = true;\n      for (var i = 0, l = nonAuthChars.length; i < l; i++) {\n        var index = rest.indexOf(nonAuthChars[i]);\n        if (index !== -1 && index < atSign) {\n          // not a valid auth.  Something like http://foo.com/bar@baz/\n          hasAuth = false;\n          break;\n        }\n      }\n      if (hasAuth) {\n        // pluck off the auth portion.\n        out.auth = rest.substr(0, atSign);\n        rest = rest.substr(atSign + 1);\n      }\n    }\n\n    var firstNonHost = -1;\n    for (var i = 0, l = nonHostChars.length; i < l; i++) {\n      var index = rest.indexOf(nonHostChars[i]);\n      if (index !== -1 &&\n          (firstNonHost < 0 || index < firstNonHost)) firstNonHost = index;\n    }\n\n    if (firstNonHost !== -1) {\n      out.host = rest.substr(0, firstNonHost);\n      rest = rest.substr(firstNonHost);\n    } else {\n      out.host = rest;\n      rest = '';\n    }\n\n    // pull out port.\n    var p = parseHost(out.host);\n    var keys = Object.keys(p);\n    for (var i = 0, l = keys.length; i < l; i++) {\n      var key = keys[i];\n      out[key] = p[key];\n    }\n\n    // we've indicated that there is a hostname,\n    // so even if it's empty, it has to be present.\n    out.hostname = out.hostname || '';\n\n    // validate a little.\n    if (out.hostname.length > hostnameMaxLen) {\n      out.hostname = '';\n    } else {\n      var hostparts = out.hostname.split(/\\./);\n      for (var i = 0, l = hostparts.length; i < l; i++) {\n        var part = hostparts[i];\n        if (!part) continue;\n        if (!part.match(hostnamePartPattern)) {\n          var newpart = '';\n          for (var j = 0, k = part.length; j < k; j++) {\n            if (part.charCodeAt(j) > 127) {\n              // we replace non-ASCII char with a temporary placeholder\n              // we need this to make sure size of hostname is not\n              // broken by replacing non-ASCII by nothing\n              newpart += 'x';\n            } else {\n              newpart += part[j];\n            }\n          }\n          // we test again with ASCII char only\n          if (!newpart.match(hostnamePartPattern)) {\n            var validParts = hostparts.slice(0, i);\n            var notHost = hostparts.slice(i + 1);\n            var bit = part.match(hostnamePartStart);\n            if (bit) {\n              validParts.push(bit[1]);\n              notHost.unshift(bit[2]);\n            }\n            if (notHost.length) {\n              rest = '/' + notHost.join('.') + rest;\n            }\n            out.hostname = validParts.join('.');\n            break;\n          }\n        }\n      }\n    }\n\n    // hostnames are always lower case.\n    out.hostname = out.hostname.toLowerCase();\n\n    // IDNA Support: Returns a puny coded representation of \"domain\".\n    // It only converts the part of the domain name that\n    // has non ASCII characters. I.e. it dosent matter if\n    // you call it with a domain that already is in ASCII.\n    var domainArray = out.hostname.split('.');\n    var newOut = [];\n    for (var i = 0; i < domainArray.length; ++i) {\n      var s = domainArray[i];\n      newOut.push(s.match(/[^A-Za-z0-9_-]/) ?\n          'xn--' + punycode.encode(s) : s);\n    }\n    out.hostname = newOut.join('.');\n\n    out.host = (out.hostname || '') +\n        ((out.port) ? ':' + out.port : '');\n    out.href += out.host;\n  }\n\n  // now rest is set to the post-host stuff.\n  // chop off any delim chars.\n  if (!unsafeProtocol[lowerProto]) {\n\n    // First, make 100% sure that any \"autoEscape\" chars get\n    // escaped, even if encodeURIComponent doesn't think they\n    // need to be.\n    for (var i = 0, l = autoEscape.length; i < l; i++) {\n      var ae = autoEscape[i];\n      var esc = encodeURIComponent(ae);\n      if (esc === ae) {\n        esc = escape(ae);\n      }\n      rest = rest.split(ae).join(esc);\n    }\n\n    // Now make sure that delims never appear in a url.\n    var chop = rest.length;\n    for (var i = 0, l = delims.length; i < l; i++) {\n      var c = rest.indexOf(delims[i]);\n      if (c !== -1) {\n        chop = Math.min(c, chop);\n      }\n    }\n    rest = rest.substr(0, chop);\n  }\n\n\n  // chop off from the tail first.\n  var hash = rest.indexOf('#');\n  if (hash !== -1) {\n    // got a fragment string.\n    out.hash = rest.substr(hash);\n    rest = rest.slice(0, hash);\n  }\n  var qm = rest.indexOf('?');\n  if (qm !== -1) {\n    out.search = rest.substr(qm);\n    out.query = rest.substr(qm + 1);\n    if (parseQueryString) {\n      out.query = querystring.parse(out.query);\n    }\n    rest = rest.slice(0, qm);\n  } else if (parseQueryString) {\n    // no query string, but parseQueryString still requested\n    out.search = '';\n    out.query = {};\n  }\n  if (rest) out.pathname = rest;\n  if (slashedProtocol[proto] &&\n      out.hostname && !out.pathname) {\n    out.pathname = '/';\n  }\n\n  //to support http.request\n  if (out.pathname || out.search) {\n    out.path = (out.pathname ? out.pathname : '') +\n               (out.search ? out.search : '');\n  }\n\n  // finally, reconstruct the href based on what has been validated.\n  out.href = urlFormat(out);\n  return out;\n}\n\n// format a parsed object into a url string\nfunction urlFormat(obj) {\n  // ensure it's an object, and not a string url.\n  // If it's an obj, this is a no-op.\n  // this way, you can call url_format() on strings\n  // to clean up potentially wonky urls.\n  if (typeof(obj) === 'string') obj = urlParse(obj);\n\n  var auth = obj.auth || '';\n  if (auth) {\n    auth = auth.split('@').join('%40');\n    for (var i = 0, l = nonAuthChars.length; i < l; i++) {\n      var nAC = nonAuthChars[i];\n      auth = auth.split(nAC).join(encodeURIComponent(nAC));\n    }\n    auth += '@';\n  }\n\n  var protocol = obj.protocol || '',\n      host = (obj.host !== undefined) ? auth + obj.host :\n          obj.hostname !== undefined ? (\n              auth + obj.hostname +\n              (obj.port ? ':' + obj.port : '')\n          ) :\n          false,\n      pathname = obj.pathname || '',\n      query = obj.query &&\n              ((typeof obj.query === 'object' &&\n                Object.keys(obj.query).length) ?\n                 querystring.stringify(obj.query) :\n                 '') || '',\n      search = obj.search || (query && ('?' + query)) || '',\n      hash = obj.hash || '';\n\n  if (protocol && protocol.substr(-1) !== ':') protocol += ':';\n\n  // only the slashedProtocols get the //.  Not mailto:, xmpp:, etc.\n  // unless they had them to begin with.\n  if (obj.slashes ||\n      (!protocol || slashedProtocol[protocol]) && host !== false) {\n    host = '//' + (host || '');\n    if (pathname && pathname.charAt(0) !== '/') pathname = '/' + pathname;\n  } else if (!host) {\n    host = '';\n  }\n\n  if (hash && hash.charAt(0) !== '#') hash = '#' + hash;\n  if (search && search.charAt(0) !== '?') search = '?' + search;\n\n  return protocol + host + pathname + search + hash;\n}\n\nfunction urlResolve(source, relative) {\n  return urlFormat(urlResolveObject(source, relative));\n}\n\nfunction urlResolveObject(source, relative) {\n  if (!source) return relative;\n\n  source = urlParse(urlFormat(source), false, true);\n  relative = urlParse(urlFormat(relative), false, true);\n\n  // hash is always overridden, no matter what.\n  source.hash = relative.hash;\n\n  if (relative.href === '') {\n    source.href = urlFormat(source);\n    return source;\n  }\n\n  // hrefs like //foo/bar always cut to the protocol.\n  if (relative.slashes && !relative.protocol) {\n    relative.protocol = source.protocol;\n    //urlParse appends trailing / to urls like http://www.example.com\n    if (slashedProtocol[relative.protocol] &&\n        relative.hostname && !relative.pathname) {\n      relative.path = relative.pathname = '/';\n    }\n    relative.href = urlFormat(relative);\n    return relative;\n  }\n\n  if (relative.protocol && relative.protocol !== source.protocol) {\n    // if it's a known url protocol, then changing\n    // the protocol does weird things\n    // first, if it's not file:, then we MUST have a host,\n    // and if there was a path\n    // to begin with, then we MUST have a path.\n    // if it is file:, then the host is dropped,\n    // because that's known to be hostless.\n    // anything else is assumed to be absolute.\n    if (!slashedProtocol[relative.protocol]) {\n      relative.href = urlFormat(relative);\n      return relative;\n    }\n    source.protocol = relative.protocol;\n    if (!relative.host && !hostlessProtocol[relative.protocol]) {\n      var relPath = (relative.pathname || '').split('/');\n      while (relPath.length && !(relative.host = relPath.shift()));\n      if (!relative.host) relative.host = '';\n      if (!relative.hostname) relative.hostname = '';\n      if (relPath[0] !== '') relPath.unshift('');\n      if (relPath.length < 2) relPath.unshift('');\n      relative.pathname = relPath.join('/');\n    }\n    source.pathname = relative.pathname;\n    source.search = relative.search;\n    source.query = relative.query;\n    source.host = relative.host || '';\n    source.auth = relative.auth;\n    source.hostname = relative.hostname || relative.host;\n    source.port = relative.port;\n    //to support http.request\n    if (source.pathname !== undefined || source.search !== undefined) {\n      source.path = (source.pathname ? source.pathname : '') +\n                    (source.search ? source.search : '');\n    }\n    source.slashes = source.slashes || relative.slashes;\n    source.href = urlFormat(source);\n    return source;\n  }\n\n  var isSourceAbs = (source.pathname && source.pathname.charAt(0) === '/'),\n      isRelAbs = (\n          relative.host !== undefined ||\n          relative.pathname && relative.pathname.charAt(0) === '/'\n      ),\n      mustEndAbs = (isRelAbs || isSourceAbs ||\n                    (source.host && relative.pathname)),\n      removeAllDots = mustEndAbs,\n      srcPath = source.pathname && source.pathname.split('/') || [],\n      relPath = relative.pathname && relative.pathname.split('/') || [],\n      psychotic = source.protocol &&\n          !slashedProtocol[source.protocol];\n\n  // if the url is a non-slashed url, then relative\n  // links like ../.. should be able\n  // to crawl up to the hostname, as well.  This is strange.\n  // source.protocol has already been set by now.\n  // Later on, put the first path part into the host field.\n  if (psychotic) {\n\n    delete source.hostname;\n    delete source.port;\n    if (source.host) {\n      if (srcPath[0] === '') srcPath[0] = source.host;\n      else srcPath.unshift(source.host);\n    }\n    delete source.host;\n    if (relative.protocol) {\n      delete relative.hostname;\n      delete relative.port;\n      if (relative.host) {\n        if (relPath[0] === '') relPath[0] = relative.host;\n        else relPath.unshift(relative.host);\n      }\n      delete relative.host;\n    }\n    mustEndAbs = mustEndAbs && (relPath[0] === '' || srcPath[0] === '');\n  }\n\n  if (isRelAbs) {\n    // it's absolute.\n    source.host = (relative.host || relative.host === '') ?\n                      relative.host : source.host;\n    source.hostname = (relative.hostname || relative.hostname === '') ?\n                      relative.hostname : source.hostname;\n    source.search = relative.search;\n    source.query = relative.query;\n    srcPath = relPath;\n    // fall through to the dot-handling below.\n  } else if (relPath.length) {\n    // it's relative\n    // throw away the existing file, and take the new path instead.\n    if (!srcPath) srcPath = [];\n    srcPath.pop();\n    srcPath = srcPath.concat(relPath);\n    source.search = relative.search;\n    source.query = relative.query;\n  } else if ('search' in relative) {\n    // just pull out the search.\n    // like href='?foo'.\n    // Put this after the other two cases because it simplifies the booleans\n    if (psychotic) {\n      source.hostname = source.host = srcPath.shift();\n      //occationaly the auth can get stuck only in host\n      //this especialy happens in cases like\n      //url.resolveObject('mailto:local1@domain1', 'local2@domain2')\n      var authInHost = source.host && source.host.indexOf('@') > 0 ?\n                       source.host.split('@') : false;\n      if (authInHost) {\n        source.auth = authInHost.shift();\n        source.host = source.hostname = authInHost.shift();\n      }\n    }\n    source.search = relative.search;\n    source.query = relative.query;\n    //to support http.request\n    if (source.pathname !== undefined || source.search !== undefined) {\n      source.path = (source.pathname ? source.pathname : '') +\n                    (source.search ? source.search : '');\n    }\n    source.href = urlFormat(source);\n    return source;\n  }\n  if (!srcPath.length) {\n    // no path at all.  easy.\n    // we've already handled the other stuff above.\n    delete source.pathname;\n    //to support http.request\n    if (!source.search) {\n      source.path = '/' + source.search;\n    } else {\n      delete source.path;\n    }\n    source.href = urlFormat(source);\n    return source;\n  }\n  // if a url ENDs in . or .., then it must get a trailing slash.\n  // however, if it ends in anything else non-slashy,\n  // then it must NOT get a trailing slash.\n  var last = srcPath.slice(-1)[0];\n  var hasTrailingSlash = (\n      (source.host || relative.host) && (last === '.' || last === '..') ||\n      last === '');\n\n  // strip single dots, resolve double dots to parent dir\n  // if the path tries to go above the root, `up` ends up > 0\n  var up = 0;\n  for (var i = srcPath.length; i >= 0; i--) {\n    last = srcPath[i];\n    if (last == '.') {\n      srcPath.splice(i, 1);\n    } else if (last === '..') {\n      srcPath.splice(i, 1);\n      up++;\n    } else if (up) {\n      srcPath.splice(i, 1);\n      up--;\n    }\n  }\n\n  // if the path is allowed to go above the root, restore leading ..s\n  if (!mustEndAbs && !removeAllDots) {\n    for (; up--; up) {\n      srcPath.unshift('..');\n    }\n  }\n\n  if (mustEndAbs && srcPath[0] !== '' &&\n      (!srcPath[0] || srcPath[0].charAt(0) !== '/')) {\n    srcPath.unshift('');\n  }\n\n  if (hasTrailingSlash && (srcPath.join('/').substr(-1) !== '/')) {\n    srcPath.push('');\n  }\n\n  var isAbsolute = srcPath[0] === '' ||\n      (srcPath[0] && srcPath[0].charAt(0) === '/');\n\n  // put the host back\n  if (psychotic) {\n    source.hostname = source.host = isAbsolute ? '' :\n                                    srcPath.length ? srcPath.shift() : '';\n    //occationaly the auth can get stuck only in host\n    //this especialy happens in cases like\n    //url.resolveObject('mailto:local1@domain1', 'local2@domain2')\n    var authInHost = source.host && source.host.indexOf('@') > 0 ?\n                     source.host.split('@') : false;\n    if (authInHost) {\n      source.auth = authInHost.shift();\n      source.host = source.hostname = authInHost.shift();\n    }\n  }\n\n  mustEndAbs = mustEndAbs || (source.host && srcPath.length);\n\n  if (mustEndAbs && !isAbsolute) {\n    srcPath.unshift('');\n  }\n\n  source.pathname = srcPath.join('/');\n  //to support request.http\n  if (source.pathname !== undefined || source.search !== undefined) {\n    source.path = (source.pathname ? source.pathname : '') +\n                  (source.search ? source.search : '');\n  }\n  source.auth = relative.auth || source.auth;\n  source.slashes = source.slashes || relative.slashes;\n  source.href = urlFormat(source);\n  return source;\n}\n\nfunction parseHost(host) {\n  var out = {};\n  var port = portPattern.exec(host);\n  if (port) {\n    port = port[0];\n    out.port = port.substr(1);\n    host = host.substr(0, host.length - port.length);\n  }\n  if (host) out.hostname = host;\n  return out;\n}\n})", "gear:url")(exports, require, module);
}
static NativeModule _module_url("url", _setup_url);