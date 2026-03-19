/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

#ifndef VL_SOCKET_H
#define VL_SOCKET_H

#include <vl/vl_memory.h>
#include <vl/vl_numtypes.h>

#ifndef VL_SOCKET_NULL
#define VL_SOCKET_NULL ((vl_socket)0)
#endif

/**
 * \brief Opaque socket handle.
 *
 * The underlying representation is platform-specific:
 * - POSIX backends typically wrap a file descriptor
 * - Win32 backends typically wrap a Winsock SOCKET
 */
typedef struct vl_socket_* vl_socket;

/**
 * \brief Supported socket address families.
 */
typedef enum vl_socket_domain_
{
    VL_SOCKET_DOMAIN_IPV4 = 0,
    VL_SOCKET_DOMAIN_IPV6 = 1
} vl_socket_domain;

/**
 * \brief Supported socket kinds.
 */
typedef enum vl_socket_type_
{
    VL_SOCKET_TYPE_STREAM = 0,
    VL_SOCKET_TYPE_DATAGRAM = 1
} vl_socket_type;

/**
 * \brief Shutdown direction for a socket.
 */
typedef enum vl_socket_shutdown_
{
    VL_SOCKET_SHUTDOWN_RECEIVE = 0,
    VL_SOCKET_SHUTDOWN_SEND = 1,
    VL_SOCKET_SHUTDOWN_BOTH = 2
} vl_socket_shutdown;

/**
 * \brief Result codes for socket operations.
 *
 * These values are intended to describe high-level API outcomes in a
 * platform-neutral way.
 */
typedef enum vl_socket_result_
{
    VL_SOCKET_SUCCESS = 0,

    VL_SOCKET_ERROR_INVALID_ARGUMENT,
    VL_SOCKET_ERROR_NOT_INITIALIZED,
    VL_SOCKET_ERROR_ALLOCATION,

    VL_SOCKET_ERROR_CREATE,
    VL_SOCKET_ERROR_BIND,
    VL_SOCKET_ERROR_LISTEN,
    VL_SOCKET_ERROR_ACCEPT,
    VL_SOCKET_ERROR_CONNECT,
    VL_SOCKET_ERROR_SEND,
    VL_SOCKET_ERROR_RECEIVE,
    VL_SOCKET_ERROR_SHUTDOWN,
    VL_SOCKET_ERROR_CLOSE,
    VL_SOCKET_ERROR_SET_OPTION,

    VL_SOCKET_ERROR_WOULD_BLOCK,
    VL_SOCKET_ERROR_NOT_CONNECTED,
    VL_SOCKET_ERROR_ADDRESS_FAMILY,
    VL_SOCKET_ERROR_SYSTEM
} vl_socket_result;

/**
 * \brief Portable socket address structure.
 *
 * This structure stores either an IPv4 or IPv6 address along with a port.
 * The address bytes are stored in network byte order form.
 *
 * For IPv4:
 * - `host.ipv4[0..3]` correspond to the four octets of the address.
 *
 * For IPv6:
 * - `host.ipv6[0..15]` contain the 128-bit address.
 *
 * The `port` field is expressed in host byte order.
 */
typedef struct vl_socket_address_
{
    vl_socket_domain domain;
    vl_uint16_t port;

    union
    {
        vl_uint8_t ipv4[4];
        vl_uint8_t ipv6[16];
    } host;
} vl_socket_address;

/**
 * \brief Initializes the process-wide socket subsystem.
 *
 * On platforms where no explicit socket startup is required, this function is a
 * no-op that returns `VL_SOCKET_SUCCESS`.
 *
 * On platforms that require initialization of the networking subsystem, this
 * function must be called successfully before creating sockets.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Initializes process-global socket state as needed.
 * - **Thread Safety**: Safe to call, but should generally be performed during application startup.
 * - **Nullability**: N/A.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_SYSTEM` or `VL_SOCKET_ERROR_NOT_INITIALIZED` if platform startup
 * fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: May allocate platform-managed resources.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \return operation result code
 */
