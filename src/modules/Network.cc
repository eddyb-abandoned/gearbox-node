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
#include "Network.h"

using namespace Gearbox;

/** \file Network.cc converted from Network.gear */

#line 1 "src/modules/Network.gear"
#ifdef _WIN32
#include <winsock.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

static v8::Handle<v8::Value> _Network_Socket_Socket(const v8::Arguments &args) {
    Value This(args.This());
    if(args.Length() >= 2) {
        #line 43 "src/modules/Network.gear"
        Value family(args[0]), type(args[1]);
        int sock = socket(family, type, 0);
        if(sock == -1)
            THROW_ERROR("Unable to create socket");
        
        This["socket"] = Internal(sock);
        This["family"] = Internal(family);
        This["type"] = Internal(type);
        return undefined;
    }
    THROW_ERROR("Invalid call to Network.Socket");
}

static v8::Handle<v8::Value> _Network_Socket_connect(const v8::Arguments &args) {
    Value This(args.This());
    if(args.Length() >= 2) {
        #line 53 "src/modules/Network.gear"
        Value host(args[0]), port(args[1]);
        struct hostent *host_s = gethostbyname(host.to<String>());
        if(!host_s)
            THROW_ERROR("Unable to resolve host");
            
        struct sockaddr_in server_addr;
        server_addr.sin_family = This["family"].to<uint32_t>();
        server_addr.sin_port = htons(port.to<uint32_t>());
        server_addr.sin_addr = *((struct in_addr *)host_s->h_addr);
        
        int result = connect(This["socket"], (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
        if(result == -1)
            THROW_ERROR("Unable to connect");
        
        This["isConnected"] = Internal(true);
        return undefined;
    }
    THROW_ERROR("Invalid call to Network.Socket.prototype.connect");
}

static v8::Handle<v8::Value> _Network_Socket_receive(const v8::Arguments &args) {
    Value This(args.This());
    #line 71 "src/modules/Network.gear"
    int maxLen = Value(args[0]) == undefined ? 1024 : Value(args[0]).to<int>();
    char *buffer = new char [maxLen];
    int len = recv(This["socket"], buffer, maxLen, 0);
    if(len > 0) {
        String str(buffer, len);
        delete [] buffer;
        return str;
    }
    delete [] buffer;
    return undefined;
}

static v8::Handle<v8::Value> _Network_Socket_send(const v8::Arguments &args) {
    Value This(args.This());
    if(args.Length() >= 1) {
        #line 82 "src/modules/Network.gear"
        Value data(args[0]);
        send(This["socket"], data.to<String>(), data.length(), 0);
        return undefined;
    }
    THROW_ERROR("Invalid call to Network.Socket.prototype.send");
}

static v8::Handle<v8::Value> _Network_Socket_close(const v8::Arguments &args) {
    Value This(args.This());
    #line 87 "src/modules/Network.gear"
    #ifdef _WIN32
    closesocket(This["socket"]);
#else
    close(This["socket"]);
#endif
    return undefined;
}

static v8::Handle<v8::Value> _Network_Socket_block(const v8::Arguments &args) {
    Value This(args.This());
    if(args.Length() >= 1) {
        #line 94 "src/modules/Network.gear"
        Value blocking(args[0]);
        #ifdef _WIN32
        u_long mode = blocking ? 1 : 0;
        ioctlsocket(This["socket"], FIONBIO, &mode);
#else
        int mode = fcntl(This["socket"], F_GETFL, 0);
        if(blocking)
            mode &= ~O_NONBLOCK;
        else
            mode |= O_NONBLOCK;
        fcntl(This["socket"], F_SETFL, mode);
#endif
        return undefined;
    }
    THROW_ERROR("Invalid call to Network.Socket.prototype.block");
}

static v8::Handle<v8::Value> _Network_toString(const v8::Arguments &args) {
    #line 37 "src/modules/Network.gear"
    return String("[module Network]");
}


#line 145 "src/modules/Network.cc"
static void _setup_Network(Value _exports) {
    v8::Handle<v8::FunctionTemplate> _Network_Socket = v8::FunctionTemplate::New(_Network_Socket_Socket);
    _Network_Socket->SetClassName(String("Socket"));
    _Network_Socket->PrototypeTemplate()->Set("connect", Function(_Network_Socket_connect, "connect"));
    _Network_Socket->PrototypeTemplate()->Set("receive", Function(_Network_Socket_receive, "receive"));
    _Network_Socket->PrototypeTemplate()->Set("send", Function(_Network_Socket_send, "send"));
    _Network_Socket->PrototypeTemplate()->Set("close", Function(_Network_Socket_close, "close"));
    _Network_Socket->PrototypeTemplate()->Set("block", Function(_Network_Socket_block, "block"));
    _Network_Socket->PrototypeTemplate()->Set("socket", Value(-1));
    _Network_Socket->PrototypeTemplate()->Set("family", Value(-1));
    _Network_Socket->PrototypeTemplate()->Set("type", Value(-1));
    _Network_Socket->PrototypeTemplate()->Set("isConnected", Value(false));
    _exports["Socket"] = _Network_Socket->GetFunction();
    _exports["Socket"]["INet"] = Value(AF_INET);
    _exports["Socket"]["Unix"] = Value(AF_UNIX);
    _exports["Socket"]["Tcp"] = Value(SOCK_STREAM);
    _exports["Socket"]["Udp"] = Value(SOCK_DGRAM);
    _exports["toString"] = Function(_Network_toString, "toString");
}
static Module _module_Network("Network", _setup_Network);