/*
 * File:   librfm.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28. Januar 2025, 19:57
 */

#include "librfm.h"
#include "utils.h"

static volatile bool packetSent = false;
static volatile bool payloadReady = false;
static volatile bool timeout = false;

/**
 * Writes the given value to the given register.
 *
 * @param reg
 * @param value
 */
static void regWrite(uint8_t reg, uint8_t value) {
    _rfmSel();
    _rfmTx(reg | 0x80);
    _rfmTx(value);
    _rfmDes();
}

/**
 * Reads and returns the value of the given register.
 *
 * @param reg
 * @return value
 */
static uint8_t regRead(uint8_t reg) {
    _rfmSel();
    _rfmTx(reg & 0x7f);
    uint8_t value = _rfmTx(0x00);
    _rfmDes();

    return value;
}

/**
 * Sets the module to the given operating mode.
 */
static void setMode(uint8_t mode) {
    regWrite(OP_MODE, (regRead(OP_MODE) & ~MASK_MODE) | (mode & MASK_MODE));
}

/**
 * Enables or disables timeouts.
 *
 * @param enable
 */
static void timeoutEnable(bool enable) {
    if (enable) {
        // get "Timeout" on DIO4
        regWrite(DIO_MAP2, (regRead(DIO_MAP2) | 0x80) & ~0x40);
        timeout = false;
        // TODO calculate - seems to be about 50, 75, 100ms
        regWrite(RX_TO_RSSI, 0x1f);
        regWrite(RX_TO_PREA, 0x2f);
        regWrite(RX_TO_SYNC, 0x3f);
    } else {
        regWrite(RX_TO_RSSI, 0x00);
        regWrite(RX_TO_PREA, 0x00);
        regWrite(RX_TO_SYNC, 0x00);
    }
}

bool rfmInit(uint64_t freq, uint8_t node) {
    // wait a bit after power on
    _rfmDelay5();
    _rfmDelay5();

    // pull reset high to turn on the module
    _rfmOn();

    _rfmDelay5();

    uint8_t version = regRead(VERSION);
    // printString("Version: ");
    // printHex(version);
    if (version == 0x00) {
        return false;
    }

    // FSK modulation, high frequency mode, sleep mode
    regWrite(OP_MODE, 0x00);

    // bit rate 9.6 kBit/s
    // regWrite(BITRATE_MSB, 0x0d);
    // regWrite(BITRATE_LSB, 0x05);

    // frequency deviation 10 kHz (default 5 kHz)
    regWrite(FDEV_MSB, 0x00);
    regWrite(FDEV_LSB, 0xa4);

    // set the carrier frequency
    uint32_t frf = freq * 1000000ULL / F_STEP;
    regWrite(FRF_MSB, frf >> 16);
    regWrite(FRF_MID, frf >> 8);
    regWrite(FRF_LSB, frf >> 0);

    // PA level +17 dBm with PA_BOOST pin (Pmax default/not relevant)
    regWrite(PA_CONFIG, 0xff);

    // LNA highest gain, no current adjustment
    regWrite(LNA, 0x20);

    // AgcAutoOn, receiver trigger event PreambleDetect
    regWrite(RX_CONFIG, 0x0e);

    // no RSSI offset, 32 samples used
    regWrite(RSSI_CONFIG, 0x04);

    // 10 dB threshold for interferer detection
    regWrite(RSSI_COLLIS, 0x0a);

    // RSSI threshold (POR 0xff)
    regWrite(RSSI_THRESH, 0x94);

    // channel filter bandwith 20.8 kHz (default 10.4 kHz)
    regWrite(RX_BW, 0x14);

    // RX_BW during AFC
    regWrite(AFC_BW, 0x14);

    // AFC auto on
    // regWrite(AFC_FEI, 0x00);

    // PreambleDetectorOn, PreambleDetectorSize 2 bytes,
    // PreambleDetectorTol 4 chips per bit
    regWrite(PREA_DETECT, 0xaa);

    // Preamble size
    regWrite(PREA_MSB, 0x00);
    regWrite(PREA_LSB, 0x03);

    // AutoRestartRxMode off, PreamblePolarity 0xaa, SyncOn,
    // FifoFillCondition if SyncAddress, SyncSize + 1 = 4 bytes
    regWrite(SYNC_CONFIG, 0x13);

    // just set all sync word values to some really creative value
    regWrite(SYNC_VAL1, 0x2f);
    regWrite(SYNC_VAL2, 0x30);
    regWrite(SYNC_VAL3, 0x31);
    regWrite(SYNC_VAL4, 0x32);
    regWrite(SYNC_VAL5, 0x33);
    regWrite(SYNC_VAL6, 0x34);
    regWrite(SYNC_VAL7, 0x35);
    regWrite(SYNC_VAL8, 0x36);

    // variable payload length, DcFree none, CrcOn, CrcAutoClearOff,
    // match broadcast or node address
    regWrite(PCK_CONFIG1, 0x9c);

    // Packet mode, ..., PayloadLength(10:8)
    regWrite(PCK_CONFIG2, 0x40);

    // PayloadLength(7:0)
    regWrite(PAYLOAD_LEN, 0x40);

    // node and broadcast address
    regWrite(NODE_ADDR, node);
    regWrite(CAST_ADDR, CAST_ADDRESS);

    // set TX start condition to "at least one byte in FIFO"
    regWrite(FIFO_THRESH, 0x8f);

    // AutoImageCalOn disabled, TempThreshold 10°C, TempMonitorOff 0
    regWrite(IMAGE_CAL, 0x02);

    // printString("Radio init done\r\n");

    return true;
}

