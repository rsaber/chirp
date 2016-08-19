# chirp
## written by Riyasat Saber
a chip8 emulator written in C. Uses SDL.

compile with (for Mac)

```
gcc -o chirp chirp.c chip8.c -F/Library/Frameworks -framework SDL2

```
compile with (for Linux)

```
gcc chirp.c chip8.c -w -lsdl2 -o chirp
```

There are a couple sample games to play around with.