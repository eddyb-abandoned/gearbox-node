var codeaze = eval(require("fs").readFileSync("tools/gear2cc/Codeaze.js", "utf8")); /** @todo require("./Codeaze.js") */
var $aze = new codeaze.Codeaze();


function createClass(name, childs, line, inheritList) {
    var out = {type:'class', name:name, inheritList:inheritList, classes:{}, vars:{}, nativeVars:{}, staticVars:{}, functions:{}, staticFunctions:{}, accessors:{}};
    for(c in childs) {
        var node = childs[c];
        switch(node.type) {
            case 'class':
                out.classes[node.name] = {classes:node.classes, vars:node.vars, nativeVars:node.nativeVars, staticVars:node.staticVars, functions:node.functions, staticFunctions:node.staticFunctions, accessors:node.accessors};
                break;
            case 'function':
                if(!out.functions[node.name])
                    out.functions[node.name] = [{args:node.args, code:node.code, line:node.line}];
                else
                    out.functions[node.name].push({args:node.args, code:node.code, line:node.line});
                break;
            case 'static-function':
                if(!out.staticFunctions[node.name])
                    out.staticFunctions[node.name] = [{args:node.args, code:node.code, line:node.line}];
                else
                    out.staticFunctions[node.name].push({args:node.args, code:node.code, line:node.line});
                break;
            case 'var':
                out.vars[node.name] = {val:node.val};
                break;
            case 'native-var':
                out.nativeVars[node.name] = {type:node._type, val:node.val};
                break;
            case 'static-var':
                out.staticVars[node.name] = {val:node.val};
                break;
            case 'getter':
                if(!out.accessors[node.name])
                    out.accessors[node.name] = {};
                out.accessors[node.name].getter = {args:node.args, code:node.code, line:node.line};
                break;
            case 'setter':
                if(!out.accessors[node.name])
                    out.accessors[node.name] = {};
                out.accessors[node.name].setter = {args:node.args, code:node.code, line:node.line};
                break;
            case 'native-block':
                throw Error('TODO: Native blocks in classes');
        }
    }
    
    if(!out.functions.hasOwnProperty(name))
        out.functions[name] = [{args:[], code:'', line:line}];
    
    return out;
}

function createObject(name, childs, line, isModule) {
    var out = {type:isModule?'module':'object', name:name, objects:{}, modules:{}, classes:{}, vars:{}, functions:{}, nativeBlocks:{}};
    for(c in childs) {
        var node = childs[c];
        switch(node.type) {
            case 'object':
                out.objects[node.name] = {objects:node.objects, classes:node.classes, vars:node.vars, functions:node.functions, nativeBlocks:node.nativeBlocks};
                break;
            case 'module':
                out.modules[node.name] = {objects:node.objects, classes:node.classes, vars:node.vars, functions:node.functions, nativeBlocks:node.nativeBlocks};
                break;
            case 'class':
                if(node.inheritList) {
                    var baseNode = {classes:{}, vars:{}, nativeVars:{}, staticVars:{}, functions:{}, staticFunctions:{}, accessors:{}};
                    function extend(x, nn) {
                        for(var j in x)
                            if(j in baseNode)
                                for(var k in x[j])
                                    if(!(j == 'functions' && k == nn))
                                        baseNode[j][k] = x[j][k];
                    }
                    for(var i = 0; i < node.inheritList.length; i++)
                        extend(out.classes[node.inheritList[i]], node.inheritList[i]);
                    extend(node);
                    out.classes[node.name] = baseNode;
                }
                else
                    out.classes[node.name] = {classes:node.classes, vars:node.vars, nativeVars:node.nativeVars, staticVars:node.staticVars, functions:node.functions, staticFunctions:node.staticFunctions, accessors:node.accessors};
                break;
            case 'function':
                if(!out.functions.hasOwnProperty(node.name))
                    out.functions[node.name] = [{args:node.args, code:node.code, line:node.line}];
                else
                    out.functions[node.name].push({args:node.args, code:node.code, line:node.line});
                break;
            case 'var':
                out.vars[node.name] = {val:node.val};
                break;
            case 'native-block':
                out.nativeBlocks[node.which] = out.nativeBlocks[node.which] ? (out.nativeBlocks[node.which] + '\n' + node.code) : node.code;
        }
    }
    
    //if(!out.functions.hasOwnProperty('toString'))
    //    out.functions['toString'] = [{args:[], code:'return String("['+(isModule?'module':'object')+' '+name+']");', line:line}];
    
    return out;
}