VL_API vl_socket_result vlSocketStartup(void);

/**
 * \brief Shuts down the process-wide socket subsystem.
 *
 * On platforms where no explicit teardown is required, this function is a
 * no-op.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Releases process-global socket resources as needed.
 * - **Thread Safety**: Should not race with active socket creation or active socket use.
 * - **Nullability**: N/A.
 * - **Error Conditions**: None reported through the API.
 * - **Undefined Behavior**: Calling while sockets are still in active use may cause backend-specific issues.
 * - **Memory Allocation Expectations**: May release platform-managed resources.
 * - **Return-value Semantics**: None (void).
 */
VL_API void vlSocketShutdownLibrary(void);

/**
 * \brief Creates a new socket.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_socket` handle and must release it with `vlSocketDelete`.
 * - **Lifetime**: The returned handle remains valid until deleted.
 * - **Thread Safety**: This function is thread-safe assuming the socket subsystem has been properly initialized.
 * - **Nullability**: Returns `VL_SOCKET_NULL` on failure.
 * - **Error Conditions**: Returns `VL_SOCKET_NULL` if the subsystem is not initialized, allocation fails, or the
 * underlying socket creation call fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates heap metadata for the socket handle.
 * - **Return-value Semantics**: Returns a valid opaque socket handle on success, or `VL_SOCKET_NULL` on error.
 *
 * \param domain socket address family
 * \param type socket kind
 * \return socket handle, or `VL_SOCKET_NULL` on failure
 */
VL_API vl_socket vlSocketNew(vl_socket_domain domain, vl_socket_type type);

/**
 * \brief Closes and deletes a socket handle.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the socket and associated platform handle.
 * - **Lifetime**: The handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call provided no other thread is concurrently using or deleting the same socket.
 * - **Nullability**: Safe to call with `VL_SOCKET_NULL` (no-op).
 * - **Error Conditions**: None reported through the API.
 * - **Undefined Behavior**: Double deletion or concurrent use-after-delete.
 * - **Memory Allocation Expectations**: Releases heap metadata and the native socket handle.
 * - **Return-value Semantics**: None (void).
 *
 * \param socket socket handle to destroy
 */
VL_API void vlSocketDelete(vl_socket socket);

/**
 * \brief Binds a socket to a local address.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` and `address` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call, but callers must externally synchronize semantic use of the same socket if
 * required.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` or `address` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_BIND` on backend bind failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param address local address to bind
 * \return operation result code
 */
VL_API vl_socket_result vlSocketBind(vl_socket socket, const vl_socket_address* address);

/**
 * \brief Marks a stream socket as a passive listening socket.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_LISTEN` if the backend listen call fails.
 * - **Undefined Behavior**: Calling on an incompatible socket type may fail.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param backlog maximum pending connection backlog
 * \return operation result code
 */
VL_API vl_socket_result vlSocketListen(vl_socket socket, vl_int_t backlog);

/**
 * \brief Accepts an incoming connection from a listening socket.
 *
 * On success, returns a newly allocated socket handle representing the accepted
 * connection.
 *
 * If `outAddress` is non-NULL, it receives the remote peer address.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned socket handle and must release it with `vlSocketDelete`.
 * - **Lifetime**: `socket` must remain valid throughout the call. `outAddress`, if provided, must be writable.
 * - **Thread Safety**: Safe to call, though concurrent accepts on the same listening socket are subject to backend
 * semantics.
 * - **Nullability**: Returns `VL_SOCKET_NULL` if `socket` is `NULL` or if the accept operation fails.
 * - **Error Conditions**: Returns `VL_SOCKET_NULL` on backend failure or if the operation would block in non-blocking
 * mode.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates heap metadata for the accepted socket.
 * - **Return-value Semantics**: Returns a new socket handle on success, or `VL_SOCKET_NULL` on failure.
 *
 * \param socket listening socket
 * \param outAddress optional pointer receiving the remote address
 * \return accepted socket handle, or `VL_SOCKET_NULL` on failure
 */
