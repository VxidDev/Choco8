#include "../include/choco8.h"
#include "../include/io/readfile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

SDL_Window* window;
SDL_Renderer* renderer;

SDL_AudioDeviceID audioDevice;
int audioEnabled = 0;

uint8_t MEMORY[4096];
uint16_t PC = 0x200;
uint16_t handledInstructions = 0;

uint8_t V[16];
uint16_t Stack[16];
uint8_t SP = 0;
uint16_t I = 0;

uint8_t delayTimer = 0;
uint8_t soundTimer = 0;

uint8_t waitingForKey = 0;
uint8_t waitingRegister = 0;

int RUNNING = 1;

#define WIDTH 64
#define HEIGHT 32

#define IDX(x, y) (y * WIDTH + x)

uint8_t SCREEN[WIDTH * HEIGHT];
uint8_t KEYPAD[16];

char keymap[16] = {
    'x', '1', '2', '3',
    'q', 'w', 'e', 'a',
    's', 'd', 'z', 'c',
    '4', 'r', 'f', 'v'
};

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
        if (SP == 0) {
          fprintf(stderr, "Stack underflow\n");
          return 1;
        }

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
      if (SP >= 16) {
        fprintf(stderr, "Stack overflow\n");
        return 1;
      }

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
    case 0x8: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;
      
      uint8_t n = opcode & 0x000F;

      switch (n) {
        case 0x0:
          V[x] = V[y];
          break;
        case 0x1:
          V[x] |= V[y];
          V[0xF] = 0;
          break;
        case 0x2:
          V[x] &= V[y];
          V[0xF] = 0;
          break;
        case 0x3:
          V[x] ^= V[y];
          V[0xF] = 0;
          break;
        case 0x4:
          uint16_t sum = V[x] + V[y];

          V[0xF] = sum > 0xFF;
          V[x] = sum & 0xFF;
          break;
        case 0x5:
          V[0xF] = V[x] >= V[y];
          V[x] -= V[y];
          break;
        case 0x6:
          V[x] = V[y];
          V[0xF] = V[x] & 1;
          V[x] >>= 1;
          break;
        case 0x7:
          V[0xF] = V[y] >= V[x];
          V[x] = V[y] - V[x];
          break;
        case 0xE:
          V[x] = V[y];
          V[0xF] = (V[x] & 0x80) >> 7;
          V[x] <<= 1;
          break;
      }
      return 1;
    }
    case 0x9: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;

      if (V[x] != V[y]) {
        PC += 2;
      }

      return 1;
    }
    case 0xA: {
      I = opcode & 0x0FFF;
      return 1;
    }
    case 0xB: {
      PC = (opcode & 0x0FFF) + V[0];
      return 1;
    }
    case 0xC: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint16_t nn = opcode & 0x0FF;
      V[x] = (rand() % 256) & nn;

      return 1;
    }
    case 0xD: {
      uint8_t Vx = (opcode & 0x0F00) >> 8;
      uint8_t Vy = (opcode & 0x00F0) >> 4;

      uint8_t height = opcode & 0x000F;

      draw(V[Vx], V[Vy], height);
      return 1;
    }
    case 0xE: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint16_t nn = opcode & 0x00FF;

      if (nn == 0x009E) {
        if (KEYPAD[V[x]] == 1) {
          PC += 2;
        }

        return 1;
      } else if (nn == 0x00A1) {
        if (KEYPAD[V[x]] == 0) {
          PC += 2;
        }

        return 1;
      }
      
      return 0;
    }
    case 0xF: {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t nn = opcode & 0x00FF;

      if (nn == 0x0007) {
        V[x] = delayTimer;
        return 1;
      }

      if (nn == 0x000A) {
        waitingRegister = x;
        waitingForKey = 1;

        return 1;
      }

      if (nn == 0x0015) {
        delayTimer = V[x];
        return 1;
      }

      if (nn == 0x0018) {
        soundTimer = V[x];
        return 1;
      }

      if (nn == 0x001E) {
        I += V[x];
        return 1;
      }

      if (nn == 0x0029) {
        I = 0x50 + (V[x] * 5);
        return 1;
      }

      if (nn == 0x0033) {
        uint8_t value = V[x];

        MEMORY[I] = value / 100;
        MEMORY[I + 1] = (value / 10) % 10;
        MEMORY[I + 2] = value % 10;

        return 1;
      }

      if (nn == 0x0055) {
        for (int i = 0; i <= x; i++) {
          MEMORY[I + i] = V[i];
        }

        I += x + 1;

        return 1;
      }

      if (nn == 0x0065) {
        for (int i = 0; i <= x; i++) {
          V[i] = MEMORY[I + i];
        }

        I += x + 1;

        return 1;
      }
    }
    default:
      return 0;
  }

  return 0;
}

void audioCallback(void *userdata, Uint8 *stream, int len) {
  Sint16 *buffer = (Sint16*)stream;

  static int phase = 0;
  int frequency = 440; // A4 tone
  
  int active = soundTimer > 0;

  for (int i = 0; i < len / 2; i++) {
      buffer[i] = active ? ((phase < 22050 / frequency) ? 3000 : -3000) : 0;
      phase = (phase + 1) % (44100 / frequency);
  }
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

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  SDL_AudioSpec spec;
  SDL_zero(spec);

  spec.freq = 44100;
  spec.format = AUDIO_S16SYS;
  spec.channels = 1;
  spec.samples = 2048;
  spec.callback = audioCallback;

  audioDevice = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
  
  if (audioDevice == 0) {
    fprintf(stderr, "SDL audio error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_PauseAudioDevice(audioDevice, 0);
  audioEnabled = 1;

  window = SDL_CreateWindow(
    "CHIP-8 Emulator",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    640, 320,   // scaled up
    0
  );
  
  srand(time(NULL));

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  uint32_t lastTime = SDL_GetTicks();
  float dt = 0;

  while (PC != romSize + 0x200 && RUNNING) {
    uint32_t now = SDL_GetTicks();
    dt += (now - lastTime);
    lastTime = now;

    while (dt >= 16) {
      if (delayTimer > 0) delayTimer--;
      if (soundTimer > 0) soundTimer--;

      dt -= 16;
    }

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: RUNNING = 0; break;
        case SDL_KEYDOWN: {
          SDL_Keycode key = event.key.keysym.sym;

          for (int i = 0; i < 16; i++) {
            if (key == SDL_GetKeyFromName((char[]){keymap[i], '\0'})) {
              KEYPAD[i] = 1;

              if (waitingForKey) {
                waitingForKey = 0;
                V[waitingRegister] = i;
              }
            } 
          }
          break;
        }
        case SDL_KEYUP: {
          SDL_Keycode key = event.key.keysym.sym;

          for (int i = 0; i < 16; i++) {
            if (key == SDL_GetKeyFromName((char[]){keymap[i], '\0'})) {
              KEYPAD[i] = 0;
            }
          }
          break;
        }
      }
    }

    if (waitingForKey) {
      render();
      continue;
    }

    uint16_t preFetchPC = PC;
    uint16_t OPCode = fetchOpcode();

    printf("PC: 0x%03x | OPCODE: 0x%04x | Delay Timer: 0x%02x | Sound Timer: 0x%02x\n", preFetchPC, OPCode, delayTimer, soundTimer);

    if (decodeOpcode(OPCode)) handledInstructions++;

    render();
  } 
  
  SDL_CloseAudioDevice(audioDevice);

  return 0;
}
