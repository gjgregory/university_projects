/* Prog3.java
 *
 * Garrett Gregory --- W01211513
 *
 *
 * The driver program for Program 3. This will prompt the user for the type flight they
 * want to query for, then prompt them for flight details and query the specified database
 * for flight options. It will then print a formatted list of flight options and their details.
*/

import java.io.*;
import java.util.*;

public class Prog3 {

    public static void main(String[] args) {
        
        //read arguments (there should only be one)
        if( args.length != 1 ) {
            System.err.println("Error: There should be exactly 1 argument.");
            System.exit(1);
        }
        
        //get connection string file name    
        String connectFileName = args[0];
        String[] connectInfo = null;       
        
        //get connection arguments from file
        try {
            FileReader file = new FileReader(new File(connectFileName));
    	    BufferedReader br = new BufferedReader(file);
    	    String line = br.readLine();
            connectInfo = line.split("\\s+");
        }catch (Exception e) {
            System.err.println("Error: Not able to open file " + connectFileName);
            System.exit(1);
        }
            
        //connect to database
        FlightData data = new FlightData(connectInfo);
        
        Scanner readInput = new Scanner(System.in);
        while (true) {
            System.out.print("\nAre you looking to book a one-way flight or a two-way? (one-way = 1, two-way = 2, exit = 0): ");
            String choice = readInput.nextLine();
            
            //"1" gets one-way flight options
            if (choice.equals("1")){       
                //get flight specifics
                System.out.print("\nEnter origin airport code: ");
                String oCode = readInput.nextLine();
                System.out.print("Enter destination airport code: ");
                String dCode = readInput.nextLine();
                System.out.print("Enter a flight date (YEAR-MO-DY): ");
                String date = readInput.nextLine();
                System.out.print("Enter a your desired cabin (Economy, Business, First): ");
                String cabin = readInput.nextLine();
                
                //get query results
                ArrayList<Flight> outFlights = new ArrayList<>();
                outFlights = data.getOptions(oCode, dCode, date, cabin);
                
                //print results
                if (outFlights != null) {
                    System.out.println("\n\nYou have the following " + String.valueOf(outFlights.size()) + " flight options:\n");
                
                    for (Flight i: outFlights) {
                        i.printFlight();
                    }
                }
                else {
                    System.out.println("\nNo flights found.");
                }
            }
            
            //"2" gets two-way flight options
            else if (choice.equals("2")) {
                //get flight specifics
                System.out.print("\nEnter origin airport code: ");
                String oCode = readInput.nextLine();
                System.out.print("Enter destination airport code: ");
                String dCode = readInput.nextLine();
                System.out.print("Enter an intial flight date (YEAR-MO-DY): ");
                String dateOne = readInput.nextLine();
                System.out.print("Enter a return flight date (YEAR-MO-DY): ");
                String dateTwo = readInput.nextLine();
                System.out.print("Enter a your desired cabin (Economy, Business, First): ");
                String cabin = readInput.nextLine();
                
                //get query results
                ArrayList<Flight> flightsOne = new ArrayList<>();
                ArrayList<Flight> flightsTwo = new ArrayList<>();
                flightsOne = data.getOptions(oCode, dCode, dateOne, cabin);
                flightsTwo = data.getOptions(dCode, oCode, dateTwo, cabin);
                
                //print results
                if (flightsOne != null && flightsTwo != null) {
                    System.out.println("\n\nYou have the following " + String.valueOf(flightsOne.size() * flightsTwo.size()) + " flight options:\n");
                    
                    for (Flight i: flightsOne) {
                        for (Flight j: flightsTwo) {
                        i.printFlights(j);                      
                        }                                      
                    }
                }
                
                else {
                    if (flightsOne == null) {
                    System.out.println("\nNo initial flights found.");
                    }
                    if (flightsTwo == null) {
                    System.out.println("\nNo return flights found.");
                    }
                }
            }
            
            //"0" signals done adding flights
            else if (choice.equals("0")) {
                System.out.println("\nHave a nice day!\n");
                break;
            }
            
            else {
                System.out.println("\n\"" + choice + "\" is not a valid option.");
            } 
                
        }
        
        //close JDBC connection
        data.closeConnection();
    }
}