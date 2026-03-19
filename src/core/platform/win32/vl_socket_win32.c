#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

struct vl_socket_
{
    SOCKET fd;
    vl_socket_domain domain;
    vl_socket_type type;
    vl_bool_t blocking;
};

static _Thread_local char vl_socket_error_buffer[256];

static void vl_SocketSetError(const char* msg)
{
    if (msg)
    {
        strncpy(vl_socket_error_buffer, msg, sizeof(vl_socket_error_buffer) - 1);
        vl_socket_error_buffer[sizeof(vl_socket_error_buffer) - 1] = '\0';
    }
    else
    {
        vl_socket_error_buffer[0] = '\0';
    }
}

static void vl_SocketSetErrorFromWSA(void)
{
    int err = WSAGetLastError();
    // In a full implementation, we'd map WSA errors to strings.
    // For now, just a placeholder.
    char buf[64];
    snprintf(buf, sizeof(buf), "WSA Error: %d", err);
    vl_SocketSetError(buf);
}

vl_socket_result vlSocketStartup(void)
{
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
        return VL_SOCKET_ERROR_SYSTEM;
    }
    return VL_SOCKET_SUCCESS;
}

void vlSocketShutdownLibrary(void) { WSACleanup(); }

vl_socket vlSocketNew(vl_socket_domain domain, vl_socket_type type)
{
    int family = (domain == VL_SOCKET_DOMAIN_IPV6) ? AF_INET6 : AF_INET;
    int sock_type = (type == VL_SOCKET_TYPE_DATAGRAM) ? SOCK_DGRAM : SOCK_STREAM;

    SOCKET s = socket(family, sock_type, 0);
    if (s == INVALID_SOCKET)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_NULL;
    }

    vl_socket sock = (vl_socket)malloc(sizeof(struct vl_socket_));
    if (!sock)
    {
        closesocket(s);
        vl_SocketSetError("Memory allocation failed");
        return VL_SOCKET_NULL;
    }

    sock->fd = s;
    sock->domain = domain;
    sock->type = type;
    sock->blocking = VL_TRUE;

    return sock;
}

void vlSocketDelete(vl_socket socket)
{
    if (socket == VL_SOCKET_NULL)
        return;
    closesocket(socket->fd);
    free(socket);
}

vl_socket_result vlSocketBind(vl_socket socket, const vl_socket_address* address)
{
    if (socket == VL_SOCKET_NULL || address == NULL)
    {
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    }

    struct sockaddr_storage addr;
    int addr_len;
    memset(&addr, 0, sizeof(addr));

    if (address->domain == VL_SOCKET_DOMAIN_IPV4)
    {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(address->port);
        memcpy(&addr4->sin_addr, address->host.ipv4, 4);
        addr_len = sizeof(struct sockaddr_in);
    }
    else
    {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(address->port);
        memcpy(&addr6->sin6_addr, address->host.ipv6, 16);
        addr_len = sizeof(struct sockaddr_in6);
    }

    if (bind(socket->fd, (struct sockaddr*)&addr, addr_len) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_BIND;
    }

    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketListen(vl_socket socket, vl_int_t backlog)
{
    if (socket == VL_SOCKET_NULL)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    if (listen(socket->fd, (int)backlog) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_LISTEN;
    }

    return VL_SOCKET_SUCCESS;
}

vl_socket vlSocketAccept(vl_socket socket, vl_socket_address* outAddress)
{
    if (socket == VL_SOCKET_NULL)
        return VL_SOCKET_NULL;

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    SOCKET client_fd = accept(socket->fd, (struct sockaddr*)&addr, &addr_len);

    if (client_fd == INVALID_SOCKET)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            vl_SocketSetErrorFromWSA();
        }
        return VL_SOCKET_NULL;
    }

    vl_socket client_sock = (vl_socket)malloc(sizeof(struct vl_socket_));
    if (!client_sock)
    {
        closesocket(client_fd);
        vl_SocketSetError("Memory allocation failed");
        return VL_SOCKET_NULL;
    }

    client_sock->fd = client_fd;
    client_sock->domain = socket->domain;
    client_sock->type = socket->type;
    client_sock->blocking = socket->blocking;

    if (outAddress)
    {
        if (addr.ss_family == AF_INET)
        {
            struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
            outAddress->domain = VL_SOCKET_DOMAIN_IPV4;
            outAddress->port = ntohs(addr4->sin_port);
            memcpy(outAddress->host.ipv4, &addr4->sin_addr, 4);
        }
        else if (addr.ss_family == AF_INET6)
        {
            struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;
            outAddress->domain = VL_SOCKET_DOMAIN_IPV6;
            outAddress->port = ntohs(addr6->sin6_port);
            memcpy(outAddress->host.ipv6, &addr6->sin6_addr, 16);
        }
    }

    return client_sock;
}