function nLines(s){return s.replace(/[^\n]/g,'').length;}
function nCols(s){return s.replace(/^(.*\n)+/,'').length;}

function sum(a,b) {
    return a+b;
}

var l = 1;
$aze.symbols.space = new codeaze.Symbol('/\\s/|/\\/\\*.*?\\*\\//', function($){var $$;l+=nLines($[0][0]);$$=$[0][0];return $$;});
$aze.symbols.l = new codeaze.Symbol(/(?:)/, function($){var $$;$$=l;return $$;});
$aze.symbols.identifier = new codeaze.Symbol(/~?[A-Za-z_]\w*/, function($){var $$;$$=$[0];return $$;});
$aze.symbols.identifierList = new codeaze.Symbol('(identifier( /,/ identifier)*)?', function($){var $$;$$=$[0]?[$[0][0]].concat($[0][1].map(function(x){return x[3]})):[];return $$;});
$aze.symbols.bulkCode = new codeaze.Symbol(/(\/\/.*|\/\*.*?\*\/|[^;{}()]+)/, function($){var $$;l+=nLines($[0]);$$=$[0];return $$;});
$aze.symbols.nativeCodeInline = new codeaze.Symbol('(bulkCode|/\\(/nativeCodeInline/\\)/)*', function($){var $$;$$=$[0].map(function(x){return x.length>1?x[0][0]+x[1]+x[2][0]:x[0]}).reduce(sum,'');return $$;});
$aze.symbols.nativeCode = new codeaze.Symbol('(bulkCode|/;/|/{/nativeCode/}/|/\\(/nativeCode/\\)/)*', function($){var $$;$$=$[0].map(function(x){return x.length>1?x[0][0]+x[1]+x[2][0]:(x[0][0]==';'?';':x[0])}).reduce(sum,'');return $$;});
$aze.symbols.module = new codeaze.Symbol('l/module/ +identifier /{/ moduleContents /}/', function($){var $$;$$=createObject($[3], $[7], $[0], true);return $$;});
$aze.symbols.moduleContents = new codeaze.Symbol('( +|object|nativeBlock)*', function($){var $$;$$=$[0].map(function(x){return x[0]}).filter(function(x){return !x.length});return $$;});
$aze.symbols.object = new codeaze.Symbol('l/object/ +identifier /{/ objectContents /}/', function($){var $$;$$=createObject($[3], $[7], $[0], false);return $$;});
$aze.symbols.objectContents = new codeaze.Symbol('( +|object|class|variableDef|function|getter|setter|nativeBlock)*', function($){var $$;$$=$[0].map(function(x){return x[0]}).filter(function(x){return !x.length});return $$;});
$aze.symbols.class = new codeaze.Symbol('l/class/ +identifier( /:/ identifierList)? /{/ classContents /}/', function($){var $$;$$=createClass($[3], $[8], $[0], $[4]&&$[4][3]);return $$;});
$aze.symbols.classContents = new codeaze.Symbol('( +|object|class|variableDef|nativeVar|function|getter|setter|nativeBlock)*', function($){var $$;$$=$[0].map(function(x){return x[0]}).filter(function(x){return !x.length});return $$;});
$aze.symbols.variableDef = new codeaze.Symbol('l(/static/ )?/var/ +identifier /=/ nativeCodeInline /;/', function($){var $$;$$={type:$[1]?'static-var':'var', name:$[4], val:$[8]};return $$;});
$aze.symbols.nativeVar = new codeaze.Symbol('l/native/ +identifier( /[*]*/ | +)identifier (/=/ nativeCodeInline )?/;/', function($){var $$;$$={type:'native-var', name:$[5], _type:$[3]+' '+($[4][1]||''), val:$[7] ? $[7][2] : null};return $$;});
$aze.symbols.function = new codeaze.Symbol('l(/static/ )?(/function/ +)?identifier /\\(/ identifierList /\\)/ /{/nativeCode/}/', function($){var $$;$$={type:$[1]?'static-function':'function', name:$[3], args:$[7], code:$[12], line:$[0]};return $$;});
$aze.symbols.getter = new codeaze.Symbol('l/get/ +identifier /\\(/ /\\)/ /{/nativeCode/}/', function($){var $$;$$={type:'getter', name:$[3], args:[], code:$[10], line:$[0]};return $$;});
$aze.symbols.setter = new codeaze.Symbol('l/set/ +identifier /\\(/ identifier /\\)/ /{/nativeCode/}/', function($){var $$;$$={type:'setter', name:$[3], args:[$[7]], code:$[12], line:$[0]};return $$;});
$aze.symbols.nativeBlock = new codeaze.Symbol('l,identifier /{/nativeCode/}/', function($){var $$;$$={type:'native-block', which:$[1], code:$[4], line:$[0]};return $$;});
$aze.symbols.main = new codeaze.Symbol('( +|module|nativeBlock)*', function($){var $$;$$=$[0].map(function(x){return x[0]}).filter(function(x){return !x.length});return $$;});


