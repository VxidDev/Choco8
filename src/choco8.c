#include "../include/choco8.h"
#include "../include/io/readfile.h"

#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

SDL_Window* window;
SDL_Renderer* renderer;

uint8_t MEMORY[4096];
uint16_t PC = 0x200;
uint16_t handledInstructions = 0;

uint8_t V[16];
uint16_t Stack[16];
uint8_t SP = 0;
uint16_t I = 0;

int RUNNING = 1;

#define WIDTH 64
#define HEIGHT 32

#define IDX(x, y) (y * WIDTH + x)

uint8_t SCREEN[WIDTH * HEIGHT];

uint16_t fetchOpcode() {
  uint16_t opcode = (MEMORY[PC] << 8) | MEMORY[PC + 1];
  PC += 2;

  return opcode;
}  

void draw(uint8_t Vx, uint8_t Vy, uint8_t height) {
  V[0xF] = 0;

  for (int y = 0; y < height; y++) {
    uint8_t sprite = MEMORY[I + y];

    for (int x = 0; x < 8; x++) {
      if (sprite & (0x80 >> x)) {
        int px = (Vx + x) % 64;
        int py = (Vy + y) % 32;

        int index = py * 64 + px;

        if (SCREEN[index] == 1)
          V[0xF] = 1;

        SCREEN[index] ^= 1;
      }
    }
  }
}

void render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 64; x++) {
      if (SCREEN[y * 64 + x]) {
        SDL_Rect rect = {
            x * 10,
            y * 10,
            10,
            10
        };

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
      }
    }
  }

  SDL_RenderPresent(renderer);
}

int decodeOpcode(uint16_t opcode) {
  uint8_t group = (opcode & 0xF000) >> 12;

  switch (group) {
    case 0x0: {
      uint16_t nn = opcode & 0x00FF;

      if (nn == 0x00EE) {
        PC = Stack[--SP];
        return 1;
      } else if (nn == 0x00E0) {
        memset(SCREEN, 0, sizeof(SCREEN));
        return 1;
      }

      return 0;
    }
    case 0x1: {
      PC = opcode & 0x0FFF;
      return 1;
    }
    case 0x2: {
      Stack[SP++] = PC;
      PC = opcode & 0x0FFF;
      return 1;
    }
    case 0x3: {
      uint8_t registerID = (opcode & 0x0F00) >> 8;
      uint8_t expectedResult = opcode & 0x00FF;

      if (V[registerID] == expectedResult) {
        PC += 2;
      }

      return 1;
    }
    case 0x4: {
      uint8_t registerID = (opcode & 0x0F00) >> 8;
      uint8_t expectedResult = opcode & 0x00FF;

      if (V[registerID] != expectedResult) {
        PC += 2;
      }

      return 1; 
    }
    case 0x5: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;

      if (V[x] == V[y]) {
        PC += 2;
      }

      return 1;
    }
    case 0x6: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t nn = opcode & 0x00FF;

      V[x] = nn;

      return 1;
    }
    case 0x7: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t nn = opcode & 0x00FF;

      V[x] += nn;

      return 1;
    }
    case 0xA: {
      I = opcode & 0x0FFF;
      return 1;
    }
    case 0xD: {
      uint8_t Vx = (opcode & 0x0F00) >> 8;
      uint8_t Vy = (opcode & 0x00F0) >> 4;

      uint8_t height = opcode & 0x000F;

      draw(V[Vx], V[Vy], height);
      return 1;
    }
    default:
      return 0;
  }

  return 0;
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

  SDL_Init(SDL_INIT_VIDEO);

  window = SDL_CreateWindow(
    "CHIP-8 Emulator",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    640, 320,   // scaled up
    0
  );

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  while (PC != romSize + 0x200 && RUNNING) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) RUNNING = 0;
    }

    uint16_t preFetchPC = PC;
    uint16_t OPCode = fetchOpcode();

    printf("PC: 0x%03x | OPCODE: 0x%04x\n", preFetchPC, OPCode);

    if (decodeOpcode(OPCode)) handledInstructions++;

    render();
  } 

  return 0;
}
