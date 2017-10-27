# Garrett Gregory
# Threaded Radix Sort
# WWU - CSCI 322
# 2 December 2015

import threading, math, time, random
from multiprocessing.pool import ThreadPool

# helper function sorts part of the list in a single thread
def cntgSort(numList, digitList):
    counts = [0,0,0,0,0,0,0,0,0,0]
    # get counts for each digit
    for i in range(0, len(numList)):
        counts[digitList[i]] += 1
    # convert to indices
    idx = 0
    for i in range(0, len(counts)):
        if counts[i] > 0:
            temp = counts[i]
            counts[i] = idx
            idx += temp
    # declare and allocate space for output list
    outList = list()
    for i in range(0, len(numList)):
        outList.append(0)
    # arrange the elements in order
    for i in range(0, len(numList)):
        outList[counts[digitList[i]]] = numList[i]
        counts[digitList[i]] += 1
    return outList

# counting sort function that splits the work into 5 threads
def threadedCntgSort(numList, digitList):
    list1a, list1b, list2a, list2b, list3a, list3b, list4a, list4b, list5a, list5b = [],[],[],[],[],[],[],[],[],[]
    # prep lists for individual threaded sorting
    for i in range(0, len(numList)):
        if digitList[i] == 0 or digitList[i] == 1:
            list1a.append(numList[i])
            list1b.append(digitList[i])
        if digitList[i] == 2 or digitList[i] == 3:
            list2a.append(numList[i])
            list2b.append(digitList[i])
        if digitList[i] == 4 or digitList[i] == 5:
            list3a.append(numList[i])
            list3b.append(digitList[i])
        if digitList[i] == 6 or digitList[i] == 7:
            list4a.append(numList[i])
            list4b.append(digitList[i])
        if digitList[i] == 8 or digitList[i] == 9:
            list5a.append(numList[i])
            list5b.append(digitList[i])
    # perform threading
    pool = ThreadPool(processes=5)
    thread1 = pool.apply_async(cntgSort, (list1a, list1b,))
    thread2 = pool.apply_async(cntgSort, (list2a, list2b,))
    thread3 = pool.apply_async(cntgSort, (list3a, list3b,))
    thread4 = pool.apply_async(cntgSort, (list4a, list4b,))
    thread5 = pool.apply_async(cntgSort, (list5a, list5b,))
    list1 = thread1.get()
    list2 = thread2.get()
    list3 = thread3.get()
    list4 = thread4.get()
    list5 = thread5.get()
    #combine lists into output
    output = []
    for i in list1:
        output.append(i)
    for i in list2:
        output.append(i)
    for i in list3:
        output.append(i)
    for i in list4:
        output.append(i)
    for i in list5:
        output.append(i)
    return output


# multi-threaded radix sort function
def threadedRadixSort(numList):
    highNum = max(numList)
    # using base-10
    power = math.floor(math.log(highNum, 10)) + 1
    i = power-1
    while i >= 0:
        newList = []
        for j in numList:
            #calculate the digit to sort
            j = int((j % (10 ** (power - i))) // (10**(power - i - 1)))
            newList.append(j)
        numList = threadedCntgSort(numList, newList)
        i -= 1
    return numList

# singly-threaded radix sort function
def normalRadixSort(numList):
    highNum = max(numList)
    # using base-10
    power = math.floor(math.log(highNum, 10)) + 1
    i = power-1
    while i >= 0:
        newList = []
        for j in numList:
            #calculate the digit to sort
            j = int((j % (10 ** (power - i))) // (10**(power - i - 1)))
            newList.append(j)
	# force single thread
    	pool = ThreadPool(processes=1)
    	thread1 = pool.apply_async(cntgSort, (numList, newList))
	numList = thread1.get()
        i -= 1
    return numList


#perform benchmark tests
testList = list()
for i in range(1001):
    testList.append(int(random.randrange(0, 2000001)))
print "list built..."

start = time.time()
megalist = normalRadixSort(testList)
end = time.time()
print "time to complete singly-threaded version:", str(end-start), "seconds"

start = time.time()
megalist = threadedRadixSort(testList)
end = time.time()
print "time to complete multi-threaded version:", str(end-start), "seconds"
