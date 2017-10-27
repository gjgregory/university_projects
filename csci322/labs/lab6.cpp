// Filip Jagodzinski
// CSCI322
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <ctime>
#include <future>
#include <thread>

using namespace std;

// get the wall time
double get_wallTime() {
  struct timeval tp;
  gettimeofday(&tp,NULL);
  return (double) (tp.tv_sec + tp.tv_usec/1000000.0);
}

// A vanilla random number generator
double genRandNum(double min, double max){
  return min + (rand() / (RAND_MAX / (max - min)));
}

// A ridiculous calculation
/*double ridiculousCalc(double* array1, double* array2,
		      double* array3, int dimension){

  double* resultArray1 = new double[dimension];
  double* resultArray2 = new double[dimension];
  double* resultArray3 = new double[dimension];
  double* resultArray4 = new double[dimension];
  double* resultArray5 = new double[dimension];
  double* outputArray = new double[dimension];
  double factor = 0.000000001;

  for (int i=0; i<dimension; i++){
    resultArray1[i] = array1[i] / array2[i] * array3[i] * -1.456;
    resultArray2[i] = resultArray1[i] / array3[i] * array3[i];
    resultArray3[i] = array3[i] * array2[i] + array1[i]
      * array2[i]/(array2[i] * -0002.7897);
    resultArray4[i] = resultArray3[i] * array2[i] / array1[i];
    resultArray5[i] = resultArray4[i] * array2[i] / array1[i]
      * sqrt(resultArray4[i] * 0.0000000023);
    outputArray[i] = resultArray1[i]*factor + resultArray2[i]*factor
      + resultArray3[i]*factor + resultArray4[i]*factor
      + resultArray5[i]*factor;
  }

  double output = 0.0;
  for (int i=0; i<dimension; i++){
    output += dimension;
  }

  return output;
}*/

/*double ridiculousCalcOptimized(double* array1, double* array2, double* array3,
        int dimension){

  double* resultArray1 = new double[dimension];
  double* resultArray2 = new double[dimension];
  double* resultArray3 = new double[dimension];
  double* resultArray4 = new double[dimension];
  double* resultArray5 = new double[dimension];
  double* outputArray = new double[dimension];
  double factor = 0.000000001;
  double output = 0.0;


  //loop fission performed
  for (int i=0; i<dimension; i++){
    resultArray1[i] = array1[i] / array2[i] * array3[i] * -1.456;
    //swapped lines 2 and 3
    resultArray3[i] = array3[i] * array2[i] + array1[i]
          * array2[i]/(array2[i] * -0002.7897);
    resultArray2[i] = resultArray1[i] / array3[i] * array3[i];
    resultArray4[i] = resultArray3[i] * array2[i] / array1[i];
    resultArray5[i] = resultArray4[i] * array2[i] / array1[i]
          * sqrt(resultArray4[i] * 0.0000000023);
    //output incrementing moved before final calculation (loop fusion)
    output += dimension;
    outputArray[i] = factor * (resultArray1[i] + resultArray2[i]
          + resultArray3[i] + resultArray4[i]
          + resultArray5[i]);

  }

  //loop peeling performed (didn't work for some reason)
  //double output = dimension * dimension;

  return output;
}*/

// A ridiculous threaded calculation
double ridiculousCalc(double* array1, double* array2,
		      double* array3, int dimLower, int dimUpper, int dimension){

  double* resultArray1 = new double[dimUpper];
  double* resultArray2 = new double[dimUpper];
  double* resultArray3 = new double[dimUpper];
  double* resultArray4 = new double[dimUpper];
  double* resultArray5 = new double[dimUpper];
  double* outputArray = new double[dimUpper];
  double factor = 0.000000001;

  for (int i=dimLower; i<dimUpper; i++){
    resultArray1[i] = array1[i] / array2[i] * array3[i] * -1.456;
    resultArray2[i] = resultArray1[i] / array3[i] * array3[i];
    resultArray3[i] = array3[i] * array2[i] + array1[i]
      * array2[i]/(array2[i] * -0002.7897);
    resultArray4[i] = resultArray3[i] * array2[i] / array1[i];
    resultArray5[i] = resultArray4[i] * array2[i] / array1[i]
      * sqrt(resultArray4[i] * 0.0000000023);
    outputArray[i] = resultArray1[i]*factor + resultArray2[i]*factor
      + resultArray3[i]*factor + resultArray4[i]*factor
      + resultArray5[i]*factor;
  }

  double output = 0.0;
  for (int i=dimLower; i<dimUpper; i++){
    output += dimension;
  }

  return output;
}

