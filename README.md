# librfm95

## About

Static avr-libc library providing basic support for RFM95 (SX1276) radio modules.
Other RFM9x radios might work as well, but were not tested yet.  

I'm impressed how well these radio modules work; the range achieved with 
simple wire antennas as well as the reliable packet transmission.  

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

Transmitting with +17 dBm, reception stopped with an RSSI of about -125 dBm at 
almost 2.7 km distance - with simple wire antennas, and a low hill in between.  

Compared to the [RFM69](https://github.com/gitdode/librfm) at +13 dBm, it does 
make it a few hundred meters further - seems fair enough!  

![FieldTest4](https://github.com/user-attachments/assets/67f745c4-a47f-4cb1-a278-547a0b0e01e3)

### LoRa

With LoRa, reception was stable with an RSSI of -111 dBm and +15 dBm Tx power at 
over 12 km distance line of sight, with the following configuration (and as well 
just simple wire antennas):

- LNA highest gain, boost on, 150% LNA current
- Signal bandwidth: 41.7 kHz
- Error correction code rate: 4/5
- Spreading factor: 10
- Low Data Rate Optimization

![FieldTest5](https://github.com/user-attachments/assets/7f1d0ec2-f95d-472f-9510-919c16c1f7f6)

So, LoRa seems to perform significantly better than FSK - as expected - but FSK 
certainly can go a lot further than 2.7 km under similar conditions (plain line of sight).

Anyway, these radio modules work very well with both modulation schemes!  

Here's the transmitter placed at Mont-Saint-Aubert, with a nice view to Mont de L'Enclus,
were the receiver was located:

![Transmitter](https://github.com/user-attachments/assets/5ef7898a-f510-4f30-ab93-302a0ff44af7)
