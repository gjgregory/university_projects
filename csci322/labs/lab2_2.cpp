 
// Garrett Gregory 
// CSCI 322, WWU; Threading Lab

// include and namespace declaratio
#include <iostream>
#include <thread>
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
    thread t1(task, "MONKEYS");
    thread t2(task, "LOVE");
    thread t3(task, "BANANAS");
    thread t4(task, "HATE");
    thread t5(task, "FROGS");
    
    // force the threads to 'finish'
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    
}