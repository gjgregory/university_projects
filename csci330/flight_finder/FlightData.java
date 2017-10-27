/* FlightData.java
 *
 * Garrett Gregory --- W01211513
 *
 *
 * The FlightData class file. This will connect to the provided database and find flights
 * that correspond the the flight details provided by the user, then return them.
*/

import java.io.*;
import java.util.*;
import java.sql.*;

public class FlightData {
    private Connection connection;
    private String dbAccess, user, password;
    
    //constructor
    public FlightData(String[] connectInfo) {
        //set connection parameters
        this.dbAccess = connectInfo[0];
        this.user = connectInfo[1];
        this.password = connectInfo[2];       
        
        //initial connection
        this.connectSQL(dbAccess, user, password);

        return;
    }


    // connectSQL()
    //
    // This simply connects to the database provided by the input file.
    // If a connection is failed, the program will terminate.
    public void connectSQL(String db, String user, String pw) {
        
        //load JDBC driver
        try {
            Class.forName("com.mysql.jdbc.Driver");
            System.out.println("Driver loaded...");
        }catch (Exception e) {
            System.err.println("Error: Not able to load driver.");
            System.exit(1);
        }    
        //connect to database
        try {
            this.connection = DriverManager.getConnection(db, user, pw);
            System.out.println("Connected to database...");
        }catch (Exception e) {
            System.err.println("Error: Could not connect to database.");
            System.exit(1);
        }
        return;
    }

    
    // closeConnection() 
    // 
    // This is a simple close connection method, provided so it can be
    // closed from Prog3 instead of internally.
    public void closeConnection() {
        try {
            this.connection.close();
        }catch (Exception e) {
            System.err.println("Error: There is no open connection to close.");
        }
        return;
    }
    
    // getOptions() 
    //
    // This method is used to query the database for unique flights with the same origin airport,
    // destination airport, date, and cabin type that the user has specified. It will then return
    // an ArrayList of Flight objects containing the information of those flights found.
    public ArrayList<Flight> getOptions(String origin, String destination, String date, String cabin) {
        ArrayList<Flight> output = new ArrayList<>();
        try {
        
            Statement statement = this.connection.createStatement();
            //get information on flights with the provided values
            ResultSet resultSet = statement.executeQuery
                    ("SELECT fares.flightNumber, fares.price, CONCAT(aircraft.manufacturer, ' ', aircraft.model), " +
                    "fares.numRows*fares.seatsPerRow, airlines.airline FROM flights, fares, aircraft, airlines WHERE fares.flightNumber = " +
                    "flights.flightNumber AND flights.origin = '" + origin.toUpperCase() + "'AND aircraft.equip = flights.typePlane " +
                    "AND flights.destination = '" + destination.toUpperCase() +"' AND flights.flightDate = '" +date + "' AND fares.cabin = '" + 
                    cabin + "' AND airlines.airlineID = fares.airlineCode AND fares.airlineCode = flights.airlineCode");
            
            //store each tuple in the ArrayList output
            while (!resultSet.isLast()) {
                resultSet.next();
                String flightNum = resultSet.getString(1);
                int price = resultSet.getInt(2);
                String plane = resultSet.getString(3);
                int seats = resultSet.getInt(4);
                String airline = resultSet.getString(5);
                
                output.add(new Flight(flightNum, price, plane, seats, airline));
            }    
        }catch (Exception e) {
            return null;
        }
        return output;
    }
}