vl_socket_result vlSocketConnect(vl_socket socket, const vl_socket_address* address)
{
    if (socket == VL_SOCKET_NULL || address == NULL)
    {
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    }

    struct sockaddr_storage addr;
    int addr_len;
    memset(&addr, 0, sizeof(addr));

    if (address->domain == VL_SOCKET_DOMAIN_IPV4)
    {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(address->port);
        memcpy(&addr4->sin_addr, address->host.ipv4, 4);
        addr_len = sizeof(struct sockaddr_in);
    }
    else
    {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(address->port);
        memcpy(&addr6->sin6_addr, address->host.ipv6, 16);
        addr_len = sizeof(struct sockaddr_in6);
    }

    if (connect(socket->fd, (struct sockaddr*)&addr, addr_len) == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            return VL_SOCKET_ERROR_WOULD_BLOCK;
        }
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_CONNECT;
    }

    return VL_SOCKET_SUCCESS;
}

vl_int64_t vlSocketSend(vl_socket socket, const void* buffer, vl_memsize_t length)
{
    if (socket == VL_SOCKET_NULL || buffer == VL_SOCKET_NULL)
        return -1;

    int sent = send(socket->fd, (const char*)buffer, (int)length, 0);
    if (sent == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return -1;
    }

    return (vl_int64_t)sent;
}

vl_int64_t vlSocketReceive(vl_socket socket, void* buffer, vl_memsize_t length)
{
    if (socket == VL_SOCKET_NULL || buffer == VL_SOCKET_NULL)
        return -1;

    int received = recv(socket->fd, (char*)buffer, (int)length, 0);
    if (received == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            vl_SocketSetErrorFromWSA();
        }
        return -1;
    }

    return (vl_int64_t)received;
}

