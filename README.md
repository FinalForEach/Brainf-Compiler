# Brainf-Compiler

Yet another compiler of the esoteric language, created as a refresher.

Compiles from BF to C++ source code. Applies some optimizations to make the output run faster.
Namely:

* Sequential adds / subtracts and shifts are combined
* Adds and shifts are postponed until necessary, as to combine as many as possible
* Clears, e.g. `[-]` or `[+]`and Multiplies are no longer loops.

Test cases based on:
* https://esolangs.org/wiki/Brainfuck
* http://www.hevanet.com/cristofd/brainfuck/
* http://esoteric.sange.fi/brainfuck/utils/mandelbrot/mandelbrot.b
