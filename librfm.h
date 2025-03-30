/* 
 * File:   librfm.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 30. März 2025
 */

#ifndef LIBRFM_H
#define LIBRFM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FIFO        0x00
#define OP_MODE     0x01
#define BITRATE_MSB 0x02
#define BITRATE_LSB 0x03
#define FDEV_MSB    0x04
#define FDEV_LSB    0x05
#define FRF_MSB     0x06
#define FRF_MID     0x07
#define FRF_LSB     0x08
#define PA_CONFIG   0x09
#define PA_RAMP     0x0a
#define OCP         0x0b
#define LNA         0x0c
#define RX_CONFIG   0x0d
#define RSSI_CONFIG 0x0e
#define RSSI_COLLIS 0x0f
#define RSSI_THRESH 0x10
#define RSSI_VALUE  0x11
#define RX_BW       0x12
#define AFC_BW      0x13
#define AFC_FEI     0x1a
#define PREA_DETECT 0x1f
#define RX_TO_RSSI  0x20
#define RX_TO_PREA  0x21
#define RX_TO_SYNC  0x22
#define RX_DELAY    0x23
#define OSC         0x24
#define PREA_MSB    0x25
#define PREA_LSB    0x26
#define SYNC_CONFIG 0x27
#define SYNC_VAL1   0x28
#define SYNC_VAL2   0x29
#define SYNC_VAL3   0x2a
#define SYNC_VAL4   0x2b
#define SYNC_VAL5   0x2c
#define SYNC_VAL6   0x2d
#define SYNC_VAL7   0x2e
#define SYNC_VAL8   0x2f
#define PCK_CONFIG1 0x30
#define PCK_CONFIG2 0x31
#define PAYLOAD_LEN 0x32
#define NODE_ADDR   0x33
#define CAST_ADDR   0x34
#define FIFO_THRESH 0x35
#define IMAGE_CAL   0x3b
#define TEMP        0x3c
#define LOW_BAT     0x3d
#define IRQ_FLAGS1  0x3e
#define IRQ_FLAGS2  0x3f
#define DIO_MAP1    0x40
#define DIO_MAP2    0x41
#define VERSION     0x42

#define MASK_MODE   0x07

#define MODE_SLEEP  0x00
#define MODE_STDBY  0x01
#define MODE_FS_TX  0x02
#define MODE_TX     0x03
#define MODE_FS_RX  0x04
#define MODE_RX     0x05

#define DBM_MIN     2
#define DBM_MAX     17
#define PA_MIN      0
#define PA_MAX      15
#define PA_OFF      2

#define MESSAGE_SIZE    63
#define F_STEP          61035
#define CAST_ADDRESS    0x84

/**
 * Flags for "payload ready" event.
 */
typedef struct {
    bool ready;
    bool crc;
    uint8_t rssi;
} PayloadFlags;

/**
 * F_CPU dependent delay of 5 milliseconds.
 * _delay_ms(5);
 * 
 * @param ms
 */
void _rfmDelay5(void);

/**
 * Turns the radio on by pulling its reset pin LOW.
 * PORTB &= ~(1 << PB0);
 */
void _rfmOn(void);

/**
 * Selects the radio to talk to via SPI.
 * PORTB &= ~(1 << PB1);
 */
void _rfmSel(void);

/**
 * Deselects the radio to talk to via SPI.
 * PORTB |= (1 << PB1);
 */
void _rfmDes(void);

/**
 * SPI transmits/receives given data/returns it.
 * 
 * @param data
 * @return data
 */
uint8_t _rfmTx(uint8_t data);

/**
 * Initializes the radio module with the given carrier frequency in kilohertz
 * and node address. Returns true on success, false otherwise.
 * 
 * @return success
 */
bool rfmInit(uint64_t freq, uint8_t node);

/**
 * Reads interrupt flags. Should be called when any interrupt occurs 
 * on DIO0 or DIO4.
 */
void rfmIrq(void);

/**
 * Shuts down the radio.
 */
void rfmSleep(void);

/**
 * Wakes up the radio.
 */
void rfmWake(void);

/**
 * Sets the node address.
 * 
 * @param address
 */
void rfmSetNodeAddress(uint8_t address);

/**
 * Sets the output power to -2 to +13 dBm. 
 * Values outside that range are ignored.
 * 
 * @param dBm ouput power
 */
void rfmSetOutputPower(int8_t dBm);

/**
 * Returns the current output power setting in dBm.
 *
 * @return ouput power
 */
int8_t rfmGetOutputPower(void);

/**
 * Sets the radio to receive mode and maps "PayloadReady" to DIO0.
 */
void rfmStartReceive(void);

/**
 * Returns true if a "PayloadReady" interrupt arrived and clears the
 * interrupt state.
 * 
 * @return true if "PayloadReady"
 */
PayloadFlags rfmPayloadReady(void);

/**
 * Sets the radio in standby mode, puts the payload into the given array 
 * with the given size, and returns the length of the payload.
 * 
 * @param payload buffer for payload
 * @param size of payload buffer
 * @return payload bytes actually received
 */
size_t rfmReadPayload(uint8_t *payload, size_t size);

/**
 * Waits for "PayloadReady", puts the payload into the given array with the 
 * given size, and returns the length of the payload, or 0 if a timeout 
 * occurred.
 * 
 * @param payload buffer for payload
 * @param size of payload buffer
 * @param timeout enable timeout
 * @return payload bytes actually received
 */
size_t rfmReceivePayload(uint8_t *payload, size_t size, bool timeout);

/**
 * Transmits up to 64 bytes of the given payload with the given node address.
 * 
 * @param payload to be sent
 * @param size of payload
 * @param node address
 * @return payload bytes actually sent
 */
size_t rfmTransmitPayload(uint8_t *payload, size_t size, uint8_t node);

#endif /* LIBRFM_H */

