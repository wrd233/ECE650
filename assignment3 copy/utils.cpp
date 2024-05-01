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

using namespace std;

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
    const char* portStr = std::to_string(port).c_str();
    const char * hostname = NULL;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    int status;
    int socket_fd;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, portStr, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    if (strcmp(portStr, "0") == 0) {
        struct sockaddr_in * addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
        addr_in->sin_port = 0;
    }

    socket_fd = socket(host_info_list->ai_family,
                        host_info_list->ai_socktype,
                        host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    status = listen(socket_fd, 100);
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    //cout << "Waiting for connection on port " << port << endl;
    freeaddrinfo(host_info_list);
    return socket_fd;
}

int build_client(const char * hostname, int port){
    int client_fd = socket( PF_INET, SOCK_STREAM, 0 );
    std::string ip_str = get_ip_from_name(hostname);
    struct sockaddr_in addr;
    inet_pton(AF_INET, ip_str.c_str(), &(addr.sin_addr.s_addr));
    addr.sin_family = AF_INET;

    // 将 IP 地址从字符串转换为二进制形式，并存储到结构中
    if (inet_pton(AF_INET, ip_str.c_str(), &(addr.sin_addr)) <= 0) {
        perror("inet_pton() failed");
        return -1;
    }

    addr.sin_port = htons(port); // 使用网络字节序表示的端口号

    // INFO("client ip = %s", ip_str.c_str());

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

int gen_random(int lower_bound, int upper_bound) {
    std::srand((unsigned int)time(NULL) + lower_bound + upper_bound);
    int range = upper_bound - lower_bound + 1;
    return lower_bound + std::rand() % range;
}