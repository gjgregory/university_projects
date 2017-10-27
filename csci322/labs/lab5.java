// UpToNoGood.java
// Garrett Gregory

// imports
import java.util.concurrent.*;
import java.util.*;

// class declaration
public class UpToNoGood {

  // main method declaration
  public static void main(String[] args) {
    int numThreads = 8;
    int chunkSize = 100000000/numThreads;

    System.out.println("Number of threads: " + numThreads);

    // DO NOT MODIFY -- START

    // number of accounts. MUST be 100 million
    int numAccounts = 100000000;

    // "simulate" an array of doubles ... accountBalances
    final double[] accountBalances = new double[numAccounts];
    for (int i=0; i<accountBalances.length; i++){
	    Random rand = new Random();
	    accountBalances[i] = (double) rand.nextInt(10000);
	  }

	  // DO NOT MODIFY -- END

	  // get system nanosecond time - start
	  double nonThreadStart = System.nanoTime();

	  // decrease each "account balance" by a tiny amount; x^{0.99}
	  for(int i=0; i<accountBalances.length; i++) {
	    accountBalances[i] = Math.pow(accountBalances[i], 0.999);
	  }

	  // get system nanosecond time - end
	  double nonThreadEnd = System.nanoTime();

	  // output the time needed to perform calculations
	  System.out.println("Time for account updates "
			   + ((nonThreadEnd - nonThreadStart)/1000000000.0));


    try{
      // initiate executor service for threads
      ExecutorService ex = Executors.newFixedThreadPool(numThreads);
      // start taking time for threaded run
      double threadStart = System.nanoTime();

      // for each chunk of data
      for (int i = 0; i < numThreads; i++) {
        final int indexStart = i * chunkSize;
    		final int indexEnd = (i + 1) * chunkSize;
        //execute thread
        ex.execute(new Runnable() {
          @Override
    			public void run() {
            for (int j = indexStart; j < indexEnd; j++){
              accountBalances[j] = Math.pow(accountBalances[j], 0.999);
            }
          }
        });
      }

      // shut down threads
      ex.shutdown();
      ex.awaitTermination(1, TimeUnit.MINUTES);

      // stop taking time for threaded run
      double threadEnd = System.nanoTime();
      // print time for threaded run
      System.out.println("Time for threaded account updates "
  			   + ((threadEnd - threadStart)/1000000000.0));

    }catch (InterruptedException e){

  	  // print out custom error messages
  	  System.out.println("Something went wrong with the threading.");
  	  System.out.println("Sorry, quitting");
  	  System.out.println("Inspect the stack to see what went wrong");

    } // end try-catch
  }
}
