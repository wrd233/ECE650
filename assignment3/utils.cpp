#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>

#include "utils.hpp"

std::string get_host_name(){
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        std::cerr << "Failed to get hostname" << std::endl;
        return "";
    }
    std::string hostNameString(hostname);

    INFO("host name : %s", hostname);

    return hostNameString;
}

std::string get_ip_from_name(std::string hostname){
    struct addrinfo* res;
    const char* service = nullptr;  // 不指定服务名
    int status = getaddrinfo(hostname.c_str(), service, nullptr, &res);
        if (status != 0) {
        std::cerr << "Failed to get address info: " << gai_strerror(status) << std::endl;
    }

    // 只返回第一个IP
    struct addrinfo* addr = res;
    struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(addr->ai_addr);
    char ipAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddr, INET_ADDRSTRLEN);

    INFO("Host's IP Address: %s", ipAddr);

    return ipAddr;
}


int build_server(int port){
    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
	assert( listenfd >= 1 );

    struct sockaddr_in address;
	memset( &address, 0, sizeof( address ) );
	address.sin_family = AF_INET;
	address.sin_port = htons( port );
	inet_pton( AF_INET, get_ip_from_name(get_host_name()).c_str(), &address.sin_addr );

	int ret = 0;
	ret = bind( listenfd, (struct sockaddr*)( &address ), sizeof( address ) );
	assert( ret != -1 );

	ret = listen( listenfd, 100 );
	assert( ret != -1 );

    return listenfd;
}

int build_client(const char * hostname, int port){
    int client_fd = socket( PF_INET, SOCK_STREAM, 0 );
    std::string ip_str = get_ip_from_name(hostname);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;

    // 将 IP 地址从字符串转换为二进制形式，并存储到结构中
    if (inet_pton(AF_INET, ip_str.c_str(), &(addr.sin_addr)) <= 0) {
        perror("inet_pton() failed");
        return -1;
    }

    addr.sin_port = htons(port); // 使用网络字节序表示的端口号

    int ret = connect(client_fd, (struct sockaddr*)( &addr ), sizeof( addr ) );
    assert( ret != -1 );

    return client_fd;
}

int get_port_num(int socket){
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    // 使用 getsockname 函数获取套接字地址信息
    if (getsockname(socket, reinterpret_cast<struct sockaddr*>(&addr), &addr_len) == -1) {
        perror("getsockname() failed");
        return -1;
    }

    // 从套接字地址中提取端口号，并将其转换为主机字节序
    int port = ntohs(addr.sin_port);
    return port;
}