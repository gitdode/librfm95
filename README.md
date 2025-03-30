# librfm

## About

Static avr-libc library providing basic support for RFM95 radio modules.
Other RFM9x radios should work as well, but were not tested yet.  

This is work in progress. Simple Tx-Rx with response works so far.  

I'm impressed how well these radio modules work; the range achieved with 
simple wire antennas as well as the reliable packet transmission.  

## TODO

- Add support for LoRa

## Usage

1. Include `librfm.h` and `librfm.a` in the project
2. Implement the `_rfm*` functions in `librfm.h` in the application
(this is to make the library device and CPU frequency independent)
3. Route interrupts occurring on `DIO0` and `DIO4` to `rfmIrq()`

## Range

Setting `RegPaLevel` to `0x5f`, which gives +13 dBm with `PA1`, indoor range is 
very good and in an actual "field" test, packet reception was still reliable 
with an RSSI of about -90 dBm at about 2.2 km distance - with simple wire 
antennas. What would be the range with +20 dBm and decent antennas?  

![FieldTest3](https://github.com/user-attachments/assets/f2289f8e-1f81-4b85-9146-07c2ce1bb563)

## Susceptibility to Temperature Changes*

*This originates from RFM69 radios, if it applies also to RFM9x is yet to be tested.  

With the default frequency deviation of 5 kHz and receiver bandwidth of 
10.4 kHz, packet transmission is very unreliable and fails completely for me 
when the temperature of the transmitter is below 10°C and above 40°C, while 
the receiver temperature is at 20°C. The receiver does not seem to be prone to 
temperature changes.  
Increasing frequency deviation to 10 kHz and receiver bandwidth to 20.8 kHz, 
temperature susceptibility is eliminated; when testing with transmitter 
temperature from -20°C to 50°C, packet transmission is perfectly reliable.

Frequency Deviation = 10 kHz (transmitter)  
`RegFdevMsb` = `0x00`  
`RegFdevLsb` = `0xa4`  

Receiver Bandwidth = 20.8 kHz  
`RegRxBw` = `0x54`  