void rfmIrq(void) {
    uint8_t irqFlags1 = regRead(IRQ_FLAGS1);
    uint8_t irqFlags2 = regRead(IRQ_FLAGS2);

    if (irqFlags1 & (1 << 2)) timeout = true;
    if (irqFlags2 & (1 << 3)) packetSent = true;
    if (irqFlags2 & (1 << 2)) payloadReady = true;
}

void rfmTimeout(void) {
    timeout = true;
}

void rfmSleep(void) {
    _rfmDelay5();
    setMode(MODE_SLEEP);
}

void rfmWake(void) {
    setMode(MODE_STDBY);
    // should better wait for ModeReady irq?
    _rfmDelay5();
}

void rfmSetNodeAddress(uint8_t address) {
    regWrite(NODE_ADDR, address);
}

void rfmSetOutputPower(int8_t dBm) {
    uint8_t pa = 0xc0; // +2 dBm with PA_BOOST
    // adjust power from 2 to +17 dBm
    pa |= (min(max(dBm - PA_OFF, PA_MIN), PA_MAX)) & 0x0f;
    regWrite(PA_CONFIG, pa);
}

int8_t rfmGetOutputPower(void) {
    return (regRead(PA_CONFIG) & 0x0f) + PA_OFF;
}

void rfmStartReceive(bool timeout) {
    timeoutEnable(timeout);

    // get "PayloadReady" on DIO0
    regWrite(DIO_MAP1, regRead(DIO_MAP1) & ~0xc0);
    payloadReady = false;

    setMode(MODE_RX);
}

PayloadFlags rfmPayloadReady(void) {
    PayloadFlags flags = {.ready = false, .rssi = 255, .crc = false};
    if (payloadReady) {
        flags.ready = true;
        flags.rssi = regRead(RSSI_VALUE);
        flags.crc = regRead(IRQ_FLAGS2) & (1 << 1);
        setMode(MODE_STDBY);
    }

    return flags;
}

size_t rfmReadPayload(uint8_t *payload, size_t size) {
    size_t len = regRead(FIFO);
    len = min(len, size);

    // TODO assume and ignore address for now (already filtered anyway)
    regRead(FIFO);

    _rfmSel();
    _rfmTx(FIFO);
    for (size_t i = 0; i < len; i++) {
        payload[i] = _rfmTx(FIFO);
    }
    _rfmDes();

    return len;
}

size_t rfmReceivePayload(uint8_t *payload, size_t size, bool enable) {
    rfmStartReceive(enable);

    // wait until "PayloadReady" or (forced) "Timeout"
    do {} while (!payloadReady && !timeout);

    if (payloadReady) {
        timeoutEnable(false);
    }

    setMode(MODE_STDBY);

    if (timeout) {
        // full power as last resort, indicate timeout
        regWrite(PA_CONFIG, 0xff);

        return 0;
    }

    return rfmReadPayload(payload, size);
}

size_t rfmTransmitPayload(uint8_t *payload, size_t size, uint8_t node) {
    // message + address byte
    size_t len = min(size, MESSAGE_SIZE) + 1;

    _rfmSel();
    _rfmTx(FIFO | 0x80);
    _rfmTx(len);
    _rfmTx(node);
    for (size_t i = 0; i < size; i++) {
        _rfmTx(payload[i]);
    }
    _rfmDes();

    // get "PacketSent" on DIO0 (default)
    regWrite(DIO_MAP1, regRead(DIO_MAP1) & ~0xc0);
    packetSent = false;

    setMode(MODE_TX);

    // wait until "PacketSent"
    do {} while (!packetSent);

    setMode(MODE_STDBY);

    return len;
}
