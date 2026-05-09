#include "../include/choco8.h"
#include "../include/io/readfile.h"

#include <stdio.h>

uint8_t MEMORY[4096];
uint16_t PC = 0x200;
uint16_t handledInstructions = 0;

uint16_t fetchOpcode() {
  uint16_t opcode = (MEMORY[PC] << 8) | MEMORY[PC + 1];
  PC += 2;

  return opcode;
}  

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("choco8: file name required.\n");
    return 1;
  }
  
  long romSize = readFile(argv[1]);

  if (romSize == -1) {
    return 1;
  }

  while (PC != romSize + 0x200) {
    uint16_t preFetchPC = PC;
    uint16_t OPCode = fetchOpcode();

    printf("PC: 0x%03x | OPCODE: 0x%04x\n", preFetchPC, OPCode);
  }
  
  uint16_t amountOfInstructions = (PC - 0x200) / 2;

  printf("\nInstruction amount: %d\n", amountOfInstructions);
  printf("Instructions NOT handled: %d\n", amountOfInstructions - handledInstructions);
  printf("Instructions handled: %d\n", handledInstructions);

  return 0;
}
