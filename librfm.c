/*
 * File:   rfm69.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28. Januar 2025, 19:57
 */

#include "librfm.h"
#include "types.h"

static volatile uint8_t irqFlags1 = 0;
static volatile uint8_t irqFlags2 = 0;
static volatile uint8_t timeoutInts = 0;
static volatile bool timeoutEnabled = false;

/**
 * Selects the radio to talk to via SPI.
 */
static void spiSel(void) {
    PORT_RFM &= ~(1 << PIN_RCS);
}

/**
 * Deselects the radio to talk to via SPI.
 */
static void spiDes(void) {
    PORT_RFM |= (1 << PIN_RCS);
}

/**
 * Writes the given value to the given register.
 *
 * @param reg
 * @param value
 */
static void regWrite(uint8_t reg, uint8_t value) {
    spiSel();
    transmit(reg | 0x80);
    transmit(value);
    spiDes();
}

/**
 * Reads and returns the value of the given register.
 *
 * @param reg
 * @return value
 */
static uint8_t regRead(uint8_t reg) {
    spiSel();
    transmit(reg & 0x7f);
    uint8_t value = transmit(0x00);
    spiDes();

    return value;
}

/**
 * Sets the module to the given operating mode.
 */
static void setMode(uint8_t mode) {
    regWrite(OP_MODE, (regRead(OP_MODE) & ~MASK_MODE) | (mode & MASK_MODE));
}

/**
 * Clears the IRQ flags read from the module.
 */
static void clearIrqFlags(void) {
    irqFlags1 = 0;
    irqFlags2 = 0;
}

/**
 * Enables timeout (sets timeout interrupt flag on expiration).
 *
 * @param enable
 */
static void timeoutEnable(bool enable) {
    timeoutEnabled = enable;
    if (!enable) timeoutInts = 0;
}

/**
 * Times out the current operation.
 */
static void timeout(void) {
    irqFlags1 |= (1 << 2);
}

void initRadio(uint64_t freq, uint8_t node) {
    // wait a bit after power on
    _delay_ms(10);

    // pull reset LOW to turn on the module
    PORT_RFM &= ~(1 << PIN_RRST);

    _delay_ms(5);

    // uint8_t version = regRead(0x10);
    // printString("Version: ");
    // printHex(version);

    // packet mode, FSK modulation, no shaping (default)
    regWrite(DATA_MOD, 0x00);

    // bit rate 9.6 kBit/s
    // regWrite(BITRATE_MSB, 0x0d);
    // regWrite(BITRATE_LSB, 0x05);

    // frequency deviation (default 5 kHz) - increasing to 10 kHz
    // completely removes susceptibility to temperature changes
    // RX_BW must be increased accordingly
    regWrite(FDEV_MSB, 0x00);
    regWrite(FDEV_LSB, 0xa4);

    // RC calibration, automatically done at device power-up
    // regWrite(OSC1, 0x80);
    // do { } while (!(regRead(OSC1) & 0x40));

    // PA level (default +13 dBm with PA0, yields very weak output power, why?)
    // regWrite(PA_LEVEL, 0x9f);
    // +13 dBm on PA1, yields the expected output power
    regWrite(PA_LEVEL, 0x5f);
    // +17 dBm - doesn't seem to work just like that?
    // regWrite(PA_LEVEL, 0x7f);

    // LNA 200 Ohm, gain AGC (default)
    regWrite(LNA, 0x88);
    // LNA 50 Ohm, gain AGC
    // regWrite(LNA, 0x08);

    // LNA high sensitivity mode
    // regWrite(TEST_LNA, 0x2d);

    // freq of DC offset canceller and channel filter bandwith (default 10.4 kHz)
    // increasing to 20.8 kHz in connection with setting FDEV_*SB to 10 kHz
    // completely removes susceptibility to temperature changes
    regWrite(RX_BW, 0x54);

    // RX_BW during AFC (default 0x8b)
    regWrite(AFC_BW, 0x54);

    // AFC auto on
    // regWrite(AFC_FEI, 0x04);

    // RSSI threshold (default, POR 0xff)
    regWrite(RSSI_THRESH, 0xe4);

    // Preamble size
    regWrite(PREAMB_MSB, 0x00);
    regWrite(PREAMB_LSB, 0x0f);

    // turn off CLKOUT (not used)
    regWrite(DIO_MAP2, 0x07);

    // set the carrier frequency
    uint32_t frf = freq * 100000000000ULL / F_STEP;
    regWrite(FRF_MSB, frf >> 16);
    regWrite(FRF_MID, frf >> 8);
    regWrite(FRF_LSB, frf >> 0);

    // enable sync word generation and detection, FIFO fill on sync address,
    // 4 bytes sync word, tolerate 3 bit errors
    regWrite(SYNC_CONF, 0x9b);

    // just set all sync word values to some really creative value
    regWrite(SYNC_VAL1, 0x2f);
    regWrite(SYNC_VAL2, 0x30);
    regWrite(SYNC_VAL3, 0x31);
    regWrite(SYNC_VAL4, 0x32);
    regWrite(SYNC_VAL5, 0x33);
    regWrite(SYNC_VAL6, 0x34);
    regWrite(SYNC_VAL7, 0x35);
    regWrite(SYNC_VAL8, 0x36);

    // variable payload length, crc on, no address matching
    // regWrite(PCK_CFG1, 0x90);
    // match broadcast or node address
    // regWrite(PCK_CFG1, 0x94);
    // + CrcAutoClearOff
    regWrite(PCK_CFG1, 0x9c);

    // disable automatic RX restart
    regWrite(PCK_CFG2, 0x00);

    // node and broadcast address
    regWrite(NODE_ADDR, node);
    regWrite(CAST_ADDR, CAST_ADDRESS);

    // set TX start condition to "at least one byte in FIFO"
    regWrite(FIFO_THRESH, 0x8f);

    // Fading Margin Improvement, improved margin, use if AfcLowBetaOn=0
    regWrite(TEST_DAGC, 0x30);

    // printString("Radio init done\r\n");
}

