 
// Garrett Gregory 
// CSCI 322, WWU; Threading Lab

// include and namespace declaratio
#include <iostream>
using namespace std;

// a "task"
void task (string msg){
    int d;
    // print out a message
    for (int a=1; a<1000; a=a+1) {
        for (int b=1; b<1000; b=b+1) {
            for (int c=1; c<1000; c=c+1) {
                d = a*b*c*d;
            }
        }
    }
    cout<<"all done! ";
}

//main routine
int main(){
    
    //invoke three threads
    task("MONKEYS");
    task("LOVE");
    task("BANANAS");
    task("HATE");
    task("FROGS");
    
}