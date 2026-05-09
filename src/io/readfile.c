#include "../../include/io/readfile.h"
#include "../../include/choco8.h"

#include <stdio.h>

long readFile(const char *filename) {
  FILE* file = fopen(filename, "rb");

  if (!file) {
    perror("choco8");
    return -1;
  }

  fseek(file, 0, SEEK_END);
  long romSize = ftell(file);
  
  if (romSize < 0) {
    perror("choco8");
    fclose(file);
    return -1;
  }

  rewind(file);

  if (romSize > (4096 - 0x200)) {
    fprintf(stderr, "ROM too large.\n");
    return -1;
  }

  size_t read = fread(MEMORY + 0x200, 1, romSize, file);

  if (read != romSize) {
    fprintf(stderr, "Failed to read ROM fully.\n");
    fclose(file);
    return -1;
  }

  fclose(file);

  return romSize;
}
