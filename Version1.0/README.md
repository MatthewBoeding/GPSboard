# PIC18 GPS Board
Code for the PIC18F26Q84 converting NMEA GNGLL packet from a MAX-M8W to CAN bus. This code was written for ECEN-4350 at the University of Nebraska-Lincoln (Scott Campus), so I will not be updating this code once completed. Written for MPLAB X IDE v5.50 and programmed with PicKit4. 
## Code Flow
Upon receiving power, the PIC18 sends packets requesting 1Hz sending form the MAX-M8 and asserts a cold restart (no stored coordinates used). Timer 0 will throw a measurement flag every 4 seconds to update latitude and longitude. The coordinates will be updated by polling for bytes on UART1 until a valid GNGLL packet is recieved. After the packet is received, the new coordinates are sent over the CAN bus. 

## Connections
![Schematic](Schematic_GPS_board_2021-09-22.png?raw=true)

## Issues and Troubleshooting
The code will hang in UART recieve function if nothing is ever received. This is rarely an issue though, since the MAX-M8 is frequently sending information. If you have timing issues, try to write the gpsOn command again. 

The CAN transmit will continuously retransmit if a NAK is received. Be sure your CAN bus is receiving at 500kbps. 
