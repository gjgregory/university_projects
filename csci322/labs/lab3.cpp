 
//Garrett Gregory
//CSCI 322, WWU; Lab 3

#include <iostream>
#include <future>
#include <sys/time.h>
#include <stdio.h>
using namespace std;


// a function, calcSum, that takes as input a 3D array, and three
// other integer arguments, and which returns a long double
long double calcSum(double*** a3DArray, int dimLower, int dimUpper, int dim){
    
    long double aValue = 0;
    
    for (int a=dimLower; a<dimUpper; a=a+1){
        for (int b=0; b<dim; b=b+1) {
            for (int c=0; c<dim; c=c+1) {
                aValue += a3DArray[a][b][c];
            }
        }
    }
    
    return aValue;
    
}

//retrieve clock time (from lab 1)
double get_wallTime(){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (double) (tp.tv_sec + tp.tv_usec/1000000.0);
}

int main(){
    double wt1, wt2, wt3, wt4, wt5, wt6;
    int dim;
    long double sumAsync, sumSync;
    
    wt1 = get_wallTime();
    
    //array dimension
    dim = 1000;

    //create a 3d array up of pointers pointer pointers to doubles
    double ***my3DArray = new double**[dim];
    for (int i=0; i<dim; i++){
        my3DArray[i] = new double*[dim];
        for (int j=0; j<dim; j++){
            my3DArray[i][j] = new double[dim];
            for (int k=0; k<dim; k++){
                my3DArray[i][j][k] = 3.62893;
            }
        }
    }
    
    wt2 = get_wallTime();
    
    //output creation time
    cout<<"Time to create the 3d array: "<<(wt2-wt1)<< " seconds.\n";
    
    // START OF ASYNC RUN...
    wt3 = get_wallTime();
    
    auto thread0 = std::async(std::launch::async, calcSum, my3DArray, 0, 167, dim);
    auto thread1 = std::async(std::launch::async, calcSum, my3DArray, 167, 334, dim);
    auto thread2 = std::async(std::launch::async, calcSum, my3DArray, 334, 500, dim);
    auto thread3 = std::async(std::launch::async, calcSum, my3DArray, 500, 667, dim);
    auto thread4 = std::async(std::launch::async, calcSum, my3DArray, 667, 834, dim);
    auto thread5 = std::async(std::launch::async, calcSum, my3DArray, 834, dim, dim);
    sumAsync = thread0.get() + thread1.get() + thread2.get() + thread3.get() + thread4.get() + thread5.get();
    
    wt4 = get_wallTime();
    
    cout<<"Time for asynchronous summing: "<<(wt4-wt3)<<" seconds.\n";
    cout<<"Threaded sum = "<<sumAsync<<"\n";
    
    // START OF SYNC RUN...
    wt5 = get_wallTime();
    
    sumSync = calcSum(my3DArray, 0, dim, dim);
    
    wt6 = get_wallTime();
    
    cout<<"Time for synchronous summing: "<<(wt6-wt5)<<" seconds.\n";
    cout<<"Non-threaded sum = "<<sumSync<<"\n";
    
    
}


            