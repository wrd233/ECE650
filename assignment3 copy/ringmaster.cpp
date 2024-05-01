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
#include <vector>
#include <algorithm>

#include "utils.hpp"
#include "potato.hpp"

using namespace std;

struct PlayerItem{
    int fd;
    int port;
    std::string ip;
};

void printPlayers(std::vector<PlayerItem>& players){
    std::cout<<"当前players的个数为: "<<players.size()<<std::endl;;

    for(int i=0; i<players.size(); i++){
        PlayerItem temp = players[i];
        printf("[%d] port = %d ", i, temp.port);
        std::cout<< temp.ip<<std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <port_num> <num_players> <num_hops>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int num_players = atoi(argv[2]);
    int num_hops = atoi(argv[3]);

    std::vector<PlayerItem> players;

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
        std::string ipStr(inet_ntoa(client.sin_addr));

        // 发送必要的信息
        send(player_fd, &i, sizeof(i), 0);
        send(player_fd, &num_players, sizeof(num_players), 0);

        // 接受player提供的port
        int player_port;
        recv(player_fd, &player_port, sizeof(player_port), 0);
        INFO("Player的端口为: %d", player_port);
        
        // 将当前的player加入到vector中
        players.push_back({player_fd, player_port, ipStr});

        std::cout << "Player " << i << " is ready to play " << std::endl;
    }

    printPlayers(players);


    // 传递neighbor
    for(int i=0; i<num_players; i++){
        PlayerItem client = players[i];
        PlayerItem neighbor = players[(i+1) % num_players];
        send(client.fd, &neighbor.port, sizeof(neighbor.port), 0);
        send(client.fd, neighbor.ip.c_str(), neighbor.ip.length(), 0);
    }

    // 生成土豆
    Potato potato(num_hops);

    vector<int> all_player_fd;
    for(int i=0; i<players.size(); i++){
        all_player_fd.push_back(players[i].fd);
    }

    if(num_hops != 0){
        int random = gen_random(0, num_players-1);
        send(players[random].fd, &potato, sizeof(potato), 0);
        std::cout << "Ready to start the game, sending potato to player " << random << std::endl;

        // TODO: 等待从player中获得最后一个土豆
        //receive last potato
        fd_set readfds;
        int nfds = *max_element(all_player_fd.begin(), all_player_fd.end());
        FD_ZERO(&readfds);
        for (int i = 0; i < num_players; i++) {
            FD_SET(players[i].fd, &readfds);
        }
        select(nfds + 1, &readfds, NULL, NULL, NULL);
        for (int i = 0; i < num_players; i++) {
            if (FD_ISSET(players[i].fd, &readfds)) {
                recv(players[i].fd, &potato, sizeof(potato), MSG_WAITALL);
                INFO("最终收到了来自 %d 的土豆", i);
                break;
            }
        }
    }

    return 0;
}
