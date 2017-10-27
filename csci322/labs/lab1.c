//Garrett Gregory
//Lab1, csci322, wwu

#include <sys/time.h>
#include <stdio.h>

// get the computer's wall time
double get_wallTime(){
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return (double) (tp.tv_sec + tp.tv_usec/1000000.0);
   }

// a dummy method to prevent compiler optimization
void dummy(double aM[], double aN[], double aP[], double aQ[]){
   }

// perform arithmetic "stuff"
double benchmark(int N, int R){

   // initialize variables
   double array1[N], array2[N], array3[N], array4[N];
   int i, j, k;

   // initialize arrays
   for (i=0; i<N; i++){
      array1[i] = 0.0; array2[i] = 1.0; array3[i] = 2.0; array4[i] = 3.0;
   }

   // perform calculations
   double startTime = get_wallTime();
   for (j=1; j<R; j++){
      for (k=1; k<N; k++){
         array1[k] = array2[k] + array3[k] * array4[k];
         dummy(array1, array2, array3, array4);
      }
   }

   // calculate MFLOPS
   double endTime = get_wallTime();
   return (R * N * 2.0/((endTime-startTime) * 1000000));
}

int main(){
   double mflops = benchmark(10000,10000);
   printf("MFLOPS: %f\n", mflops);
   return 0;
}

