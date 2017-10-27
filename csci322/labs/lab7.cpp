// Garrett Gregory
// CSCI322
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <ctime>
#include <future>
#include <thread>
#include <unistd.h>

using namespace std;

// get the wall time
double get_wallTime() {
  struct timeval tp;
  gettimeofday(&tp,NULL);
  return (double) (tp.tv_sec + tp.tv_usec/1000000.0);
}

// a vanilla random num generator mod 10
int genRandNum(int min, int max){
  return min + (rand() / (RAND_MAX / (max - min))) % 10;
}

// a global variable
int sumConcurrentGlobal = 0;

// bakery algorithm variables
bool entering[6];
int num[6];

// calcualte sum using local sum variable
int sumArrayContentsLocalSum(int** array, int dimension){

  int output = 0;
  for (int i=0; i<dimension; i++){
    for (int j=0; j<dimension; j++){
      output += array[i][j];
    }
  }
  return output;
}

// calcualte sum using global sum variable (employs the bakery algorithm)
void sumArrayContentsGlobalSum(int** array, int dimension, int x){
   int maxInt=0;
   //lock
   entering[x] = true;
   for (int m=0; m<6; m++){
      if (num[m] > maxInt)
         maxInt = num[m];
   }
   num[x] = maxInt + 1;
   entering[x] = false;
   for (int n=0; n<6; n++){
      while (entering[n]) {}
      while (num[n]!=0 && num[n] < num[x]) {}
   }

   //critical section
   for (int i=0; i<dimension; i++){
      for (int j=0; j<dimension; j++){
       sumConcurrentGlobal += array[i][j];
      }
   }

   //unlock
   num[x] = 0;
}

// main routine
int main(){

   // initialize bakery algorithm variables
   for (int i=0; i<6; i++){
      entering[i] = false;
      num[i] = 0;
   }

  // dimension of the array
  int dim = 5000;

  // number of sums to perform
  int numSums = 6;

  // seed the random number generator
  srand( time(NULL));

  // create the 2D array and populate it
  // time how long to create array
  double createStart = get_wallTime();
  int** a2DArray = new int*[dim];
  for (int i=0; i<dim; i++){
    a2DArray[i] = new int[dim];
    for (int j=0; j<dim; j++){
      a2DArray[i][j] = genRandNum(0,100);
    }
  }
  double createEnd = get_wallTime();

  ///////////////////////////////////////
  // Sequential
  ///////////////////////////////////////

  // sum the entires of array times 6
  int sumSequential = 0;
  double sumStart = get_wallTime();
  for (int m=0; m<6; m++){
    for (int i=0; i<dim; i++){
      for (int j=0; j<dim; j++){
	sumSequential += a2DArray[i][j];
      }
    }
  }
  double sumEnd = get_wallTime();


  ///////////////////////////////////////
  // Concurrent, using a local Sum var
  ///////////////////////////////////////

  double concurrentLocalStart = get_wallTime();
  std::vector<std::future<int>> thread_poolLocal;
  for (int i = 0; i < numSums; ++i) {
    thread_poolLocal.push_back( std::async(launch::async,
					   sumArrayContentsLocalSum,
					   a2DArray, dim));
  }

  int sumConcurrentLocal = 0;
  for (auto &threadLocalOutput : thread_poolLocal){
    sumConcurrentLocal += threadLocalOutput.get();
  }
  double concurrentLocalEnd = get_wallTime();

  ///////////////////////////////////////
  // Concurrent, using a global Sum var
  ///////////////////////////////////////

  double concurrentGlobalStart = get_wallTime();
  std::vector<std::future<void>> thread_poolGlobal;
  for (int i = 0; i < numSums; ++i) {
    thread_poolGlobal.push_back( std::async(launch::async,
					    sumArrayContentsGlobalSum,
					    a2DArray, dim, i));
  }
  //added this so the threads would actually finish
  for (int i = 0; i < numSums; ++i) {
     thread_poolGlobal.back().get();
     thread_poolGlobal.pop_back();
  }
  double concurrentGlobalEnd = get_wallTime();

  // print run-times
  cout << "Time needed to create array                 : " << createEnd - createStart << endl;
  cout << "Time needed to sum * 6 array's contents     : " << sumEnd - sumStart << endl;
  cout << "Time needed for concurrent local sum        : " << concurrentLocalEnd - concurrentLocalStart << endl;
  cout << "Time needed for concurrent global sum       : " << concurrentGlobalEnd - concurrentGlobalStart << endl;

  // print sums
  cout << "The sum of the 2D array                     : " << sumSequential << endl;
  cout << "The sum of the 2D array conc, local sum     : " << sumConcurrentLocal << endl;
  cout << "The sum of the 2D array conc, global sum    : " << sumConcurrentGlobal << endl;

 }