VL_API vl_socket vlSocketAccept(vl_socket socket, vl_socket_address* outAddress);

/**
 * \brief Connects a socket to a remote address.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` and `address` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` or `address` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_CONNECT` on backend connection failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param address remote address
 * \return operation result code
 */
VL_API vl_socket_result vlSocketConnect(vl_socket socket, const vl_socket_address* address);

/**
 * \brief Sends bytes through a connected socket.
 *
 * This function may perform a partial send. A successful return value smaller
 * than `length` does not indicate an error.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` and `buffer` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call, though callers should externally synchronize concurrent writes when message
 * framing matters.
 * - **Nullability**: Returns `-1` if `socket` or `buffer` is `NULL`.
 * - **Error Conditions**: Returns `-1` if the backend send operation fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the number of bytes sent on success, or `-1` on error.
 *
 * \param socket socket handle
 * \param buffer source buffer
 * \param length number of bytes requested to send
 * \return bytes actually sent, or `-1` on error
 */
VL_API vl_int64_t vlSocketSend(vl_socket socket, const void* buffer, vl_memsize_t length);

/**
 * \brief Receives bytes from a connected socket.
 *
 * A return value of `0` typically indicates an orderly remote shutdown for
 * stream sockets.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` and `buffer` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call, though callers should externally synchronize concurrent reads when protocol
 * framing matters.
 * - **Nullability**: Returns `-1` if `socket` or `buffer` is `NULL`.
 * - **Error Conditions**: Returns `-1` if the backend receive operation fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the number of bytes received on success, `0` on end-of-stream, or `-1` on
 * error.
 *
 * \param socket socket handle
 * \param buffer destination buffer
 * \param length maximum number of bytes to receive
 * \return bytes actually received, `0` on orderly close, or `-1` on error
 */
VL_API vl_int64_t vlSocketReceive(vl_socket socket, void* buffer, vl_memsize_t length);

/**
 * \brief Shuts down part or all of a full-duplex connection.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_SHUTDOWN` if the backend shutdown call fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param how which direction(s) to shut down
 * \return operation result code
 */
VL_API vl_socket_result vlSocketShutdown(vl_socket socket, vl_socket_shutdown how);

/**
 * \brief Configures whether a socket operates in blocking mode.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_SET_OPTION` if the backend mode change fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param blocking `VL_TRUE` for blocking mode, `VL_FALSE` for non-blocking mode
 * \return operation result code
 */
VL_API vl_socket_result vlSocketSetBlocking(vl_socket socket, vl_bool_t blocking);

/**
 * \brief Fills a socket address with an IPv4 address and port.
 *
 * The `port` argument is specified in host byte order.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `address` must remain valid for the duration of the call.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Returns `VL_FALSE` if `address` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` on success, `VL_FALSE` on invalid input.
 *
 * \param address destination address structure
 * \param a first IPv4 octet
 * \param b second IPv4 octet
 * \param c third IPv4 octet
 * \param d fourth IPv4 octet
 * \param port port in host byte order
 * \return boolean indicating success
 */
VL_API vl_bool_t vlSocketAddressSetIPv4(vl_socket_address* address, vl_uint8_t a, vl_uint8_t b, vl_uint8_t c,
                                        vl_uint8_t d, vl_uint16_t port);

/**
 * \brief Fills a socket address with an IPv6 address and port.
 *
 * The sixteen-byte address is copied from `ipv6Bytes`. The `port` argument is
 * specified in host byte order.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `address` and `ipv6Bytes` must remain valid for the duration of the call.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Returns `VL_FALSE` if `address` or `ipv6Bytes` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` on success, `VL_FALSE` on invalid input.
 *
 * \param address destination address structure
 * \param ipv6Bytes pointer to 16 bytes of IPv6 address data
 * \param port port in host byte order
 * \return boolean indicating success
 */
