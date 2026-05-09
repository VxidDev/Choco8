# 🍫 Choco8

A simple CHIP-8 emulator written in C.

CHIP-8 is a minimal virtual machine from the 1970s designed to run simple games. This project emulates its CPU, memory, display, and input system.

---

## Features (MVP)

- ROM loading (`.ch8`)
- Opcode fetch & decode
- Core instruction set implemented
- 64×32 monochrome display
- IBM logo support (verified)


---

## Project Structure

```

.
├── include
│   ├── choco8.h
│   └── io
│       └── readfile.h
├── src
│   ├── choco8.c
│   └── io
│       └── readfile.c
├── ROMs
│   └── IBM-logo.ch8
├── makefile
├── LICENSE
└── README.md

````

---

## How it works

The emulator simulates a simple virtual machine with:

- 4KB memory
- 16 registers (V0–VF)
- Index register (I)
- Stack (16 levels)
- Program counter (PC)
- 64×32 display buffer

Each cycle:

1. Fetch opcode from memory
2. Decode and execute instruction
3. Render display

---

## Running ROMs

Place ROMs in the `ROMs/` directory and run:

```bash
./choco8 ROMs/filename.ch8
```

---

## Build

```bash
make
```

---

## Current status

This is an MVP implementation:

* Core CPU works
* Graphics rendering works
* IBM logo runs correctly

---

## Planned improvements

* Full opcode coverage verification
* Proper 60Hz timer synchronization
* SDL-based window rendering improvements
* Audio support (sound timer beep)
* Better compatibility across CHIP-8 variants

---

## Tested ROMs

* IBM Logo

---

## License

MIT License