function makeTabs(n, ch) {
    var s = '';
    for(var i = 0; i < n; i++)
        s += ch;
    return s;
}

function makeLine(tbs, line) {
    return '\n' + tbs + '#line ' + line + ' "' + gear.src + '"';
}

var lineNumber = 1;
function generateFunctionCode(functions, name, parentPrefix, parentPath, code, _class, ctor, dest) {
    var prefix = parentPrefix + '_' + name, path = parentPath + '["' + name + '"]', replaces = [], funcCode = '', hasNoArgsVer = false;
    functions.sort(function(a, b) {return b.args.length - a.args.length;});
    for(f in functions) {
        var func = functions[f], replaces = [], tbs = (!dest && func.args.length ? '\t\t' : '\t');
        var actualCode = '\n' + tbs + func.code.trim() + '\n';
        
        var argsLine = '';
        if(dest=='setter')
            argsLine = func.args[0]+'(_'+func.args[0]+')';
        else
            for(var _arg in func.args)
                argsLine += (argsLine ? ', ' : '') + func.args[_arg] + '(args[' + _arg + '])';
        if(argsLine)
            actualCode = makeLine(tbs, func.line) + '\n' + tbs + 'Value ' + argsLine + ';' + actualCode;
        else
            actualCode = makeLine(tbs, func.line + 1) + actualCode;
        
        replaces.push({regex:'\n' + makeTabs(prefix.split('_').length-1, '    '), replace:'\n' + tbs});
        if(dest!='setter')
            replaces.push({regex:'\\breturn\\b\\s*;', replace:'return undefined;'});
        replaces.push({regex:'\\bthis\\b', replace:'This'});
        
        for(r in replaces) {
            var replace = replaces[r];
            actualCode = actualCode.replace(new RegExp(replace.regex, 'g'), replace.replace);
        }
        if(dest!='setter' && !RegExp('\n'+tbs+'\\breturn\\b[^;]*;\\s*$').exec(actualCode))
            actualCode += tbs + 'return undefined;\n';
        
        if(!dest && func.args.length)
            funcCode += '\n\tif(args.Length() >= ' + func.args.length + ') {' + actualCode + '\t}\n';
        else {
            funcCode += actualCode;
            hasNoArgsVer = true;
        }
    }
    
    if(_class) {
        if(_class.wrapName)
            funcCode = '\n\t'+_class.wrapName+'::This This(args.This()'+(ctor?', new '+_class.wrapName:'')+');'+funcCode;
        else
            funcCode = '\n\tValue This(args.This());'+funcCode;
    }
    
    if(!hasNoArgsVer)
        funcCode += '\tTHROW_ERROR("Invalid call to ' + parentPrefix.replace(/_/g, '.').replace(/^\./, '') + (ctor ? '' : (_class?'.prototype':'') + '.' + name) + '");\n';
    if(dest=='getter')
        code.func += 'static v8::Handle<v8::Value> ' + prefix + '(v8::Local<v8::String>, const v8::AccessorInfo &args) {' + funcCode + '}\n\n';
    else if(dest=='setter')
        code.func += 'static void ' + prefix + '(v8::Local<v8::String>, v8::Local<v8::Value> _'+func.args[0]+', const v8::AccessorInfo &args) {' + funcCode + '}\n\n';
    else
        code.func += 'static v8::Handle<v8::Value> ' + prefix + '(const v8::Arguments &args) {' + funcCode + '}\n\n';
}