VL_API vl_bool_t vlSocketAddressSetIPv6(vl_socket_address* address, const vl_uint8_t ipv6Bytes[16], vl_uint16_t port);

/**
 * \brief Configures whether local address reuse is enabled for a socket.
 *
 * This function requests that the underlying socket enable or disable the
 * platform's address reuse option, typically corresponding to `SO_REUSEADDR`.
 * This is commonly used for server sockets that need to re-bind to a recently
 * used local address without waiting for normal timeout expiration.
 *
 * The exact backend behavior may vary slightly by platform. In particular,
 * address reuse semantics are not perfectly identical between POSIX systems and
 * Winsock.
 *
 * For best portability, this option should generally be configured before
 * calling `vlSocketBind`.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_SET_OPTION` if the backend socket option call fails.
 * - **Undefined Behavior**: Backend behavior may be implementation-defined if this is called after binding, depending
 * on platform semantics.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param enabled `VL_TRUE` to enable local address reuse, `VL_FALSE` to disable it
 * \return operation result code
 */
VL_API vl_socket_result vlSocketSetReuseAddress(vl_socket socket, vl_bool_t enabled);

/**
 * \brief Configures whether the Nagle algorithm is disabled for a TCP socket.
 *
 * When enabled (no-delay is `VL_TRUE`), small packets are sent as soon as
 * possible without waiting for additional data to fill a segment. This
 * reduces latency at the potential cost of network efficiency.
 *
 * This function typically corresponds to the `TCP_NODELAY` socket option.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_SET_OPTION` if the backend socket option call fails.
 * - **Undefined Behavior**: Calling this on a non-TCP socket is platform-dependent.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param enabled `VL_TRUE` to disable Nagle's algorithm, `VL_FALSE` to enable it
 * \return operation result code
 */
VL_API vl_socket_result vlSocketSetNoDelay(vl_socket socket, vl_bool_t enabled);

/**
 * \brief Configures whether TCP keep-alive probes are enabled.
 *
 * When enabled, the underlying TCP implementation will periodically send
 * keep-alive probes on an idle connection to verify the remote endpoint is
 * still reachable.
 *
 * This function typically corresponds to the `SO_KEEPALIVE` socket option.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_SET_OPTION` if the backend socket option call fails.
 * - **Undefined Behavior**: Calling this on a non-TCP socket is platform-dependent.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param enabled `VL_TRUE` to enable keep-alive, `VL_FALSE` to disable it
 * \return operation result code
 */
VL_API vl_socket_result vlSocketSetKeepAlive(vl_socket socket, vl_bool_t enabled);

/**
 * \brief Checks if local address reuse is enabled for a socket.
 *
 * \param socket socket handle
 * \param outEnabled pointer receiving the enabled status
 * \return operation result code
 */
VL_API vl_socket_result vlSocketGetReuseAddress(vl_socket socket, vl_bool_t* outEnabled);

/**
 * \brief Checks if the Nagle algorithm is disabled for a TCP socket.
 *
 * \param socket socket handle
 * \param outEnabled pointer receiving the enabled status
 * \return operation result code
 */
VL_API vl_socket_result vlSocketGetNoDelay(vl_socket socket, vl_bool_t* outEnabled);

/**
 * \brief Checks if TCP keep-alive probes are enabled.
 *
 * \param socket socket handle
 * \param outEnabled pointer receiving the enabled status
 * \return operation result code
 */
VL_API vl_socket_result vlSocketGetKeepAlive(vl_socket socket, vl_bool_t* outEnabled);

/**
 * \brief Checks if a socket is in blocking mode.
 *
 * \param socket socket handle
 * \param outBlocking pointer receiving the blocking status
 * \return operation result code
 */
