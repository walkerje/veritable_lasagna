\page man_advanced Advanced Features

Veritable Lasagna goes beyond simple data structures, providing advanced features for modern application development such as SIMD acceleration, unified I/O, and more.

## Table of Contents
- [SIMD Support (vl_simd)](#simd-support-vl_simd)
- [Streams (vl_stream)](#streams-vl_stream)
- [Filesystem Utilities (vl_filesys)](#filesystem-utilities-vl_filesys)
- [Logging (vl_log)](#logging-vl_log)
- [Serialization with MessagePack (vl_msgpack)](#serialization-with-messagepack-vl_msgpack)
- [Networking (vl_socket)](#networking-vl_socket)

## SIMD Support ( vl_simd )

### Description
The `vl_simd` module provides a portable, cross-platform interface for hardware-accelerated vector instructions. It allows you to write vector code once and have it run efficiently on different architectures (SSE, AVX, etc.).

### Key Features
- **SIMD Vectors:** Support for float, int32, and int16 vectors (e.g., `vl_simd_vec4_f32`, `vl_simd_vec8_f32`).
- **Standard Operations:** Load/Store, Add, Sub, Mul, Div, FMA (Fused Multiply-Add).
- **Horizontal Operations:** Horizontal sum, max, and min.

### Use Cases
- **Graphics & Audio:** Processing large arrays of vertices or samples.
- **Scientific Computing:** Heavy mathematical operations on large datasets.
- **Performance Optimization:** Accelerating tight loops that operate on independent data elements.

### Initialization
Before using SIMD functions, it is recommended to call `vlSIMDInit()` to detect the best available hardware features.

## Streams ( vl_stream )

### Description
The `vl_stream` API provides a generic, thread-safe byte stream abstraction. It decouples the I/O logic from the underlying storage, allowing the same code to work with files, memory, or custom backends.

### Key Features
- **Generic Interface:** Standard read, write, seek, tell, and flush operations.
- **Reference Counted:** Streams manage their own lifetime via `vlStreamRetain` and `vlStreamDelete`.
- **Thread Safe:** Internal mutex serialization for all I/O operations.

### Use Cases
- **Unified I/O:** Writing data to a file or a memory buffer using the same function calls.
- **Networking:** Abstracting socket communication as a stream.
- **Data Compression/Encryption:** Wrapping a stream to transform data on the fly.

### Implementations
- **Memory Stream (`vl_stream_memory`):** Use a `vl_buffer` or raw memory as a stream.
- **File Stream (`vl_stream_filesys`):** Use a filesystem file as a stream.

### Basic Usage
```c
#include <vl/vl_stream_filesys.h>

void stream_example(vl_filesys* fs) {
    vl_stream* stream = vlStreamFilesysOpen(fs, "data.bin", VL_FS_OPEN_WRITE);
    if (stream) {
        const char* msg = "Hello Stream";
        vlStreamWrite(stream, msg, strlen(msg));
        vlStreamDelete(stream);
    }
}
```

## Filesystem Utilities ( vl_filesys )

### Description
The `vl_filesys` module provides a cross-platform way to handle paths, files, and directory operations. It simplifies common filesystem tasks that differ between Unix-like systems and Windows.

### Key Features
- **Path Manipulation:** Join, normalize, and get parent directories safely.
- **Directory Traversal:** Both simple and recursive directory iteration.
- **File Info:** Stat files to get size, modification time, and type.

### Use Cases
- **File Management:** Creating, deleting, and renaming files and directories.
- **Asset Loading:** Iterating through a directory to load all available resources.
- **Path Cleanup:** Normalizing user-provided paths to prevent errors.

### Basic Usage
```c
#include <vl/vl_filesys.h>

void fs_example() {
    vl_filesys fs;
    vlFSInit(&fs);

    vl_filesys_path* path = vlFSPathNew(&fs, "./test_dir");
    if (!vlFSPathExists(path)) {
        vlFSPathMkDir(path);
    }

    // Iterate through directory
    vl_filesys_iter iter = vlFSIterNew(&fs);
    if (vlFSIterDir(&iter, path) == VL_FILESYS_SUCCESS) {
        while (vlFSIterNext(&iter)) {
            // Process each file in the directory
        }
    }

    vlFSIterDelete(iter);
    vlFSPathDelete(path);
    vlFSFree(&fs);
}
```

## Logging ( vl_log )

### Description
The logging module provides a configurable, thread-safe logging system. It supports multiple output targets, log rotation, and formatted output.

### Key Features
- **Console & File Output:** Log messages to stdout and/or rotated log files.
- **Log Rotation:** Automatically rotates logs when they exceed a specified size.
- **Formatting:** Supports printf-style formatted messages.
- **Debug Macros:** Lightweight macros for debug-only logging.

### Use Cases
- **Application Monitoring:** Tracking the state and activity of a running program.
- **Error Reporting:** Recording critical failures with enough context for debugging.
- **Audit Trails:** Keeping a history of significant events in a system.

### Basic Usage
```c
#include <vl/vl_log.h>

void log_example() {
    vlLogInit(NULL); // Use default configuration

    VL_LOG_MSG0("Initializing application...");
    VL_LOG_MSGF("Application started on port %d", 8080);

    vlLogShutdown();
}
```

## Serialization with MessagePack ( vl_msgpack )

### Description
MessagePack is a compact, binary serialization format. Veritable Lasagna provides both a DOM-style API for building and navigating data structures in memory, and a high-performance streaming API.

### Use Cases
- **Network Protocol:** Encoding data for transmission between services.
- **State Persistence:** Saving application state or configuration to disk.
- **Data Exchange:** Interfacing with other programs or languages that support MessagePack.

### DOM API
Build and parse MessagePack objects in memory.

```c
#include <vl/vl_msgpack.h>

void msgpack_dom_example() {
    vl_msgpack pack;
    vlMsgPackInit(&pack);

    vl_msgpack_iter root = vlMsgPackRoot(&pack);
    vlMsgPackSetMapIndexed(&pack, root, 0); 

    vlMsgPackSetStringNamed(&pack, root, "Hello", "msg");
    vlMsgPackFree(&pack);
}
```

### Streaming API ( vl_msgpack_io )
For high-performance encoding and decoding without building a full DOM.

```c
#include <vl/vl_msgpack_io.h>

void msgpack_io_example() {
    vl_msgpack_encoder enc;
    vlMsgPackIOEncoderInit(&enc);

    vlMsgPackIOEncodeMapBegin(&enc);
    vlMsgPackIOEncodeString(&enc, "key");
    vlMsgPackIOEncodeInt(&enc, 123);
    vlMsgPackIOEncodeMapEnd(&enc);

    vlMsgPackIOEncoderFree(&enc);
}
```

## Networking ( vl_socket )

### Description
The `vl_socket` module provides a portable, cross-platform socket API for networking over IPv4 and IPv6. It supports both TCP (Stream) and UDP (Datagram) communication with an easy-to-use interface that abstracts away the differences between Win32 and POSIX socket implementations.

### Key Features
- **Protocols:** Full support for TCP and UDP over IPv4 and IPv6.
- **Cross-Platform:** Single API that works on both Windows (WinSock) and Linux/macOS.
- **I/O Modes:** Easy switching between blocking and non-blocking I/O.
- **Options:** Support for common socket options like `SO_REUSEADDR`, `TCP_NODELAY`, and `SO_KEEPALIVE`, including getters to check their current status.
- **Integrated Addresses:** Simple API for setting up IPv4 and IPv6 addresses, with support for converting to and from human-readable strings.
- **Local & Remote Address Retrieval:** Retrieve the address a socket is bound to via `vlSocketGetLocalAddress` or connected to via `vlSocketGetRemoteAddress`.

### Use Cases
- **Client/Server Applications:** Building high-performance network services or clients.
- **Inter-Process Communication:** Using loopback sockets for communication between local processes.
- **Networked Tools:** Implementing protocols like HTTP, FTP, or custom binary protocols.

### Basic Usage (TCP Client)
```c
#include <vl/vl_socket.h>

void client_example() {
    // Initialize the library
    vlSocketStartup();

    vl_socket_address addr;
    vlSocketAddressSetIPv4(&addr, 127, 0, 0, 1, 8080);

    vl_socket client = vlSocketNew(VL_SOCKET_DOMAIN_IPV4, VL_SOCKET_TYPE_STREAM);
    if (client != VL_SOCKET_NULL) {
        if (vlSocketConnect(client, &addr) == VL_SOCKET_SUCCESS) {
            const char* msg = "Hello from VL";
            vlSocketSend(client, msg, strlen(msg));
        }
        vlSocketDelete(client);
    }

    // Cleanup
    vlSocketShutdownLibrary();
}
```

### Basic Usage (TCP Server)
```c
#include <vl/vl_socket.h>

void server_example() {
    vlSocketStartup();

    vl_socket_address addr;
    vlSocketAddressSetIPv4(&addr, 0, 0, 0, 0, 8080); // Any address

    vl_socket listener = vlSocketNew(VL_SOCKET_DOMAIN_IPV4, VL_SOCKET_TYPE_STREAM);
    vlSocketSetReuseAddress(listener, VL_TRUE);
    
    if (vlSocketBind(listener, &addr) == VL_SOCKET_SUCCESS) {
        // Find which port we were bound to (useful if we used port 0)
        vl_socket_address localAddr;
        vlSocketGetLocalAddress(listener, &localAddr);
        
        vlSocketListen(listener, 10);
        
        vl_socket_address clientAddr;
        vl_socket client = vlSocketAccept(listener, &clientAddr);
        if (client != VL_SOCKET_NULL) {
            char buffer[256];
            vl_int64_t len = vlSocketReceive(client, buffer, sizeof(buffer));
            // Process buffer...
            vlSocketDelete(client);
        }
    }

    vlSocketDelete(listener);
    vlSocketShutdownLibrary();
}
```
