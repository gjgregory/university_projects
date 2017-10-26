## Keccak-p Transformer

This is a simulation of Keccak-p. It reads a file, determines the length of the file, and outputs an appropriately sized Keccak-p transformation to a new output file. The steps in building the state matrix, the round functions, and building the output string are all based off of the slides provided to our CSCI 497F class.

* my_keccakp.py

Usage:
```
$ python2 my_keccakp.py <input_filename> <# of rounds>
$ hexdump -C p-hash.sha3
```
