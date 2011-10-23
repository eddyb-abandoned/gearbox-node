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

#include <gearbox.h>
#include <modules/Io.h>

using namespace Gearbox;
using namespace Modules;

std::function<void()> Gearbox::g_pMainLoop;

#include <cstdlib>

#include <readline/readline.h>
#include <readline/history.h>

#define GEARBOX_HISTORY_FILE "/.gearbox_history"

void shellLoop(Context &context) {
    TryCatch tryCatch;
    
    // Read the history records prior to starting this shell
    String historyFile = String::concat(std::getenv("HOME"), GEARBOX_HISTORY_FILE);
    read_history(historyFile);
    
    printf("v8-gearbox [v8 version %s]" _STR_NEWLINE, v8::V8::GetVersion());
    
    while(true) {
        // Read a line from the user
        String line = readline("gearbox> ");
        
        // Ignore empty lines
        if(line.empty() || !line.length())
            continue;
        
        // Add the line to the history only if it's different to the line before it
        HIST_ENTRY *lastEntry = history_get(history_length);
        if(!lastEntry || strcmp(line, lastEntry->line)) {
            add_history(line);
            append_history(1, historyFile);
        }
        
        // Execute the expression
        var result = context.runScript(line, "(shell)");
        
        // Check for exceptions
        if(tryCatch.hasCaught())
            tryCatch.reportException();
        else
            printf("%s" _STR_NEWLINE, *result.to<String>());
    }
}


int main(int argc, char* argv[]) {
    v8::HandleScope handleScope;
    
    // Pass the flags first to v8
    // TODO Pass to v8 only the flags that we do not recognize
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    
    // Create a new context
    Context context;
    TryCatch tryCatch;
    
    // Set the arguments array
    var arguments = Array();
    context.global()["arguments"] = arguments;
    
    bool runShell = (argc == 1);
    for(int i = 1; i < argc; i++) {
        String arg = argv[i];
        
        // -s --shell: force running the shell
        if(arg == "-s" || arg == "--shell")
            runShell = true;
        // -e --eval <code>: evaluate the code
        else if((arg == "-e" || arg ==  "--eval") && i <= argc) {
            // Run the code
            context.runScript(argv[++i], "unnamed");
            
            // Stop if there are exceptions
            if(tryCatch.hasCaught())
                return 1;
        }
        // Warn about unknown flags
        else if(arg.compare("--", 2))
            printf("Warning: unknown flag %s." _STR_NEWLINE "Try --help for options" _STR_NEWLINE, *arg);
        // Treat the first argument that is not an option as a file to execute
        else {
            // Read the file
            String source = Io::read(*arg);
            
            // Report exceptions caught while reading the file
            if(tryCatch.hasCaught())
                return 1;
            
            // Set the rest of the arguments into the arguments array
            for(int j = i; j < argc; j++)
                arguments[j - i] = argv[j];
            
            // Run the script
            context.runScript(source, arg);
            
            // Specific mainLoop handlers.
            if(g_pMainLoop)
                g_pMainLoop();
            
            // Stop if there are exceptions
            if(tryCatch.hasCaught())
                return 1;
            else
                break;
        }
    }
    
    // Run the shell (if no arguments or -s/--shell was given)
    if(runShell)
        shellLoop(context);
    return 0;
}
