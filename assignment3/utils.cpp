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
    std::cout << "IP Address: " << ipAddr << std::endl;

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