vl_socket_result vlSocketShutdown(vl_socket socket, vl_socket_shutdown how)
{
    if (socket == VL_SOCKET_NULL)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    int sh;
    switch (how)
    {
    case VL_SOCKET_SHUTDOWN_RECEIVE:
        sh = SD_RECEIVE;
        break;
    case VL_SOCKET_SHUTDOWN_SEND:
        sh = SD_SEND;
        break;
    case VL_SOCKET_SHUTDOWN_BOTH:
        sh = SD_BOTH;
        break;
    default:
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    }

    if (shutdown(socket->fd, sh) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SHUTDOWN;
    }

    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketSetBlocking(vl_socket socket, vl_bool_t blocking)
{
    if (socket == VL_SOCKET_NULL)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    u_long mode = blocking ? 0 : 1;
    if (ioctlsocket(socket->fd, FIONBIO, &mode) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SET_OPTION;
    }

    socket->blocking = blocking;
    return VL_SOCKET_SUCCESS;
}

vl_bool_t vlSocketAddressSetIPv4(vl_socket_address* address, vl_uint8_t a, vl_uint8_t b, vl_uint8_t c, vl_uint8_t d,
                                 vl_uint16_t port)
{
    if (!address)
        return VL_FALSE;

    address->domain = VL_SOCKET_DOMAIN_IPV4;
    address->port = port;
    address->host.ipv4[0] = a;
    address->host.ipv4[1] = b;
    address->host.ipv4[2] = c;
    address->host.ipv4[3] = d;

    return VL_TRUE;
}

vl_bool_t vlSocketAddressSetIPv6(vl_socket_address* address, const vl_uint8_t ipv6Bytes[16], vl_uint16_t port)
{
    if (!address || !ipv6Bytes)
        return VL_FALSE;

    address->domain = VL_SOCKET_DOMAIN_IPV6;
    address->port = port;
    memcpy(address->host.ipv6, ipv6Bytes, 16);

    return VL_TRUE;
}

vl_socket_result vlSocketSetReuseAddress(vl_socket socket, vl_bool_t enabled)
{
    if (socket == VL_SOCKET_NULL)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    BOOL opt = enabled ? TRUE : FALSE;
    if (setsockopt(socket->fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SET_OPTION;
    }

    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketSetNoDelay(vl_socket socket, vl_bool_t enabled)
{
    if (socket == VL_SOCKET_NULL)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    BOOL opt = enabled ? TRUE : FALSE;
    if (setsockopt(socket->fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SET_OPTION;
    }

    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketSetKeepAlive(vl_socket socket, vl_bool_t enabled)
{
    if (socket == VL_SOCKET_NULL)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    BOOL opt = enabled ? TRUE : FALSE;
    if (setsockopt(socket->fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SET_OPTION;
    }

    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketGetReuseAddress(vl_socket socket, vl_bool_t* outEnabled)
{
    if (socket == VL_SOCKET_NULL || !outEnabled)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    int opt = 0;
    int len = sizeof(opt);
    if (getsockopt(socket->fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, &len) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SET_OPTION;
    }

    *outEnabled = opt ? VL_TRUE : VL_FALSE;
    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketGetNoDelay(vl_socket socket, vl_bool_t* outEnabled)
{
    if (socket == VL_SOCKET_NULL || !outEnabled)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    int opt = 0;
    int len = sizeof(opt);
    if (getsockopt(socket->fd, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, &len) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SET_OPTION;
    }

    *outEnabled = opt ? VL_TRUE : VL_FALSE;
    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketGetKeepAlive(vl_socket socket, vl_bool_t* outEnabled)
{
    if (socket == VL_SOCKET_NULL || !outEnabled)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;

    int opt = 0;
    int len = sizeof(opt);
    if (getsockopt(socket->fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt, &len) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_SET_OPTION;
    }

    *outEnabled = opt ? VL_TRUE : VL_FALSE;
    return VL_SOCKET_SUCCESS;
}

vl_socket_result vlSocketIsBlocking(vl_socket socket, vl_bool_t* outBlocking)
{
    if (socket == VL_SOCKET_NULL || !outBlocking)
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    *outBlocking = socket->blocking;
    return VL_SOCKET_SUCCESS;
}

const char* vlSocketError(void) { return vl_socket_error_buffer; }

vl_socket_result vlSocketGetRemoteAddress(vl_socket socket, vl_socket_address* outAddress)
{
    if (socket == VL_SOCKET_NULL || outAddress == NULL)
    {
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    }

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    if (getpeername(socket->fd, (struct sockaddr*)&addr, &addr_len) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_NOT_CONNECTED;
    }

    if (addr.ss_family == AF_INET)
    {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
        outAddress->domain = VL_SOCKET_DOMAIN_IPV4;
        outAddress->port = ntohs(addr4->sin_port);
        memcpy(outAddress->host.ipv4, &addr4->sin_addr, 4);
    }
    else if (addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;
        outAddress->domain = VL_SOCKET_DOMAIN_IPV6;
        outAddress->port = ntohs(addr6->sin6_port);
        memcpy(outAddress->host.ipv6, &addr6->sin6_addr, 16);
    }
    else
    {
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    }

    return VL_SOCKET_SUCCESS;
}

vl_bool_t vlSocketAddressToString(const vl_socket_address* address, char* buffer, vl_memsize_t bufferSize)
{
    if (!address || !buffer || bufferSize == 0)
        return VL_FALSE;

    if (address->domain == VL_SOCKET_DOMAIN_IPV4)
    {
        char addrStr[INET_ADDRSTRLEN];
        struct in_addr in;
        memcpy(&in, address->host.ipv4, 4);
        if (inet_ntop(AF_INET, &in, addrStr, sizeof(addrStr)) == NULL)
            return VL_FALSE;

        int written = _snprintf(buffer, bufferSize, "%s:%u", addrStr, address->port);
        return (written > 0 && (vl_memsize_t)written < bufferSize);
    }
    else if (address->domain == VL_SOCKET_DOMAIN_IPV6)
    {
        char addrStr[INET6_ADDRSTRLEN];
        struct in6_addr in6;
        memcpy(&in6, address->host.ipv6, 16);
        if (inet_ntop(AF_INET6, &in6, addrStr, sizeof(addrStr)) == NULL)
            return VL_FALSE;

        int written = _snprintf(buffer, bufferSize, "[%s]:%u", addrStr, address->port);
        return (written > 0 && (vl_memsize_t)written < bufferSize);
    }

    return VL_FALSE;
}

vl_bool_t vlSocketAddressFromString(vl_socket_address* address, const char* string)
{
    if (!address || !string)
        return VL_FALSE;

    char host[256];
    vl_uint16_t port = 0;

    // Try IPv6 format [addr]:port
    if (string[0] == '[')
    {
        const char* endBracket = strchr(string, ']');
        if (!endBracket || endBracket[1] != ':')
            return VL_FALSE;

        size_t hostLen = endBracket - string - 1;
        if (hostLen >= sizeof(host))
            return VL_FALSE;
        memcpy(host, string + 1, hostLen);
        host[hostLen] = '\0';

        port = (vl_uint16_t)atoi(endBracket + 2);

        struct in6_addr in6;
        if (inet_pton(AF_INET6, host, &in6) <= 0)
            return VL_FALSE;

        address->domain = VL_SOCKET_DOMAIN_IPV6;
        address->port = port;
        memcpy(address->host.ipv6, &in6, 16);
        return VL_TRUE;
    }
    else
    {
        // Try IPv4 format addr:port
        const char* colon = strrchr(string, ':');
        if (!colon)
            return VL_FALSE;

        size_t hostLen = colon - string;
        if (hostLen >= sizeof(host))
            return VL_FALSE;
        memcpy(host, string, hostLen);
        host[hostLen] = '\0';

        port = (vl_uint16_t)atoi(colon + 1);

        struct in_addr in;
        if (inet_pton(AF_INET, host, &in) <= 0)
            return VL_FALSE;

        address->domain = VL_SOCKET_DOMAIN_IPV4;
        address->port = port;
        memcpy(address->host.ipv4, &in, 4);
        return VL_TRUE;
    }
}

vl_socket_result vlSocketGetLocalAddress(vl_socket socket, vl_socket_address* outAddress)
{
    if (socket == VL_SOCKET_NULL || outAddress == NULL)
    {
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    }

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    if (getsockname(socket->fd, (struct sockaddr*)&addr, &addr_len) == SOCKET_ERROR)
    {
        vl_SocketSetErrorFromWSA();
        return VL_SOCKET_ERROR_BIND;
    }

    if (addr.ss_family == AF_INET)
    {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
        outAddress->domain = VL_SOCKET_DOMAIN_IPV4;
        outAddress->port = ntohs(addr4->sin_port);
        memcpy(outAddress->host.ipv4, &addr4->sin_addr, 4);
    }
    else if (addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;
        outAddress->domain = VL_SOCKET_DOMAIN_IPV6;
        outAddress->port = ntohs(addr6->sin6_port);
        memcpy(outAddress->host.ipv6, &addr6->sin6_addr, 16);
    }
    else
    {
        return VL_SOCKET_ERROR_INVALID_ARGUMENT;
    }

    return VL_SOCKET_SUCCESS;
}
