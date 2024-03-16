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

    int player_fd_port = 114514;
    send(master_fd, &player_fd_port, sizeof(player_fd_port), 0);


    return 0;
}