{
  'variables': {
    'v8_use_snapshot': 'true',
    # Turn off -Werror in V8
    # See http://codereview.chromium.org/8159015
    'werror': '',
    'target_arch': 'ia32',
    'node_use_dtrace': 'false',
    'node_use_openssl%': 'true',
    'node_use_system_openssl%': 'false',
    'gear_files': [
      'src/modules/assert.js',
      'src/modules/buffer.gear',
      'src/modules/buffer_ieee754.js',
      'src/modules/console.js',
      'src/modules/constants.gear',
      'src/modules/events.js',
      'src/modules/fs.gear',
      'src/modules/path.js',
      'src/modules/punycode.js',
      'src/modules/stream.js',
      'src/modules/sys.js',
      'src/modules/url.js',
      'src/modules/util.js',
      
      #FIXME
      'src/modules/Io.gear',
    ],
    'gear_output': [
      'src/modules/assert.cc',
      'src/modules/buffer.cc',
      'src/modules/buffer_ieee754.cc',
      'src/modules/console.cc',
      'src/modules/constants.cc',
      'src/modules/events.cc',
      'src/modules/fs.cc',
      'src/modules/path.cc',
      'src/modules/punycode.cc',
      'src/modules/stream.cc',
      'src/modules/sys.cc',
      'src/modules/url.cc',
      'src/modules/util.cc',
      
      #FIXME
      'src/modules/Io.cc',
    ],
  },

  'targets': [
    {
      'target_name': 'node',
      'type': 'executable',

      'dependencies': [
        'deps/http_parser/http_parser.gyp:http_parser',
        'deps/uv/uv.gyp:uv',
        'deps/zlib/zlib.gyp:zlib',
        'gears#host',
      ],

      'include_dirs': [
        'src',
        'deps/uv/src/ares'
      ],

      'sources': [
        'src/main.cc',
        'src/Context.cc',
        'src/NativeModule.cc',
        'src/String.cc',
        'src/TryCatch.cc',
        'src/Value.cc',
        
        '<@(gear_output)',
        
        # headers to make for a more pleasant IDE experience
        'src/Context.h',
        'src/gearbox.h',
        'src/NativeModule.h',
        'src/String.h',
        'src/TryCatch.h',
        'src/Value.h',
        
        'deps/http_parser/http_parser.h',
        #'deps/v8/include/v8.h',
        #'deps/v8/include/v8-debug.h',
        
        '<@(gear_files)',
      ],

      'defines': [
        'ARCH="<(target_arch)"',
        'PLATFORM="<(OS)"',
        '_LARGEFILE_SOURCE',
        '_FILE_OFFSET_BITS=64',
      ],

      'conditions': [
        [ 'node_use_openssl=="true"', {
          'defines': [ 'HAVE_OPENSSL=1' ],
          #'sources': [ 'src/node_crypto.cc' ],
          'conditions': [
            [ 'node_use_system_openssl=="false"', {
              'dependencies': [ './deps/openssl/openssl.gyp:openssl' ],
            }]]
        }, {
          'defines': [ 'HAVE_OPENSSL=0' ]
        }],

        [ 'node_use_dtrace=="true"', {
          'sources': [
            'src/node_dtrace.cc',
            'src/node_dtrace.h',
            # why does node_provider.h get generated into src and not
            # SHARED_INTERMEDIATE_DIR?
            'src/node_provider.h',
          ],
        }],

        [ 'node_shared_v8=="false"', {
          'dependencies': [ 'deps/v8/tools/gyp/v8.gyp:v8' ],
        }, {
          'libraries': [ '-lv8' ],
        }],

        [ 'OS=="win"', {
          'sources': [
            #'src/platform_win32.cc',
            ## headers to make for a more pleasant IDE experience
            #'src/platform_win32.h',
          ],
          'defines': [
            'FD_SETSIZE=1024',
            # we need to use node's preferred "win32" rather than gyp's preferred "win"
            'PLATFORM="win32"',
          ],
          'libraries': [ '-lpsapi.lib' ]
        },{ # POSIX
          'defines': [ '__POSIX__' ],
          'sources': [
            #'src/node_signal_watcher.cc',
            #'src/node_stat_watcher.cc',
            #'src/node_io_watcher.cc',
          ]
        }],
        [ 'OS=="mac"', {
          #'sources': [ 'src/platform_darwin.cc' ],
          'libraries': [ '-framework Carbon' ],
        }],
        [ 'OS=="linux"', {
          #'sources': [ 'src/platform_linux.cc' ],
          'libraries': [
            '-ldl',
            '-lutil' # needed for openpty
          ],
        }],
        [ 'OS=="freebsd"', {
          #'sources': [ 'src/platform_freebsd.cc' ],
          'libraries': [
            '-lutil',
            '-lkvm',
          ],
        }],
        [ 'OS=="solaris"', {
          #'sources': [ 'src/platform_sunos.cc' ],
          'libraries': [
            '-lkstat',
          ],
        }],
      ],
      'msvs-settings': {
        'VCLinkerTool': {
          'SubSystem': 1, # /subsystem:console
        },
      },
      'cflags_cc': [ '-std=c++0x', '-Wno-unused' ],
    },

    {
      'target_name': 'gear2cc',
      'type': 'none',
      'toolsets': ['host'],
      'variables': {
      },

      'actions': [
        {
          'action_name': 'gear2cc',

          'inputs': [
            'tools/gear2cc/aze2js.js',
            'tools/gear2cc/gear2cc.aze',
          ],

          'outputs': [
            'tools/gear2cc/gear2cc.js',
          ],
          
          'action': ['./node', 'tools/gear2cc/aze2js.js', 'tools/gear2cc/gear2cc.aze', 'tools/gear2cc/gear2cc.js'],
        },
      ],
    }, # end gear2cc

    {
      'target_name': 'gears',
      'type': 'none',
      'toolsets': ['host'],
      'variables': {
      },
      
      'dependencies': [
        'gear2cc#host',
      ],

      'actions': [
        {
          'action_name': 'gears',

          'inputs': [
            'tools/gear2cc/gear2cc.js',
            '<@(gear_files)',
          ],

          'outputs': [
            '<@(gear_output)',
          ],
          
          'action': ['./node', 'tools/gear2cc/gear2cc.js', '<@(gear_files)'],
        },
      ],
      
    }, # end gears
  ] # end targets
}