function generateClassCode(_class, name, parentPrefix, parentPath, code) {
    var prefix = parentPrefix + '_' + name, path = parentPath + '["' + name + '"]';
    
    code.addClass(prefix, name);
    
    // Do we need an Wrap struct?
    if(('~'+name in _class.functions) || Object.keys(_class.nativeVars).length) {
        _class.wrapName = prefix+'_wrap';
        var wrap = 'struct '+_class.wrapName+' /*: public Value::DtorWrap*/ {\n';
        for(var i in _class.nativeVars)
            wrap += '\t'+_class.nativeVars[i].type+i+';\n';
        
        wrap += '\n\tstruct This : public Value {\n';
        
        wrap += '\t\tThis(v8::Handle<v8::Object> &&_this, '+_class.wrapName+' *wrap) : Value(_this), _wrap(wrap)';
        for(var i in _class.nativeVars)
            wrap += ', '+i+'(wrap->'+i+')';
        wrap += ' {\n\t\t\t_this->SetPointerInInternalField(0, wrap);\n\t\t}\n';
        
        wrap += '\t\tThis(v8::Handle<v8::Object> &&_this) : Value(_this), _wrap(static_cast<'+_class.wrapName+'*>(_this->GetPointerFromInternalField(0)))';
        for(var i in _class.nativeVars)
            wrap += ', '+i+'(_wrap->'+i+')';
        wrap += ' {}\n';
        
        wrap += '\t\t'+_class.wrapName+' *_wrap;\n';
        for(var i in _class.nativeVars)
            wrap += '\t\t'+_class.nativeVars[i].type+'&'+i+';\n';
        wrap += '\t};\n';
        
        wrap += '};\n\n';
        
        code.func += wrap;
        code.setInternalFieldCount(prefix, 1);
    }
    
    for(funcName in _class.functions) {
        if(funcName != name)
            code.setPrototype(prefix, funcName, code.makeFunction(prefix + '_' + funcName, funcName));
        generateFunctionCode(_class.functions[funcName], funcName, prefix, prefix, code, _class, funcName == name);
    }
    
    for(accName in _class.accessors) {
        if(!_class.accessors[accName].getter)
            throw new Error('No getter');
        generateFunctionCode([_class.accessors[accName].getter], accName, prefix, prefix, code, _class, false, 'getter');
        if(_class.accessors[accName].setter)
            generateFunctionCode([_class.accessors[accName].setter], accName, prefix, prefix, code, _class, false, 'setter');
        code.setPrototypeAccessor(prefix, accName, prefix + '_' + accName, !!_class.accessors[accName].setter);
    }
    
    for(varName in _class.vars) {
        var val = _class.vars[varName].val;
        code.setPrototype(prefix, varName, /^\s*\b[A-Z]\w+\b\(.+\)$/.test(val) ? val : 'Value(' + val + ')');
    }
    
    code.setStatic(parentPath, name, prefix + '->GetFunction()');
    
    for(className in _class.classes)
        generateClassCode(_class.classes[className], className, prefix, path, code);
    for(varName in _class.staticVars) {
        var val = _class.staticVars[varName].val;
        code.setStatic(path, varName, /^\s*\b[A-Z]\w+\b\(.+\)$/.test(val) ? val : 'Value(' + val + ')');
    }
    for(funcName in _class.staticFunctions) {
        code.setStatic(path, funcName, code.makeFunction(prefix + '_' + funcName, funcName));
        generateFunctionCode(_class.staticFunctions[funcName], funcName, prefix, path, code);
    }
}

