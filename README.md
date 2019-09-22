# Brainf-Compiler

Yet another compiler of the esoteric language, created as a refresher.

Compiles from BF to CPP source code. Applies some optimizations to make the output run faster.
Namely:

* Sequential adds / subtracts and shifts are combined
* Clears, e.g. `[-]` or `[+]`and Multiplies are no longer loops.

Test cases based on:
* https://esolangs.org/wiki/Brainfuck
* http://www.hevanet.com/cristofd/brainfuck/