VL_API vl_socket_result vlSocketIsBlocking(vl_socket socket, vl_bool_t* outBlocking);

/**
 * \brief Retrieves the remote address that the socket is connected to.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` and `outAddress` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` or `outAddress` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_NOT_CONNECTED` if the socket is not connected or a backend error
 * occurs.
 * - **Undefined Behavior**: Calling this on a socket that is not connected may return platform-defined results.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param outAddress pointer to address structure where the remote address will be stored
 * \return operation result code
 */
VL_API vl_socket_result vlSocketGetRemoteAddress(vl_socket socket, vl_socket_address* outAddress);

/**
 * \brief Converts a socket address to a human-readable string.
 *
 * The output format is typically "a.b.c.d:port" for IPv4 and "[addr]:port" for IPv6.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `address` and `buffer` must remain valid for the duration of the call.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Returns `VL_FALSE` if `address` or `buffer` is `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if `bufferSize` is too small.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` on success, `VL_FALSE` on failure.
 *
 * \param address source address structure
 * \param buffer destination string buffer
 * \param bufferSize size of the destination buffer
 * \return boolean indicating success
 */
VL_API vl_bool_t vlSocketAddressToString(const vl_socket_address* address, char* buffer, vl_memsize_t bufferSize);

/**
 * \brief Parses a human-readable string into a socket address.
 *
 * Supports IPv4 ("a.b.c.d:port") and IPv6 ("[addr]:port") formats.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `address` and `string` must remain valid for the duration of the call.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Returns `VL_FALSE` if `address` or `string` is `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the string format is invalid.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` on success, `VL_FALSE` on failure.
 *
 * \param address destination address structure
 * \param string source string to parse
 * \return boolean indicating success
 */
VL_API vl_bool_t vlSocketAddressFromString(vl_socket_address* address, const char* string);

/**
 * \brief Returns a human-readable description of the most recent socket error.
 *
 * The returned string is owned by the library and must not be modified or
 * freed by the caller. The storage may be thread-local, static, or overwritten
 * by subsequent socket API calls.
 *
 * ## Contract
 * - **Ownership**: The caller does not own the returned string.
 * - **Lifetime**: The pointer remains valid until overwritten by a later backend error query or until program
 * termination, depending on implementation.
 * - **Thread Safety**: Backend-dependent; callers should treat the returned pointer as ephemeral.
 * - **Nullability**: May return `NULL` if no additional error text is available.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Modifying or freeing the returned string.
 * - **Memory Allocation Expectations**: None required by the caller.
 * - **Return-value Semantics**: Returns a pointer to a diagnostic string, or `NULL`.
 *
 * \return error message string or `NULL`
 */
VL_API const char* vlSocketError(void);

/**
 * \brief Retrieves the local address that the socket is bound to.
 *
 * This function retrieves the local address information for the given socket.
 * This is particularly useful after binding to an ephemeral port (port 0)
 * to determine which port was actually assigned by the operating system.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `socket` and `outAddress` must remain valid for the duration of the call.
 * - **Thread Safety**: Safe to call with normal external synchronization expectations for the same socket.
 * - **Nullability**: Returns `VL_SOCKET_ERROR_INVALID_ARGUMENT` if `socket` or `outAddress` is `NULL`.
 * - **Error Conditions**: Returns `VL_SOCKET_ERROR_BIND` or a similar code if the backend `getsockname` call fails.
 * - **Undefined Behavior**: Calling this on a socket that is not yet bound may return platform-defined results.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_SOCKET_SUCCESS` on success, otherwise an error code.
 *
 * \param socket socket handle
 * \param outAddress pointer to address structure where the local address will be stored
 * \return operation result code
 */
VL_API vl_socket_result vlSocketGetLocalAddress(vl_socket socket, vl_socket_address* outAddress);

#endif // VL_SOCKET_H
