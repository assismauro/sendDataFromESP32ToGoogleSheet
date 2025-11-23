# sendDataFromESP32ToGoogleSheet
It's an example showing how to collect thermistor data and store in a Google Sheet worksheet. An Arduino-like script run in a NodeMCU 1.0 device collects the temperature measured by three thermistors then send it to a Google Sheet file. You can see it clicking [here](https://docs.google.com/spreadsheets/d/15cVD-PQSRoMAoWLANx6Bgpv8LjGBc5tTnS37DZrOWYg/edit?usp=sharing). 

How I did it:

1) There are two spreadsheets in the Google sheet: one that receive the data, "Página1", and the "Graphic" one, that shows graphically the collected data em real time.
   In "Pagina1"m there´s a headline describing the column data: the timestapmp of the collected data, the three temperature values collected and the time interval  

   <img width="915" height="131" alt="image" src="https://github.com/user-attachments/assets/95ba96be-59fe-48cb-9ea6-2ac3519e69d6" />

