#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#include "utils.hpp"

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

    return 0;
}