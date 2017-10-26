#!/usr/bin/env python

#   Dannon Dixon && Garrett Gregory
#   CSCI 497F
#   Assignment 4 - Asymmetric Algorithms - alice.py
#
#   This script acts as the server in the message exchange with Bob and thus
#   should be ran first. The processes generate an AES key using the Elgamal
#   encryption algorithm, then use AES to encrypt and send messages to each
#   other which are provided by the user of each script.
#
#
#           USAGE:  $ python alice.py <port>


import socket
import sys
import random
from Crypto.Cipher import AES
from Crypto import Random
from mycrypto import square_and_multiply, calculate_s


def main():
    # get args
    if len(sys.argv) != 2:
        print "\nProper Usage: python alice.py <port>\n"
        sys.exit(0)

    try:
        port = int(sys.argv[1])
    except:
        print "Invalid port #"
        sys.exit(0)

    # open socket session
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error, message:
        print "Socket error: " + str(message[0]) + " - " + str(message[1])
        sys.exit(0)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('localhost', port))
    # wait for Bob to connect
    print "Waiting for Bob..."
    sock.listen(1)
    (conn, addr) = sock.accept()

    # receive Elgamal public key from Bob
    message = ''
    inchar = ''
    while inchar != '\r':
        inchar = conn.recv(1)
        message += inchar
    nums = message[:-1].split()
    p, alpha, beta = int(nums[0]), int(nums[1]), int(nums[2])

    k = random.randrange(1, p) # secret key
    x = random.randrange(1, p) # plaintext message

    # generate Elgamal ciphertext y1 and y2
    y1 = square_and_multiply(alpha, k, p)
    part1 = square_and_multiply(x, 1, p)
    part2 = square_and_multiply(beta, k, p)
    y2 = square_and_multiply(part1 * part2, 1, p)

    # send Elgamal ciphertext to Bob
    conn.send(str(y1) + ' ' + str(y2) + '\r')

    # convert x to a bit string for use as AES key (pad if necessary)
    aesKey = calculate_s(x)
    while len(aesKey) < 32:
        aesKey += " "

    # get plaintext from user, encrypt it with AES, and send it to Bob
    iv = Random.new().read(AES.block_size)
    cipher = AES.new(aesKey, AES.MODE_CFB, iv)
    message = raw_input("Enter message to encrypt: ")
    ciphertext = iv + cipher.encrypt(message)
    conn.send(ciphertext + '\r')

    # receive AES ciphertext from Bob
    ciphertext = ''
    inchar = ''
    while inchar != '\r':
        inchar = conn.recv(1)
        ciphertext += inchar

    # decrypt AES ciphertext and output plaintext to stdout
    iv = Random.new().read(AES.block_size)
    cipher = AES.new(aesKey, AES.MODE_CFB, iv)
    plaintext = cipher.decrypt(ciphertext[:-1])[16:]
    print 'Bob says, "' + plaintext + '"'


    sock.shutdown(1)


main()
