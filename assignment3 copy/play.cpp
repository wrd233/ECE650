#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include "utils.hpp"
#include "potato.hpp"

using namespace std;

int main(int argc, char * argv[]){
    if (argc != 3) {
        printf("Usage: %s <machine_name> <port_num>\n", argv[0]);
        return 1;
    }
    const char * master_hostname = argv[1];
    int master_port = atoi(argv[2]);
    int player_id;
    int num_players;

    int master_fd = build_client(master_hostname, master_port);
    recv(master_fd, &player_id, sizeof(player_id), 0);
    recv(master_fd, &num_players, sizeof(num_players), 0);
    INFO("被分配到的player_id = %d", player_id);

    // 监听一个端口，用来接受来自其他player的信息
    int player_fd = build_server(0);  // 传入端口0，分配一个临时端口
    int player_fd_port = get_port_num(player_fd);
    INFO("player[%d]所占用的端口为 %d", player_id, player_fd_port);
    send(master_fd, &player_fd_port, sizeof(player_fd_port), 0);

    // 分配邻居
    int neighbor_port;
    recv(master_fd, &neighbor_port, sizeof(neighbor_port), MSG_WAITALL);
    char buf_size[1024] = {0};
    recv(master_fd, buf_size, sizeof(buf_size), MSG_WAITALL);
    std::string neighbor_ip(buf_size);

    INFO("邻居的IP = %s, port = %d", buf_size, neighbor_port);

    // 建立socket与左右邻居进行联系
    int right_neighbor_fd = build_client(neighbor_ip.c_str(), neighbor_port);

    struct sockaddr_in left_neighbor_socket;
    socklen_t addrlength = sizeof(left_neighbor_socket);
    int left_neighbor_fd = accept(player_fd, (struct sockaddr*)(&left_neighbor_socket), &addrlength);

    INFO("与左右邻居完成连接的建立");

    // 进行select，IO多路复用
    // 创建用于监视的文件描述符集合
    fd_set readfds;
    Potato potato;
    std::vector<int> fds{left_neighbor_fd, right_neighbor_fd, master_fd};
    while(1){
        FD_ZERO(&readfds);
        for(int i=0; i<3; i++){
            FD_SET(fds[i], &readfds);
        }
        // 计算最大文件描述符值
        int maxfd = *std::max_element(fds.begin(), fds.end());;

        // 调用 select() 进行多路复用
        int ready = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);
        int buffer_size;
        for(int i=0; i<3; i++){
            if(FD_ISSET(fds[i], &readfds)){
                buffer_size = recv(fds[i], &potato, sizeof(potato), MSG_WAITALL);
                INFO("收到来自 %d 的土豆", i);
                break;
            }
        }

        //receive num_hops =0 potato from master or shut down signal 0 from other socket, shut down
        if (potato.num_hops == 0 || buffer_size == 0) {
            break;
        }
        //send potato to master
        else if (potato.num_hops == 1) {
            potato.num_hops--;
            potato.path[potato.count] = player_id;
            potato.count++;
            send(master_fd, &potato, sizeof(potato), 0);
            cout << "I'm it" << endl;
        }
        //send potato to neighbor
        else {
            potato.num_hops--;
            potato.path[potato.count] = player_id;
            potato.count++;
            int random = rand() % 2;
            if (random == 0) {
                send(left_neighbor_fd, &potato, sizeof(potato), 0);
                int left_neighbor_id = (player_id + num_players - 1) % num_players;
                cout << "Sending potato to " << left_neighbor_id << endl;
            }
            else {
                send(right_neighbor_fd, &potato, sizeof(potato), 0);
                int right_neighbor_id = (player_id + 1) % num_players;
                cout << "Sending potato to " << right_neighbor_id << endl;
            }
        }

    }

    return 0;
}