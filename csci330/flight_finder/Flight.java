/* Flight.java
 *
 * Garrett Gregory --- W01211513
 *
 *
 * The Flight class file. Each Flight object contains its flight number, plane type,
 * airline name, ticket cost, and # of available seats. These Flight objects also
 * have the functionality to print their details (and optionally those of a return flight).
*/

import java.io.*;
import java.util.*;

public class Flight {
    public String flightNum, planeType, airline;
    public int cost, seats;
    
    
    //Constructor
    public Flight(String fNum, int cst, String plType, int sts, String arln) {
            
        this.flightNum = fNum;
        this.planeType = plType;
        this.airline = arln;
        this.cost = cst;
        this.seats = sts;
    }
    
    // printFlight()
    //
    // This is the method used to print individual flight details that have been queried.    
    public void printFlight() {
        System.out.print("--------------------------------------------------------");
        System.out.println("\nFlight Number: " + this.flightNum + "\nAirline : " + this.airline +
                "\nPlane Type: " + this.planeType + "\nSeats Available: " + this.seats +
                "\nPrice: $" + this.cost);
        System.out.print("--------------------------------------------------------\n");
    }
    
    // printFlights()
    //
    // This is the method used to print intial/return flight details that have been queried.
    public void printFlights(Flight retFlight) {
        System.out.print("-----------------------------------------------------------------------------------------------------------------------------------------------------");
        
        System.out.printf("%n%-50s %40s %s", "Initial Flight Number: " + this.flightNum, "", "Return Flight Number: " + retFlight.flightNum);
        System.out.printf("%n%-50s %40s %s", "Initial Plane Type: " + this.planeType, "", "Return Plane Type: " + retFlight.planeType);
        System.out.printf("%n%-50s %40s %s", "Initial Seats Available: " + String.valueOf(this.seats), "", "Return Seats Available: " + String.valueOf(retFlight.seats));
        System.out.printf("%n%-50s %40s %s", "Initial Airline: " + this.airline, "", "Return Airline: " + retFlight.airline);
        System.out.printf("%n%75s%n", "Total Price: $" + String.valueOf(this.cost + retFlight.cost)); 
        
        System.out.print("-----------------------------------------------------------------------------------------------------------------------------------------------------\n");
    
    
    }
    
}