# 🍫 Choco8

A lightweight and efficient CHIP-8 emulator written in C using SDL2.

Choco8 emulates the classic CHIP-8 virtual machine, allowing you to run legendary games and programs from the 1970s. It features a clean implementation of the CPU, memory, timers, and a square-wave audio engine.

---

## Features

- **Core CPU:** Complete implementation of the CHIP-8 instruction set (35 opcodes).
- **Graphics:** 64x32 monochrome display, scaled 10x for modern screens using SDL2.
- **Audio:** Integrated sound timer with a 440Hz square-wave beep.
- **Input:** Standard 16-key hexadecimal keypad support.
- **Compatibility:** Tested with various test ROMs and classic games like Tetris.
- **Cross-Platform:** Built with C and SDL2 for portability.

---

## Installation & Build

### Prerequisites

- A C compiler (GCC recommended)
- **SDL2** development libraries

On Debian/Ubuntu-based systems:
```bash
sudo apt-get install libsdl2-dev
```

### Building

Simply run `make` in the project root:

```bash
make
```

This will generate the `choco8` executable.

### Installation

To install the binary to your system (default prefix is `/usr`):

```bash
sudo make install
```

To uninstall:

```bash
sudo make uninstall
```

---

## Usage

Run the emulator by passing the path to a CHIP-8 ROM file:

```bash
./choco8 ROMs/Tetris.ch8
```

### Controls (Keymap)

The original CHIP-8 used a 4x4 hexadecimal keypad. Choco8 maps these keys to your modern keyboard as follows:

| CHIP-8 Key | Keyboard Key |
| :---: | :---: |
| **1** | `1` |
| **2** | `2` |
| **3** | `3` |
| **C** | `4` |
| **4** | `Q` |
| **5** | `W` |
| **6** | `E` |
| **D** | `R` |
| **7** | `A` |
| **8** | `S` |
| **9** | `D` |
| **E** | `F` |
| **A** | `Z` |
| **0** | `X` |
| **B** | `C` |
| **F** | `V` |

---

## Project Structure

- `src/`: Source code (`choco8.c`, `io/readfile.c`).
- `include/`: Header files.
- `ROMs/`: A collection of public domain CHIP-8 ROMs and test suites.
- `makefile`: Build configuration.

---

## 🕹️ Included ROMs

The `ROMs/` directory contains several programs to get you started:

- `1-chip8-logo.ch8`: Displays the CHIP-8 logo.
- `3-corax+.ch8`: Opcode test suite.
- `4-flags.ch8`: Graphics and flag test.
- `5-quirks.ch8`: Tests for various CHIP-8 quirks.
- `IBM-logo.ch8`: The classic "Hello World" for CHIP-8.
- `Tetris.ch8`: A fully playable version of Tetris.

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
