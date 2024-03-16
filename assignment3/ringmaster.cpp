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

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <port_num> <num_players> <num_hops>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int num_players = atoi(argv[2]);
    int num_hops = atoi(argv[3]);

    // 在这里使用接收到的参数进行后续操作
    printf("Potato Ringmaster\n");
    printf("Players = %d\n", num_players);
    printf("Hops = %d\n", num_hops);

    int listen_fd = build_server(port);

    for(int i=0; i<num_players; i++){
        struct sockaddr_in client;
	    socklen_t client_addrlength = sizeof( client );
        int player_fd = accept( listen_fd, (struct sockaddr*)(&client), &client_addrlength );

        // 测试性质的输入
        INFO("连入的player的IP为: %s",inet_ntoa(client.sin_addr));

        // 发送必要的信息
        send(player_fd, &i, sizeof(i), 0);
        send(player_fd, &num_players, sizeof(num_players), 0);

        // 接受player提供的port
        int player_port;
        recv(player_fd, &player_port, sizeof(player_port), 0);
        INFO("Player的端口为: %d", player_port);
        
        // close( player_fd );

        std::cout << "Player " << i << " is ready to play " << std::endl;
    }

    return 0;
}
