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

/* Registers shared by FSK and LoRa mode */
#define RFM_FIFO                0x00
#define RFM_OP_MODE             0x01
#define RFM_FRF_MSB             0x06
#define RFM_FRF_MID             0x07
#define RFM_FRF_LSB             0x08
#define RFM_PA_CONFIG           0x09
#define RFM_PA_RAMP             0x0a
#define RFM_OCP                 0x0b
#define RFM_LNA                 0x0c
#define RFM_DIO_MAP1            0x40
#define RFM_DIO_MAP2            0x41
#define RFM_VERSION             0x42

/* FSK mode registers */
#define RFM_FSK_BITRATE_MSB     0x02
#define RFM_FSK_BITRATE_LSB     0x03
#define RFM_FSK_FDEV_MSB        0x04
#define RFM_FSK_FDEV_LSB        0x05
#define RFM_FSK_RX_CONFIG       0x0d
#define RFM_FSK_RSSI_CONFIG     0x0e
#define RFM_FSK_RSSI_COLLIS     0x0f
#define RFM_FSK_RSSI_THRESH     0x10
#define RFM_FSK_RSSI_VALUE      0x11
#define RFM_FSK_RX_BW           0x12
#define RFM_FSK_AFC_BW          0x13
#define RFM_FSK_AFC_FEI         0x1a
#define RFM_FSK_PREA_DETECT     0x1f
#define RFM_FSK_RX_TO_RSSI      0x20
#define RFM_FSK_RX_TO_PREA      0x21
#define RFM_FSK_RX_TO_SYNC      0x22
#define RFM_FSK_RX_DELAY        0x23
#define RFM_FSK_OSC             0x24
#define RFM_FSK_PREA_MSB        0x25
#define RFM_FSK_PREA_LSB        0x26
#define RFM_FSK_SYNC_CONFIG     0x27
#define RFM_FSK_SYNC_VAL1       0x28
#define RFM_FSK_SYNC_VAL2       0x29
#define RFM_FSK_SYNC_VAL3       0x2a
#define RFM_FSK_SYNC_VAL4       0x2b
#define RFM_FSK_SYNC_VAL5       0x2c
#define RFM_FSK_SYNC_VAL6       0x2d
#define RFM_FSK_SYNC_VAL7       0x2e
#define RFM_FSK_SYNC_VAL8       0x2f
#define RFM_FSK_PCK_CONFIG1     0x30
#define RFM_FSK_PCK_CONFIG2     0x31
#define RFM_FSK_PAYLOAD_LEN     0x32
#define RFM_FSK_NODE_ADDR       0x33
#define RFM_FSK_CAST_ADDR       0x34
#define RFM_FSK_FIFO_THRESH     0x35
#define RFM_FSK_IMAGE_CAL       0x3b
#define RFM_FSK_TEMP            0x3c
#define RFM_FSK_LOW_BAT         0x3d
#define RFM_FSK_IRQ_FLAGS1      0x3e
#define RFM_FSK_IRQ_FLAGS2      0x3f

/* LoRa mode registers */
#define RFM_LORA_FIFO_ADDR_PTR  0x0d
#define RFM_LORA_FIFO_TX_ADDR   0x0e
#define RFM_LORA_FIFO_RX_ADDR   0x0f
#define RFM_LORA_FIFO_CURR_ADDR 0x10
#define RFM_LORA_IRQ_FLAGS_MASK 0x11
#define RFM_LORA_IRQ_FLAGS      0x12
#define RFM_LORA_RX_BYTES_NB    0x13
#define RFM_LORA_RX_HDR_CNT_MSB 0x14
#define RFM_LORA_RX_HDR_CNT_LSB 0x15
#define RFM_LORA_RX_PCK_CNT_MSB 0x16
#define RFM_LORA_RX_PCK_CNT_LSB 0x17
#define RFM_LORA_MODEM_STAT     0x18
#define RFM_LORA_PCK_SNR        0x19
#define RFM_LORA_PCK_RSSI       0x1a
#define RFM_LORA_RSSI           0x1b
#define RFM_LORA_HOP_CHANNEL    0x1c
#define RFM_LORA_MODEM_CONFIG1  0x1d
#define RFM_LORA_MODEM_CONFIG2  0x1e
#define RFM_LORA_SYMB_TIMEO_LSB 0x1f
#define RFM_LORA_PREA_LEN_MSB   0x20
#define RFM_LORA_PREA_LEN_LSB   0x21
#define RFM_LORA_PAYLD_LEN      0x22
#define RFM_LORA_PAYLD_MAX_LEN  0x23
#define RFM_LORA_HOP_PERIOD     0x24
#define RFM_LORA_FIFO_RX_B_ADDR 0x25
#define RFM_LORA_MODEM_CONFIG3  0x26

