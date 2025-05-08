# librfm95

## About

Static avr-libc library providing basic support for RFM95 (SX1276) radio modules.
Other RFM9x radios might work as well, but were not tested.  

This is work in progress. Currently available is (FSK and LoRa):

- Transmit a packet
- Blocking receive a single packet with timeout
- Async'ly receive a packet (MCU sleeps or does something else until reception) 

## Usage

1. Include `librfm.h` and `librfm.a` in the project
2. Implement the `_rfm*` functions in `librfm.h` in the application
(this is to make the library device and CPU frequency independent)
3. Route interrupts occurring on `DIO0` and `DIO4`(FSK)/`DIO1`(LoRa) to `rfmIrq()`

## Range

### FSK

At almost 8 km distance line of sight, reception was stable with an RSSI of -98 dBm 
and +15 dBm Tx power - with simple wire antennas. Quite impressive!

The configuration:

- AGC auto on, boost on, 150% LNA current
- Modulation shaping Gaussian filter BT = 0.5
- Transmitter frequency deviation: 10 kHz
- Receiver channel filter bandwith: 20.8 kHz

![FieldTest6](https://github.com/user-attachments/assets/7ba77a8e-1384-40ab-b151-372788e90d88)

### LoRa

With LoRa, reception was stable with an RSSI of -116 dBm and +17 dBm Tx power at 
18 km distance line of sight, with the following configuration (and as well just 
simple wire antennas):

- LNA highest gain, boost on, 150% LNA current
- Signal bandwidth: 41.7 kHz
- Spreading factor: 10
- Error correction code rate: 4/5
- Low Data Rate Optimization

![FieldTest8](https://github.com/user-attachments/assets/6570100d-76c0-4678-ac04-e4bbb1f21067)

So, as expected, range is significatly increased with LoRa, and the link is more robust 
when there are obstacles in the path, such as buildings and terrain.  

Anyway, these radio modules work very well with both modulation schemes!  

Here's the transmitter placed at Mont-Saint-Aubert, with a nice view to Mont de L'Enclus:

![Transmitter](https://github.com/user-attachments/assets/5ef7898a-f510-4f30-ab93-302a0ff44af7)