function generateObjectCode(object, name, parentPrefix, parentPath, code) {
    var prefix = parentPrefix + '_' + name, path = parentPath ? parentPath + '["' + name + '"]' : name;
    
    if(path != 'exports' && !parentPath)
        code.addObject(path);
    
    for(className in object.classes)
        generateClassCode(object.classes[className], className, prefix, path, code);
    
    for(funcName in object.functions) {
        code.setStatic(path, funcName, code.makeFunction(prefix + '_' + funcName, funcName));
        generateFunctionCode(object.functions[funcName], funcName, prefix, path, code);
    }
    
    for(varName in object.vars) {
        var val = object.vars[varName].val;
        code.setStatic(path, varName, /^\s*\b[A-Z]\w+\b\(.+\)$/.test(val) ? val : 'Value(' + val + ')');
    }
}

function generateModuleCode(object, name, parentPrefix, parentPath, code) {
    var prefix = '_' + name, path = parentPath;
    
    for(objectName in object.objects)
        generateObjectCode(object.objects[objectName], objectName, prefix, path, code);
    
    if(object.nativeBlocks.postSetup)
        code.init += object.nativeBlocks.postSetup;
}

function generateCode(gear, global) {
    lineNumber = 1;
    var code = {
        addObject: function(path) {
            this.init += '\tvar ' + path + ' = Object();\n';
        },
        addClass: function(objName, ctor) {
            this.init += '\tv8::Handle<v8::FunctionTemplate> ' + objName + ' = v8::FunctionTemplate::New(' + objName + '_' + ctor + ');\n';
            this.init += '\t' + objName + '->SetClassName(String("' + ctor + '"));\n';
        },
        makeFunction: function(prefix, name) {
            return 'Function(' + prefix + ', "' + name + '")';
        },
        setStatic: function(parentObjName, name, value) {
            this.init += '\t' + parentObjName + '["' + name + '"] = ' + value + ';\n';
        },
        setPrototype: function(parentObjName, name, value) {
            this.init += '\t' + parentObjName + '->PrototypeTemplate()->Set("' + name + '", ' + value + ');\n';
        },
        setPrototypeAccessor: function(parentObjName, name, getter, setter) {
            this.init += '\t' + parentObjName + '->PrototypeTemplate()->SetAccessor(String("' + name + '"), ' + getter + (setter?', '+getter:'') + ');\n';
        },
        setInternalFieldCount: function(parentObjName, value) {
            this.init += '\t' + parentObjName + '->InstanceTemplate()->SetInternalFieldCount(' + value + ');\n';
        },
        addJS: function(name, js, args) {
            args = args.join(', ');
            js = '(function('+args+'){'+js+'})'; // TODO minify
            this.init += '\tContext::getCurrent()->runScript(' + JSON.stringify(js) + ', "' + name + '")('+args+');\n';
        },
    };
    
    var modules = Object.keys(global.modules);
    
    if(!modules.length)
        throw Error('No modules');
    
    function getBlock(o, n) {
        return o.nativeBlocks[n] ? (o.nativeBlocks[n].trim().replace(/\n    /g, '\n') + '\n\n') : '\n';
    }
    
    var license = getBlock(global, 'license'), top = getBlock(global, 'top'), header = getBlock(global, 'header');
    var ccCode = license+'#include <gearbox.h>\n'+(global.nativeBlocks.header?'#include "'+gear.h.replace(/^([^/]*\/)+/,'')+'"\n':'')+'\nusing namespace Gearbox;\n\n\
/** \\file '+gear.baseName+'.cc converted from '+gear.src+' */\n'+makeLine('',1) + '\n' + top;

    for(var i = 0; i < modules.length; i++) {
        code.func = code.init = '';
        
        var moduleName = modules[i], module = global.modules[moduleName];
        generateModuleCode(module, moduleName, '', '', code);
        
        ccCode += code.func;
        
        if(module.nativeBlocks.js) {
            /// function (exports, require, module, __filename, __dirname)
            var args = ['exports', 'require', 'module'];
            for(var j in module.objects)
                if(args.indexOf(j) === -1)
                    args.push(j);
            code.addJS('gear:'+moduleName, module.nativeBlocks.js, args);
        }
        
        ccCode += makeLine('',nLines(ccCode)+2).replace('.gear','.cc') + '\n\
static void _setup_' + moduleName + '(Value exports, Value require, Value module) {\n' + code.init + '}\n\
static NativeModule _module_' + moduleName + '("'+moduleName+'", _setup_' + moduleName + ');';
    }
    ccCode = ccCode.replace(/\t/g, '    ');
    fs.writeFileSync(gear.cc, ccCode);
    
    if(global.nativeBlocks.header) {
        var hCode = license+'\
#ifndef GEARBOX_MODULES_'+baseName.replace(/^([^/]*\/)+/,'').toUpperCase()+'_H\n\
#define GEARBOX_MODULES_'+baseName.replace(/^([^/]*\/)+/,'').toUpperCase()+'_H\n\n\
#include <gearbox.h>\n\n'+header+'#endif\n';
        fs.writeFileSync(gear.h, hCode);
    }
}

