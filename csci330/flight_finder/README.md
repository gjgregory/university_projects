## Flight Finder

This is a program that queries a database, which was hosted on a school server, to find flights that match user-supplied attributes. It then displays formatted results on standard output.

* Prog3.java - Driver program.
* FlightData.java - Provides the FlightData class, which connects to the database and finds flights.
* Flight.java - Provides the Flight class. Each flight object contains information for a single flight.
* connection.txt - Template for connection information to be supplied.

Usage:
```
$ java -cp .:mysql-connector-java-5.1.18-bin.jar Prog3 connection.txt
```
