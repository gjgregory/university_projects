#!/usr/bin/env python

#   Dannon Dixon && Garrett Gregory
#   CSCI 497F
#   Assignment 4 - Asymmetric Algorithms - mycrypto.py
#
#   This implements functions used by Bob and Alice to do their
#   Elgamal encryption/decryption.


# converts an int to a bit string
def calculate_s(num):
    s = bin(num)
    s = s[2:]
    return s

# calculates z = m**s % n efficiently
def square_and_multiply(m, num, n):
    s = calculate_s(num)
    z = 1
    y = m
    for i in range(1, len(s)+1):
        if s[len(s)-i] == '1':
            z = z * y % n
        y = y * y % n
    return z

# gets the inverse of x, provided prime number n
def extended_euclidean(x, n):
    original_n = n

    auxilary = []
    quotients = []

    auxilary.append(0)
    auxilary.append(1)

    quotient = n // x
    quotients.append(quotient)
    remainder = n % x
    n = x
    x = remainder

    quotient = n // x
    quotients.append(quotient)
    remainder = n % x
    n = x
    x = remainder

    while remainder != 0:
        quotient = n // x
        remainder = n % x
        n = x
        x = remainder
        aux = (auxilary[len(auxilary) - 2] - auxilary[len(auxilary) - 1] * quotients[len(quotients) - 2]) % original_n
        auxilary.append(aux)
        quotients.append(quotient)

    aux = (auxilary[len(auxilary) - 2] - auxilary[len(auxilary) - 1] * quotients[len(quotients) - 2]) % original_n
    auxilary.append(aux)

    return auxilary[len(auxilary) - 1]

# def millerRabin(num, rounds):
#     if num == 2 or num == 3:
#         return True
#     elif num < 2 or num & 1 == 0:
#         return False
#
#     s = 0
#     d = num - 1
#
#     while d & 1 == 0:
#         s += 1
#         d = d >> 1
#
#     while rounds > 0:
#         rounds -= 1
#         a = random.randrange(2, num-1)
#         #x = a**d % num
#         x = pow(a, d, num)
#         if x == 1 or x == num - 1:
#             continue
#         for _ in range(1, s):
#             #x = x**2 % num
#             x = pow(x, 2, num)
#             if x == 1:
#                 return False
#             elif x == num - 1:
#                 break
#         if x != num - 1:
#             return False
#
#     return True

# def getGenerator(p, q):
#     while True:
#         g = random.randrange(2, p) # can't be 0, 1, or p-1
#         if pow(g, q, p) == 1:
#             return g
