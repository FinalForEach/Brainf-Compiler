# Brainf-Compiler

Yet another compiler of the esoteric language, created as a refresher.

Compiles from BF to C++ source code. Applies some optimizations to make the output run faster.
Namely:

* Sequential adds / subtracts and shifts are combined
* Adds and shifts are postponed until necessary, as to combine as many as possible
* Clears, e.g. `[-]` or `[+]`and Multiplies are no longer loops.
* Multiplies with known factors are reduced to adds
* Dead-code elimination
* Balanced shifts are pre-calculated into offsets, e.g. >>>+<<< becomes data[dataIndex+3]+=1;
* Some loops under certain conditions are reduced to if blocks
* The data 'pointer' is marked as a register, due to its very frequent usage.
* Values are stored in constants when used later.

The output code is produced with two debug functions and macros to help debug your BF programs, by printing out what is stored in the data at the current moment, and can store the data into a BF file, so that it can be reloaded or analyzed separately.

Test cases based on:
* https://esolangs.org/wiki/Brainfuck
* http://www.hevanet.com/cristofd/brainfuck/
* http://esoteric.sange.fi/brainfuck/utils/mandelbrot/mandelbrot.b


Additional recommended reading:
* https://www.nayuki.io/page/optimizing-brainfuck-compiler
* http://calmerthanyouare.org/2015/01/07/optimizing-brainfuck.html
* https://mearie.org/projects/esotope/bfc/index.en
