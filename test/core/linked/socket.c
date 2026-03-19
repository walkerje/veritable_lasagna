#include "socket.h"
#include <vl/vl_socket.h>
#include <vl/vl_thread.h>
#include <string.h>

vl_bool_t vlTestSocketCreateAndDelete(void) {
    if (vlSocketStartup() != VL_SOCKET_SUCCESS) return VL_FALSE;
    vl_socket sock = vlSocketNew(VL_SOCKET_DOMAIN_IPV4, VL_SOCKET_TYPE_STREAM);
    if (sock == VL_SOCKET_NULL) {
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }
    vlSocketDelete(sock);
    vlSocketShutdownLibrary();
    return VL_TRUE;
}

vl_bool_t vlTestSocketAddressIPv4(void) {
    vl_socket_address addr;
    if (!vlSocketAddressSetIPv4(&addr, 127, 0, 0, 1, 8080)) return VL_FALSE;
    if (addr.domain != VL_SOCKET_DOMAIN_IPV4) return VL_FALSE;
    if (addr.port != 8080) return VL_FALSE;
    if (addr.host.ipv4[0] != 127) return VL_FALSE;
    if (addr.host.ipv4[3] != 1) return VL_FALSE;
    return VL_TRUE;
}

vl_bool_t vlTestSocketOptions(void) {
    if (vlSocketStartup() != VL_SOCKET_SUCCESS) return VL_FALSE;
    vl_socket sock = vlSocketNew(VL_SOCKET_DOMAIN_IPV4, VL_SOCKET_TYPE_STREAM);
    if (sock == VL_SOCKET_NULL) {
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }

    vl_bool_t success = VL_TRUE;

    if (vlSocketSetReuseAddress(sock, VL_TRUE) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (vlSocketSetReuseAddress(sock, VL_FALSE) != VL_SOCKET_SUCCESS) success = VL_FALSE;

    if (vlSocketSetNoDelay(sock, VL_TRUE) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (vlSocketSetNoDelay(sock, VL_FALSE) != VL_SOCKET_SUCCESS) success = VL_FALSE;

    if (vlSocketSetKeepAlive(sock, VL_TRUE) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (vlSocketSetKeepAlive(sock, VL_FALSE) != VL_SOCKET_SUCCESS) success = VL_FALSE;

    vl_bool_t enabled;
    if (vlSocketGetReuseAddress(sock, &enabled) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (enabled != VL_FALSE) success = VL_FALSE;
    if (vlSocketSetReuseAddress(sock, VL_TRUE) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (vlSocketGetReuseAddress(sock, &enabled) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (enabled != VL_TRUE) success = VL_FALSE;

    if (vlSocketGetNoDelay(sock, &enabled) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (enabled != VL_FALSE) success = VL_FALSE;
    if (vlSocketSetNoDelay(sock, VL_TRUE) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (vlSocketGetNoDelay(sock, &enabled) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (enabled != VL_TRUE) success = VL_FALSE;

    if (vlSocketGetKeepAlive(sock, &enabled) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (enabled != VL_FALSE) success = VL_FALSE;
    if (vlSocketSetKeepAlive(sock, VL_TRUE) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (vlSocketGetKeepAlive(sock, &enabled) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (enabled != VL_TRUE) success = VL_FALSE;

    vl_bool_t blocking;
    if (vlSocketIsBlocking(sock, &blocking) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (blocking != VL_TRUE) success = VL_FALSE;
    if (vlSocketSetBlocking(sock, VL_FALSE) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (vlSocketIsBlocking(sock, &blocking) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (blocking != VL_FALSE) success = VL_FALSE;

    vlSocketDelete(sock);
    vlSocketShutdownLibrary();
    return success;
}

vl_bool_t vlTestSocketAddressConversion(void) {
    vl_socket_address addr;
    char buffer[128];

    // IPv4
    if (!vlSocketAddressSetIPv4(&addr, 127, 0, 0, 1, 8080)) return VL_FALSE;
    if (!vlSocketAddressToString(&addr, buffer, sizeof(buffer))) return VL_FALSE;
    if (strcmp(buffer, "127.0.0.1:8080") != 0) return VL_FALSE;

    vl_socket_address parsed;
    if (!vlSocketAddressFromString(&parsed, "127.0.0.1:8080")) return VL_FALSE;
    if (parsed.domain != VL_SOCKET_DOMAIN_IPV4) return VL_FALSE;
    if (parsed.port != 8080) return VL_FALSE;
    if (parsed.host.ipv4[0] != 127) return VL_FALSE;
    if (parsed.host.ipv4[3] != 1) return VL_FALSE;

    // IPv6
    vl_uint8_t ipv6[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    if (!vlSocketAddressSetIPv6(&addr, ipv6, 443)) return VL_FALSE;
    if (!vlSocketAddressToString(&addr, buffer, sizeof(buffer))) return VL_FALSE;
    if (strcmp(buffer, "[::1]:443") != 0) return VL_FALSE;

    if (!vlSocketAddressFromString(&parsed, "[::1]:443")) return VL_FALSE;
    if (parsed.domain != VL_SOCKET_DOMAIN_IPV6) return VL_FALSE;
    if (parsed.port != 443) return VL_FALSE;
    if (parsed.host.ipv6[15] != 1) return VL_FALSE;

    return VL_TRUE;
}

vl_bool_t vlTestSocketInvalidArguments(void) {
    if (vlSocketStartup() != VL_SOCKET_SUCCESS) return VL_FALSE;
    vl_bool_t success = VL_TRUE;
    if (vlSocketSetReuseAddress(VL_SOCKET_NULL, VL_TRUE) != VL_SOCKET_ERROR_INVALID_ARGUMENT) success = VL_FALSE;
    if (vlSocketSetNoDelay(VL_SOCKET_NULL, VL_TRUE) != VL_SOCKET_ERROR_INVALID_ARGUMENT) success = VL_FALSE;
    if (vlSocketSetKeepAlive(VL_SOCKET_NULL, VL_TRUE) != VL_SOCKET_ERROR_INVALID_ARGUMENT) success = VL_FALSE;
    vlSocketShutdownLibrary();
    return success;
}

struct AcceptArgs {
    vl_socket listener;
    vl_socket accepted;
};

static void acceptThreadProc(void* arg) {
    struct AcceptArgs* a = (struct AcceptArgs*)arg;
    a->accepted = vlSocketAccept(a->listener, NULL);
}

vl_bool_t vlTestSocketLoopbackTCP(void) {
    if (vlSocketStartup() != VL_SOCKET_SUCCESS) return VL_FALSE;

    vl_socket_address listenAddr;
    vlSocketAddressSetIPv4(&listenAddr, 127, 0, 0, 1, 0); // Port 0 for ephemeral

    vl_socket listener = vlSocketNew(VL_SOCKET_DOMAIN_IPV4, VL_SOCKET_TYPE_STREAM);
    if (listener == VL_SOCKET_NULL) {
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }
    
    vlSocketSetReuseAddress(listener, VL_TRUE);

    if (vlSocketBind(listener, &listenAddr) != VL_SOCKET_SUCCESS) {
        vlSocketDelete(listener);
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }
    
    vl_socket_address localAddr;
    if (vlSocketGetLocalAddress(listener, &localAddr) != VL_SOCKET_SUCCESS) {
        vlSocketDelete(listener);
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }

    if (vlSocketListen(listener, 5) != VL_SOCKET_SUCCESS) {
        vlSocketDelete(listener);
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }

    vl_socket client = vlSocketNew(VL_SOCKET_DOMAIN_IPV4, VL_SOCKET_TYPE_STREAM);
    if (client == VL_SOCKET_NULL) {
        vlSocketDelete(listener);
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }

    struct AcceptArgs args;
    args.listener = listener;
    args.accepted = VL_SOCKET_NULL;

    vl_thread thread = vlThreadNew(acceptThreadProc, &args);
    if (thread == (vl_thread)NULL) {
        vlSocketDelete(client);
        vlSocketDelete(listener);
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }

    if (vlSocketConnect(client, &localAddr) != VL_SOCKET_SUCCESS) {
        vlThreadJoin(thread);
        vlThreadDelete(thread);
        vlSocketDelete(client);
        vlSocketDelete(listener);
        vlSocketShutdownLibrary();
        return VL_FALSE;
    }

    vl_socket_address remoteAddr;
    vl_bool_t success = VL_TRUE;
    if (vlSocketGetRemoteAddress(client, &remoteAddr) != VL_SOCKET_SUCCESS) success = VL_FALSE;
    if (remoteAddr.domain != VL_SOCKET_DOMAIN_IPV4) success = VL_FALSE;
    if (remoteAddr.port != localAddr.port) success = VL_FALSE;
    if (remoteAddr.host.ipv4[0] != 127) success = VL_FALSE;
    if (remoteAddr.host.ipv4[3] != 1) success = VL_FALSE;

    vlThreadJoin(thread);
    vlThreadDelete(thread);

    if (args.accepted == VL_SOCKET_NULL) success = VL_FALSE;

    if (success) {
        const char* msg = "Hello Socket";
        vl_int64_t sent = vlSocketSend(client, msg, (vl_memsize_t)strlen(msg));
        if (sent != (vl_int64_t)strlen(msg)) success = VL_FALSE;

        char buffer[64];
        vl_int64_t received = vlSocketReceive(args.accepted, buffer, (vl_memsize_t)sizeof(buffer));
        if (received <= 0) {
            success = VL_FALSE;
        } else {
            buffer[received] = '\0';
            if (strcmp(buffer, msg) != 0) success = VL_FALSE;
        }
    }

    vlSocketDelete(client);
    if (args.accepted != VL_SOCKET_NULL) vlSocketDelete(args.accepted);
    vlSocketDelete(listener);
    vlSocketShutdownLibrary();

    return success;
}
