import os
import re
import sys

env = Environment()

crossmingw = ARGUMENTS.get('crossmingw', 0)
env['USING_CROSSMINGW'] = False
if int(crossmingw):
    env.Tool('crossmingw', toolpath = ['#tools'])
    env['USING_CROSSMINGW'] = True

# Windows gets special treatement
if sys.platform == 'win32' or env['USING_CROSSMINGW']:
    env.Append(CPPPATH = Glob(os.path.join('#contrib', '*', 'include')))
    env.Append(CPPPATH = Glob(os.path.join('#contrib', '*')))
    env.Append(CPPPATH = '#src')
    env.Append(LIBPATH = Glob(os.path.join('#contrib', '*', 'bin')))
    env.Append(LIBPATH = Glob(os.path.join('#contrib', '*', 'lib')))
    env.Append(LIBPATH = Glob(os.path.join('#contrib', '*')))
    env.Append(LINKFLAGS = '-static-libgcc -static-libstdc++')
    env.Append(CXXFLAGS = '-std=c++0x -O3 -fno-var-tracking-assignments')
    env.Append(LIBS = ['v8', 'readline', 'pthread', 'ws2_32' , 'winmm'])
else:
    env.Append(CPPPATH = '#src')
    env.Append(LINKFLAGS = '-Wl,--no-warn-search-mismatch')
    env.Append(CXXFLAGS = '-std=c++0x -O3 -fno-var-tracking-assignments')
    env.Append(LIBS = ['v8', 'readline'])

# Pretty output
if sys.platform == 'win32' or not hasattr(os.environ, 'TERM') or os.environ['TERM'] == 'dumb':
    env['CCCOMSTR']   =    '     Compiling $SOURCES -> $TARGET'
    env['CXXCOMSTR']  =    '     Compiling $SOURCES -> $TARGET'
    env['ASCOMSTR']   =    '    Assembling $SOURCES -> $TARGET'
    env['LINKCOMSTR'] =    '       Linking $SOURCES -> $TARGET'
    env['ARCOMSTR']   =    '     Archiving $SOURCES -> $TARGET'
    env['RANLIBCOMSTR'] =  '      Indexing $SOURCES -> $TARGET'
    env['NMCOMSTR']   =    '  Creating map $SOURCES -> $TARGET'
    env['DOCCOMSTR']  =    '   Documenting $SOURCES -> $TARGET'
    env['TARCOMSTR']  =    '      Creating $SOURCES -> $TARGET'
    env['AZE2JSCOMSTR']  = '    Converting $SOURCES -> $TARGET'
    env['GEAR2CCCOMSTR']  ='    Converting $SOURCES -> $TARGETS'
else:
    env['CCCOMSTR']   =    '     Compiling \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['CXXCOMSTR']  =    '     Compiling \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['ASCOMSTR']   =    '    Assembling \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['LINKCOMSTR'] =    '       Linking \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['ARCOMSTR']   =    '     Archiving \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['RANLIBCOMSTR'] =  '      Indexing \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['NMCOMSTR']   =    '  Creating map \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['DOCCOMSTR']  =    '   Documenting \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['TARCOMSTR']  =    '      Creating \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['AZE2JSCOMSTR']  = '    Converting \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGET\033[0m'
    env['GEAR2CCCOMSTR']  ='    Converting \033[32m$SOURCES\033[0m\033[1m -> \033[0m\033[32m$TARGETS\033[0m'
    
env['BUILD_DIR'] = 'build'
env['GEARBOX'] = env['BUILD_DIR'] + os.sep + 'gearbox' + env['PROGSUFFIX']
env['GEARBOX_EXISTS'] = os.path.exists(env['GEARBOX'])
env['BUILDERS']['Aze2JS'] = Builder(action=Action(env['GEARBOX']+' gear2cc'+os.sep+'aze2js.js $SOURCE $TARGET', cmdstr=env['AZE2JSCOMSTR']))
env['BUILDERS']['Gear2CC'] = Builder(action=Action(env['GEARBOX']+' gear2cc'+os.sep+'gear2cc.js $SOURCE', cmdstr=env['GEAR2CCCOMSTR']))

SConscript('src/SConscript', variant_dir = env['BUILD_DIR'], exports = ['env'], duplicate = 0)