/* Values shared by FSK and LoRa mode */
#define RFM_MODE_SLEEP          0x00
#define RFM_MODE_STDBY          0x01
#define RFM_MODE_FS_TX          0x02
#define RFM_MODE_TX             0x03
#define RFM_MODE_FS_RX          0x04
#define RFM_MODE_RX             0x05 // LoRa RXCONTINUOUS
#define RFM_MODE_RXSINGLE       0x06 // LoRa only
#define RFM_MODE_CAD            0x07 // LoRa only
#define RFM_MASK_MODE           0x07

#define RFM_F_STEP              61035
#define RFM_CAST_ADDRESS        0x84

#define RFM_DBM_MIN             2
#define RFM_DBM_MAX             17
#define RFM_PA_MIN              0
#define RFM_PA_MAX              15
#define RFM_PA_OFF              2

/* FSK mode values */
#define RFM_FSK_MSG_SIZE        63

/* LoRa mode values */
// assuming 50/50 for Tx/Rx for now
#define RFM_LORA_MSG_SIZE       128

/**
 * Flags for 'PayloadReady'/'RxDone' event.
 */
typedef struct {
    bool ready;
    bool crc;
    uint8_t rssi;
} RxFlags;

/**
 * F_CPU dependent delay of 5 milliseconds.
 * _delay_ms(5);
 * 
 * @param ms
 */
void _rfmDelay5(void);

/**
 * Turns the radio on by pulling its reset pin high.
 * PORTB |= (1 << PB0);
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
 * Initializes the radio module in FSK or LoRa mode with the given carrier 
 * frequency in kilohertz and node address. Returns true on success, 
 * false otherwise.
 * 
 * @return success
 */
bool rfmInit(uint64_t freq, uint8_t node, bool lora);

/**
 * Reads interrupt flags. Should be called when any interrupt occurs 
 * on DIO0 or DIO4.
 */
void rfmIrq(void);

/**
 * Sets the "Timeout" interrupt flag, allowing to "unlock" a possibly hanging 
 * wait for either "PayloadReady" or "Timeout" by the radio in FSK mode, 
 * ignored in LoRa mode.
 */
void rfmTimeout(void);

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
 * Sets the radio to receive mode and maps "PayloadReady" to DIO0 and enables
 * or disables timeout.
 * 
 * @param timeout enable timeout
 */
void rfmStartReceive(bool timeout);

/**
 * Returns true and puts the radio in standby mode if a "PayloadReady" 
 * interrupt arrived.
 * 
 * @return flags
 */
RxFlags rfmPayloadReady(void);

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
 * given size, enables or disables timeout, and returns the length of the 
 * payload, or 0 if a timeout occurred.
 * 
 * @param payload buffer for payload
 * @param size of payload buffer
 * @param timeout enable timeout
 * @return payload bytes actually received
 */
size_t rfmReceivePayload(uint8_t *payload, size_t size, bool timeout);

/**
 * Transmits up to 63 bytes of the given payload with the given node address.
 * 
 * @param payload to be sent
 * @param size of payload
 * @param node address
 * @return payload bytes actually sent
 */
size_t rfmTransmitPayload(uint8_t *payload, size_t size, uint8_t node);

/**
 * Sets the radio in continous receive mode and maps "RxDone" to DIO0.
 */
void rfmLoRaStartRx(void);

/**
 * Returns true if a "RxDone" interrupt arrived.
 * 
 * @return flags
 */
RxFlags rfmLoRaRxDone(void);

/**
 * Puts the received payload into the given array with the given size, 
 * and returns the length of the payload.
 * 
 * @param payload buffer for payload
 * @param size of payload buffer
 * @return payload bytes actually received
 */
size_t rfmLoRaRxRead(uint8_t *payload, size_t size);

/**
 * Sets the radio in single receive mode, waits for "RxDone" with timeout, 
 * puts the payload into the given array with the given size, and returns 
 * the length of the payload, or 0 if a timeout occurred.
 * 
 * @param payload buffer for payload
 * @param size of payload buffer
 * @return payload bytes actually received
 */
size_t rfmLoRaRx(uint8_t *payload, size_t size);

/**
 * Transmits up to 128 bytes of the given payload.
 * 
 * @param payload to be sent
 * @param size of payload
 * @return payload bytes actually sent
 */
size_t rfmLoRaTx(uint8_t *payload, size_t size);

#endif /* LIBRFM_H */
