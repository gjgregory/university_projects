#!/usr/bin/env python

#   Garrett Gregory
#   CSCI 497F
#   Assignment 3 - Hash Functions
#
#   This is a simulation of Keccak-p. It reads a file, determines the length of
#   the file, and outputs an appropriately sized Keccak-p transformation to a
#   new output file. The steps in building the state matrix, the round functions,
#   and building the output string are all based off of the slides provided to our
#   CSCI 497F class.
#
#
#           USAGE:  $ python2 my_keccakp.py <input_filename> <# of rounds>
#                   $ hexdump -C p-hash.sha3

import sys
import os
import binascii
import math
from cStringIO import StringIO


#round constant function
def rc(t):
    if t%255 == 0:
        return 1

    R = [1,0,0,0,0,0,0,0]
    for i in range(1, t%255 + 1):
        Rp = [0]
        Rp.extend(R)

        Rp[0] = Rp[0] ^ Rp[8]
        Rp[4] = Rp[4] ^ Rp[8]
        Rp[5] = Rp[5] ^ Rp[8]
        Rp[6] = Rp[6] ^ Rp[8]
        R = Rp[0:8]

    return R[0]

def stepmapTheta(A, w):
    C = [[0 for z in range(w)] for x in range(5)]
    D = [[0 for z in range(w)] for x in range(5)]
    Ap = [[[0 for z in range(w)] for y in range(5)] for x in range(5)]

    for x in range(5):
        for z in range(w):
            C[x][z] = A[x][0][z] ^ A[x][1][z] ^ A[x][2][z] ^ A[x][3][z] ^ A[x][4][z]

    for x in range(5):
        for z in range(w):
            D[x][z] = C[(x-1)%5][z] ^ C[(x+1)%5][(z-1)%w]

    for x in range(5):
        for y in range(5):
            for z in range(w):
                Ap[x][y][z] = A[x][y][z] ^ D[x][z]

    return Ap


def stepmapRho(A, w):
    Ap = [[[0 for z in range(w)] for y in range(5)] for x in range(5)]
    x, y = 1, 0
    for z in range(w):
        Ap[0][0][z] = A[0][0][z]

    for t in range(24):
        for z in range(w):
            Ap[x][y][z] = A[x][y][(z-(t+1)*(t+2)/2)%w]
        x, y = y, (2*x + 3*y)%5

    return Ap

def stepmapPi(A, w):
    Ap = [[[0 for z in range(w)] for y in range(5)] for x in range(5)]

    for x in range(5):
        for y in range(5):
            for z in range(w):
                Ap[x][y][z] = A[(x+3*y)%5][x][z]

    return Ap

def stepmapChi(A, w):
    Ap = [[[0 for z in range(w)] for y in range(5)] for x in range(5)]

    for x in range(5):
        for y in range(5):
            for z in range(w):
                Ap[x][y][z] = A[x][y][z] ^ ((A[(x+1)%5][y][z] ^ 1) & A[(x+2)%5][y][z])

    return Ap

def stepmapIota(A, w, ir, l):
    Ap = [[[0 for z in range(w)] for y in range(5)] for x in range(5)]

    for x in range(5):
        for y in range(5):
            for z in range(w):
                Ap[x][y][z] = A[x][y][z]

    RC = [0 for z in range(w)]

    for j in range(l+1):
        RC[2**j - 1] = rc(j + 7*ir)

    for z in range(w):
        Ap[0][0][z] = Ap[0][0][z] ^ RC[z]

    return Ap


def main():

    if len(sys.argv) != 3:
        print "\nProper Usage: python my_keccakp.py <input_filename>\n"
        sys.exit(0)

    filename = sys.argv[1]

    # get validity of # of rounds
    try:
        nr = int(sys.argv[2])
    except:
        print "\n# of rounds must be a positive integer.\n"
        sys.exit(0)

    if nr < 1:
        print "\n# of rounds must be a positive integer.\n"
        sys.exit(0)

    #read input file byte-by-byte and get size in bits
    try:
        infile = open(filename, "rb")
        filesize = os.stat(filename).st_size * 8
    except:
        print "\ncannot open input file -", filename, "\n"
        sys.exit(0)

    # calculate b value
    if filesize >= 1600:
        bslength = 1600
    elif filesize >= 800:
        bslength = 800
    elif filesize >= 400:
        bslength = 400
    elif filesize >= 200:
        bslength = 200
    elif filesize >= 100:
        bslength = 100
    elif filesize >= 50:
        bslength = 50
    elif filesize >= 25:
        bslength = 25
    else:
        print "\nfile is invalid input\n"
        sys.exit(0)

    # calculate w and l values
    w = bslength/25
    l = int(math.log(bslength/25,2))

    # initialize an "empty" state matrix
    state_array = [[[0 for i in range(w)] for j in range(5)] for k in range(5)]

    # build the binary string
    binary_string = ""
    for i in range(filesize/8):
        raw = infile.read(1)
        binary_string += format(int(binascii.hexlify(raw), 16), '08b')

    infile.close()

    # populate the state array in order of indices: z, x, y
    for x in range(5):
        for y in range(5):
            for z in range(w):
                state_array[x][y][z] = int(binary_string[w*(5*y + x) + z])



    # MAIN LOOP:
    for ir in range(12 + 2*l - nr, 12 + 2*l):
        # ------------------ STEP MAPPING THETA ----------------------- #
        state_array = stepmapTheta(state_array, w)
        # ------------------ STEP MAPPING RHO ----------------------- #
        state_array = stepmapRho(state_array, w)
        # ------------------ STEP MAPPING PI ----------------------- #
        state_array = stepmapPi(state_array, w)
        # ------------------ STEP MAPPING CHI ----------------------- #
        state_array = stepmapChi(state_array, w)
        # ------------------ STEP MAPPING IOTA ----------------------- #
        state_array = stepmapIota(state_array, w, ir, l)



    # get string representation of each lane
    lanes = [["" for j in range(5)] for i in range(5)]
    for i in range(5):
        for j in range(5):
            for k in range(w):
                lanes[i][j] += str(state_array[i][j][(w-1)-k]) # get indices from back of lane to front

    # get string representation of each plane
    planes = ["" for j in range(5)]
    for j in range(5):
        planes[j] = lanes[0][j] + lanes[1][j] + lanes[2][j] + lanes[3][j] + lanes[4][j]

    # get string representation of entire state
    out_binary_str = planes[0] + planes[1] + planes[2] + planes[3] + planes[4]

    # write out
    out_binary = StringIO(out_binary_str)
    outfilename = 'p-hash.sha3'
    outfile = open(outfilename, 'wb')
    while True:
        out_byte = out_binary.read(8)
        if not out_byte:
            break
        if len(out_byte) < 8:
            out_byte += '0'*(8-len(out_byte))
        c = chr(int(out_byte, 2))
        outfile.write(c)

    print "Results stored in file:", outfilename


main()