// An optimized threaded calculation
double ridiculousCalcOptimized(double* array1, double* array2,
		      double* array3, int dimLower, int dimUpper, int dimension){

  double* resultArray1 = new double[dimUpper];
  double* resultArray2 = new double[dimUpper];
  double* resultArray3 = new double[dimUpper];
  double* resultArray4 = new double[dimUpper];
  double* resultArray5 = new double[dimUpper];
  double* outputArray = new double[dimUpper];
  double factor = 0.000000001;
  double output = 0.0;

  //loop fission performed
  for (int i=dimLower; i<dimUpper; i++){
    resultArray1[i] = array1[i] / array2[i] * array3[i] * -1.456;
    //swapped lines 2 and 3
    resultArray3[i] = array3[i] * array2[i] + array1[i]
          * array2[i]/(array2[i] * -0002.7897);
    resultArray2[i] = resultArray1[i] / array3[i] * array3[i];
    resultArray4[i] = resultArray3[i] * array2[i] / array1[i];
    resultArray5[i] = resultArray4[i] * array2[i] / array1[i]
          * sqrt(resultArray4[i] * 0.0000000023);
    //output incrementing moved before final calculation (loop fusion)
    output += dimension;
    outputArray[i] = factor * (resultArray1[i] + resultArray2[i]
          + resultArray3[i] + resultArray4[i]
          + resultArray5[i]);
  }

  //loop peeling performed (didn't work for some reason)
  //output = dimension * (dimension/8);

  return output;
}

// main routine
int main(){

  // dimension of the array
  int dim = 10000000;

  // number of threads
  int threads = std::thread::hardware_concurrency();

  // seed the random number generator
  srand( time(NULL));

  // create the arrays and populate them
  // time the entire event
  double createStart = get_wallTime();
  static double *array1 = new double[dim];
  static double *array2 = new double[dim];
  static double *array3 = new double[dim];
  for (int i=0; i<dim; i++){
    array1[i] = genRandNum(0, 1000000);
    array2[i] = genRandNum(0, 1000000);
    array3[i] = genRandNum(0, 1000000);
  }
  double createEnd = get_wallTime();
  cout << "Time needed to create arrays                  : "
       << createEnd - createStart << endl;
  cout << "========================================================"
       << endl;

  // perform non-optimized calculations
  double ridiculousStart = get_wallTime();
  double output = ridiculousCalc(array1, array2, array3, 0, dim, dim);
  double ridiculousEnd = get_wallTime();
  cout << "Time needed to complete ridiculous calculation: "
       << ridiculousEnd - ridiculousStart << endl;
  cout << "Ridiculous calculation output                 : "
       << output << endl;
  cout << "========================================================"
       << endl;

  // perform optimized calculations
  double optimizedStart = get_wallTime();
  double output2 = ridiculousCalcOptimized(array1, array2, array3, 0, dim, dim);
  double optimizedEnd = get_wallTime();
  cout << "Time needed to complete optimized calculation: "
       << optimizedEnd - optimizedStart << endl;
  cout << "Optimized calculation output                  : "
       << output2 << endl;
  cout << "========================================================"
       << endl;

  // perform ridiculous threaded calculations
  double ridicThreadedStart = get_wallTime();
  std::vector<std::future<double>> nonOptimizedThreadedPool;
  // create threads
  for (int i=0; i < threads; i++){
    nonOptimizedThreadedPool.push_back(std::async(std::launch::async, ridiculousCalc,
          array1, array2, array3, (dim/threads)*i, (dim/threads)*(i+1), dim));
  }
  double output3 = 0;
  // add up outputs
  while (nonOptimizedThreadedPool.size() > 0){
    output3 += nonOptimizedThreadedPool.back().get();
    nonOptimizedThreadedPool.pop_back();
  }
  double ridicThreadedEnd = get_wallTime();
  cout << "Number of threads available                   : " << threads << endl;
  cout << "Time for ridiculous threaded calculation      : "
       << ridicThreadedEnd - ridicThreadedStart << endl;
  cout << "Ridiculous threaded output                    : "
       << output3 << endl;
  cout << "========================================================"
       << endl;

  // perform optimized threaded calculations
  double optimThreadedStart = get_wallTime();
  std::vector<std::future<double>> optimizedThreadedPool;
  // create threads
  for (int i=0; i < threads; i++){
    optimizedThreadedPool.push_back(std::async(std::launch::async, ridiculousCalcOptimized,
          array1, array2, array3, (dim/threads)*i, (dim/threads)*(i+1), dim));
  }
  double output4 = 0;
  // add up outputs
  while (optimizedThreadedPool.size() > 0){
    output4 += optimizedThreadedPool.back().get();
    optimizedThreadedPool.pop_back();
  }
  double optimThreadedEnd = get_wallTime();
  cout << "Time for optimized threaded calculation       : "
      << optimThreadedEnd - optimThreadedStart << endl;
  cout << "Ridiculous threaded output                    : "
      << output4 << endl;
  cout << "========================================================"
      << endl;

}
