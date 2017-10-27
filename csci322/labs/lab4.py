# threadsWSemaphore.py
# Garrett Gregory

# import statement
from pythonThreading import *

#declare function_1
def function_1(val_1,val_2,val_3):
    sem1.wait()
    value = val_3
    for x in range(1,val_1):
        for y in range(1,val_2):
            value = value + x + y
    print "function 1 says :", value
    sem1.signal()

 #declare function_2
def function_2(val_1,val_2,val_3):
    sem1.wait()
    value = val_3
    for x in range(1,val_1):
        for y in range(1,val_2):
            value = value + x + y
    print "function 2 says :", value
    sem1.signal()


#initiate semaphore
sem1 = Semaphore(1)

#issue a thread that invokes function_1
Thread(function_1, 1000, 1000, 3.6)

#issue a thread that invokes function_2
Thread(function_2, 100, 100, 3.6)