/**
 * Reads interrupt flags when a radio interrupt occurs.
 */
void intRadio(void) {
    irqFlags1 = regRead(IRQ_FLAGS1);
    irqFlags2 = regRead(IRQ_FLAGS2);
}

void timeRadio(void) {
    if (timeoutEnabled && timeoutInts++ >= TIMEOUT_INTS) {
        timeoutEnable(false);
        timeout();
    }
}

void sleepRadio(void) {
    setMode(MODE_SLEEP);
}

void wakeRadio(void) {
    setMode(MODE_STDBY);
    // should better wait for ModeReady irq?
    _delay_ms(5);
}

void setNodeAddress(uint8_t address) {
    regWrite(NODE_ADDR, address);
}

uint8_t getRssi(void) {
    return regRead(RSSI_VALUE);
}

void setOutputPower(uint8_t rssi) {
    uint8_t pa = 0x40; // -18 dBm with PA1
    // adjust power from -2 to +13 dBm
    pa += min(max(rssi - 69, PA_MIN), PA_MAX);
    regWrite(PA_LEVEL, pa);
}

uint8_t getOutputPower(void) {
    return regRead(PA_LEVEL);
}

void startReceive(void) {
    // get "PayloadReady" on DIO0
    regWrite(DIO_MAP1, 0x40);

    setMode(MODE_RX);
}

PayloadFlags payloadReady(void) {
    PayloadFlags flags = {.ready = false, .crc = false};
    if (irqFlags2 & (1 << 2)) {
        flags.ready = true;
        flags.crc = irqFlags2 & (1 << 1);
        clearIrqFlags();
        setMode(MODE_STDBY);
    }

    return flags;
}

size_t readPayload(uint8_t *payload, size_t size) {
    size_t len = regRead(FIFO);
    len = min(len, size);

    // TODO assume and ignore address for now
    regRead(FIFO);

    spiSel();
    transmit(FIFO);
    for (size_t i = 0; i < len; i++) {
        payload[i] = transmit(FIFO);
    }
    spiDes();

    return len;
}

size_t receivePayload(uint8_t *payload, size_t size, bool timeout) {
    timeoutEnable(timeout);
    startReceive();

    // wait until "PayloadReady" or timeout
    do {} while (!(irqFlags2 & (1 << 2)) && !(irqFlags1 & (1 << 2)));
    bool ready = irqFlags2 & (1 << 2);
    bool timedout = irqFlags1 & (1 << 2);

    clearIrqFlags();
    if (ready) {
        timeoutEnable(false);
    }
    setMode(MODE_STDBY);

    if (timedout) {
        // full power as last resort
        regWrite(PA_LEVEL, 0x5f);

        return 0;
    }

    return readPayload(payload, size);
}

size_t transmitPayload(uint8_t *payload, size_t size, uint8_t node) {
    // message + address byte
    size_t len = min(size, MESSAGE_SIZE) + 1;

    spiSel();
    transmit(FIFO | 0x80);
    transmit(len);
    transmit(node);
    for (size_t i = 0; i < size; i++) {
        transmit(payload[i]);
    }
    spiDes();

    // get "PacketSent" on DIO0 (default)
    regWrite(DIO_MAP1, 0x00);

    setMode(MODE_TX);

    loop_until_bit_is_set(irqFlags2, 3);
    clearIrqFlags();

    setMode(MODE_STDBY);

    return len;
}