$aze.spaceIgnore = false;
var fs = require('fs');
if(!global.arguments)
    arguments = process.argv.slice(1);
if(arguments.length > 1) {
    for(var i = 1; i < arguments.length; i++) {
        l = 1;
        var lastDot = arguments[i].lastIndexOf('.'), lastSlash = arguments[i].lastIndexOf('/')+1, baseName = arguments[i].substr(0, lastDot), ext = arguments[i].substr(lastDot+1);
        var gear = {src:arguments[i], baseName:baseName, cc:baseName+'.cc', h:baseName+'.h'};
        var src = fs.readFileSync(arguments[i], 'utf8');
        if(ext == 'gear')
            generateCode(gear, createObject('', $aze.parse(src), 1));
        else if(ext == 'js') {
            var licenseRegExp = /[ \t]*\/\/.*copyright.*(\n[ \t]*\/\/.*)*/mi, license = null;
            if(license = licenseRegExp.exec(src)) {
                index = license.index;
                license = license[0];
                src = src.substr(0, index) + src.substr(index + license.length);
            }
            var o = {type:'object', name:'', objects:{}, modules:{}, classes:{}, vars:{}, functions:{}, nativeBlocks:{license:license}};
            o.modules[arguments[i].slice(lastSlash, lastDot)] = {objects:{}, modules:{}, classes:{}, vars:{}, functions:{}, nativeBlocks:{js:src}};
            generateCode(gear, o);
        }
    }
} else
    print('Usage: ' + arguments[0] + ' <files>');
(global.exit || process.exit)(); // Just in case gearbox-node is a (bit) broken
