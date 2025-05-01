# librfm95

## About

Static avr-libc library providing basic support for RFM95 radio modules.
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

With LoRa, reception was stable with about -125 dBm at almost 7 km distance 
somewhat line of sight, with the following configuration (and as well just 
simple wire antennas):

- Output power: +17 dBm
- LNA highest gain, boost on, 150% LNA current
- Signal bandwidth: 62.5 kHz
- Error correction code rate: 4/5
- Spreading factor: 9
- Low Data Rate Optimization

So, LoRa seems to perform significantly better than FSK - as expected - but FSK 
probably can go further than 2.7 km under similar conditions.

Anyway, these radio modules work very well with both modulation schemes!  

![Transmitter placement](https://github.com/user-attachments/assets/0579ce09-9a47-4a64-8224-680413c7764e)

