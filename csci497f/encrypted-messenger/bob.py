#!/usr/bin/env python

#   Dannon Dixon && Garrett Gregory
#   CSCI 497F
#   Assignment 4 - Asymmetric Algorithms - bob.py
#
#   This script acts as the client in the message exchange with Bob and thus
#   should be ran second. The processes generate an AES key using the Elgamal
#   encryption algorithm, then use AES to encrypt and send messages to each
#   other which are provided by the user of each script.
#
#
#           USAGE:  $ python bob.py localhost <port>


import socket
import sys
import random
from Crypto.Cipher import AES
from Crypto import Random
from mycrypto import *


def main():
    # get args
    if len(sys.argv) != 3:
        print "\nProper Usage: python bob.py <local address> <port>\n"
        sys.exit(0)

    host = sys.argv[1]
    try:
        port = int(sys.argv[2])
    except:
        print "Invalid port #"
        sys.exit(0)

    # open socket session
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error, message:
        print "Socket error: " + str(message[0]) + " - " + str(message[1])
        sys.exit(0)
    # get ip of Alice
    try:
        proc2_ip = socket.gethostbyname(host)
    except socket.error, message:
        print "Socket error: " + str(message[0]) + " - " + str(message[1])
        sys.exit(0)
    # connect to Alice
    try:
        sock.connect((proc2_ip, port))
    except socket.error, message:
        print "Socket error: " + str(message[0]) + " - " + str(message[1])
        print "Is Alice up and running yet?"
        sys.exit(0)
    print "Connected to Alice"

    p = 4094027087  # prime number such that p = 2q + 1
    q = 2047013543 # prime number used to generate p (not used here)
    alpha = 1659252438
    a = random.randrange(1, p) # secret key
    beta = pow(alpha, a, p)

    # send Elgamal public key to Alice
    message = str(p) + " " + str(alpha) + " " + str(beta) + '\r'
    sock.send(message)

    # receive Elgamal ciphertext from Alice
    message = ''
    inchar = ''
    while inchar != '\r':
        inchar = sock.recv(1)
        message += inchar
    nums = message[:-1].split()
    y1, y2 = int(nums[0]), int(nums[1])

    # get S from y1
    S = square_and_multiply(y1, a, p)
    # get S's inverse
    invS = extended_euclidean(S, p)
    # get x from y2 and invS
    x = square_and_multiply(y2 * invS, 1, p)

    # convert x to a bit string for use as AES key (pad if necessary)
    aesKey = calculate_s(x)
    while len(aesKey) < 32:
        aesKey += " "

    # receive AES ciphertext from Alice
    ciphertext = ''
    inchar = ''
    while inchar != '\r':
        inchar = sock.recv(1)
        ciphertext += inchar

    # decrypt AES ciphertext and output plaintext to stdout
    iv = Random.new().read(AES.block_size)
    cipher = AES.new(aesKey, AES.MODE_CFB, iv)
    plaintext = cipher.decrypt(ciphertext[:-1])[16:]
    print 'Alice says, "' + plaintext + '"'

    # get plaintext from user, encrypt it with AES, and send it to Alice
    iv = Random.new().read(AES.block_size)
    cipher = AES.new(aesKey, AES.MODE_CFB, iv)
    message = raw_input("How would you like to respond, Bob?: ")
    ciphertext = iv + cipher.encrypt(message)
    sock.send(ciphertext + '\r')


    sock.shutdown(1)


main()
