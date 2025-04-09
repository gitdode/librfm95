/*
 * File:   librfm.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28. Januar 2025, 19:57
 */

#include "librfm.h"
#include "utils.h"

/* FSK 'Timeout' */
static volatile bool rxTimeout = false;
/* FSK 'PacketSent' */
static volatile bool txDone = false;
/* FSK 'PayloadReady' */
static volatile bool rxDone = false;

/* Current mode: LoRa or FSK */
static bool lora = false;

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
    regWrite(RFM_OP_MODE, (regRead(RFM_OP_MODE) & ~RFM_MASK_MODE) | (mode & RFM_MASK_MODE));
}

/**
 * Enables or disables timeouts in FSK mode.
 *
 * @param enable
 */
static void timeoutEnableFSK(bool enable) {
    if (enable) {
        // get "Timeout" on DIO4
        regWrite(RFM_DIO_MAP2, (regRead(RFM_DIO_MAP2) | 0x80) & ~0x40);
        rxTimeout = false;
        // TODO calculate - seems to be about 50, 75, 100ms
        regWrite(RFM_FSK_RX_TO_RSSI, 0x1f);
        regWrite(RFM_FSK_RX_TO_PREA, 0x2f);
        regWrite(RFM_FSK_RX_TO_SYNC, 0x3f);
    } else {
        regWrite(RFM_FSK_RX_TO_RSSI, 0x00);
        regWrite(RFM_FSK_RX_TO_PREA, 0x00);
        regWrite(RFM_FSK_RX_TO_SYNC, 0x00);
    }
}

static bool initFSK(uint8_t node) {
    // bit rate 9.6 kBit/s
    // regWrite(BITRATE_MSB, 0x0d);
    // regWrite(BITRATE_LSB, 0x05);

    // frequency deviation 10 kHz (default 5 kHz)
    regWrite(RFM_FSK_FDEV_MSB, 0x00);
    regWrite(RFM_FSK_FDEV_LSB, 0xa4);

    // AgcAutoOn, receiver trigger event PreambleDetect
    regWrite(RFM_FSK_RX_CONFIG, 0x0e);

    // no RSSI offset, 32 samples used
    regWrite(RFM_FSK_RSSI_CONFIG, 0x04);

    // 10 dB threshold for interferer detection
    regWrite(RFM_FSK_RSSI_COLLIS, 0x0a);

    // RSSI threshold (POR 0xff)
    regWrite(RFM_FSK_RSSI_THRESH, 0x94);

    // channel filter bandwith 20.8 kHz (default 10.4 kHz)
    regWrite(RFM_FSK_RX_BW, 0x14);

    // RX_BW during AFC
    regWrite(RFM_FSK_AFC_BW, 0x14);

    // AFC auto on
    // regWrite(AFC_FEI, 0x00);

    // PreambleDetectorOn, PreambleDetectorSize 2 bytes,
    // PreambleDetectorTol 4 chips per bit
    regWrite(RFM_FSK_PREA_DETECT, 0xaa);

    // Preamble size
    regWrite(RFM_FSK_PREA_MSB, 0x00);
    regWrite(RFM_FSK_PREA_LSB, 0x03);

    // AutoRestartRxMode off, PreamblePolarity 0xaa, SyncOn,
    // FifoFillCondition if SyncAddress, SyncSize + 1 = 4 bytes
    regWrite(RFM_FSK_SYNC_CONFIG, 0x13);

    // just set all sync word values to some really creative value
    regWrite(RFM_FSK_SYNC_VAL1, 0x2f);
    regWrite(RFM_FSK_SYNC_VAL2, 0x30);
    regWrite(RFM_FSK_SYNC_VAL3, 0x31);
    regWrite(RFM_FSK_SYNC_VAL4, 0x32);
    regWrite(RFM_FSK_SYNC_VAL5, 0x33);
    regWrite(RFM_FSK_SYNC_VAL6, 0x34);
    regWrite(RFM_FSK_SYNC_VAL7, 0x35);
    regWrite(RFM_FSK_SYNC_VAL8, 0x36);

    // variable payload length, DcFree none, CrcOn, CrcAutoClearOff,
    // match broadcast or node address
    regWrite(RFM_FSK_PCK_CONFIG1, 0x9c);

    // Packet mode, ..., PayloadLength(10:8)
    regWrite(RFM_FSK_PCK_CONFIG2, 0x40);

    // PayloadLength(7:0)
    regWrite(RFM_FSK_PAYLOAD_LEN, 0x40);

    // node and broadcast address
    regWrite(RFM_FSK_NODE_ADDR, node);
    regWrite(RFM_FSK_CAST_ADDR, RFM_CAST_ADDRESS);

    // set TX start condition to "at least one byte in FIFO"
    regWrite(RFM_FSK_FIFO_THRESH, 0x8f);

    // AutoImageCalOn disabled, TempThreshold 10°C, TempMonitorOff 0
    regWrite(RFM_FSK_IMAGE_CAL, 0x02);

    // printString("Radio init done\r\n");

    return true;
}

static bool initLoRa(uint8_t node) {
    // SPI interface address pointer in FIFO data buffer (POR 0x00)
    regWrite(RFM_LORA_FIFO_ADDR_PTR, 0x00);

    // write base address in FIFO data buffer for TX modulator (POR 0x80)
    regWrite(RFM_LORA_FIFO_TX_ADDR, 0x80);

    // read base address in FIFO data buffer for RX demodulator (POR 0x00)
    regWrite(RFM_LORA_FIFO_RX_ADDR, 0x00);

    // signal bandwidth 62.5 kHz, error coding rate 4/5, explicit header mode
    regWrite(RFM_LORA_MODEM_CONFIG1, 0x62);

    // spreading factor 9, TX single packet mode, CRC enable, RX timeout MSB
    regWrite(RFM_LORA_MODEM_CONFIG2, 0x94);

    // RX timeout LSB
    regWrite(RFM_LORA_SYMB_TIMEO_LSB, 0x64);

    // static node, AGC auto off
    regWrite(RFM_LORA_MODEM_CONFIG3, 0x00);

    // preamble length MSB
    regWrite(RFM_LORA_PREA_LEN_MSB, 0x00);

    // preamble length LSB
    regWrite(RFM_LORA_PREA_LEN_LSB, 0x08);

    // payload length in bytes (>0 only for implicit header mode)
    // regWrite(RFM_LORA_PAYLD_LEN, 0x01);

    // max payload length (CRC error if exceeded)
    regWrite(RFM_LORA_PAYLD_MAX_LEN, 0xff);

    // frequency hopping disabled
    regWrite(RFM_LORA_HOP_PERIOD, 0x00);

    return true;
}

bool rfmInit(uint64_t freq, uint8_t node, bool _lora) {
    lora = _lora;

    // wait a bit after power on
    _rfmDelay5();
    _rfmDelay5();

    // pull reset high to turn on the module
    _rfmOn();
    _rfmDelay5();

    uint8_t version = regRead(RFM_VERSION);
    // printString("Version: ");
    // printHex(version);
    if (version == 0x00) {
        return false;
    }

    // set the carrier frequency
    uint32_t frf = freq * 1000000UL / RFM_F_STEP;
    regWrite(RFM_FRF_MSB, frf >> 16);
    regWrite(RFM_FRF_MID, frf >> 8);
    regWrite(RFM_FRF_LSB, frf >> 0);

    // PA level +17 dBm with PA_BOOST pin (Pmax default/not relevant)
    regWrite(RFM_PA_CONFIG, 0xff);

    // LNA highest gain, no current adjustment
    regWrite(RFM_LNA, 0x20);

    if (lora) {
        // go to sleep before switching to LoRa mode
        setMode(RFM_MODE_SLEEP);
        _rfmDelay5();

        // LoRa mode, high frequency mode, sleep mode
        regWrite(RFM_OP_MODE, 0x80);

        return initLoRa(node);
    } else {
        // FSK mode, FSK modulation, high frequency mode, sleep mode
        regWrite(RFM_OP_MODE, 0x00);

        return initFSK(node);
    }
}

void rfmIrq(void) {
    if (lora) {
        uint8_t irqFlags = regRead(RFM_LORA_IRQ_FLAGS);

        if (irqFlags & (1 << 7)) rxTimeout = true;
        if (irqFlags & (1 << 3)) txDone = true;
        if (irqFlags & (1 << 6)) rxDone = true;
    } else {
        uint8_t irqFlags1 = regRead(RFM_FSK_IRQ_FLAGS1);
        uint8_t irqFlags2 = regRead(RFM_FSK_IRQ_FLAGS2);

        if (irqFlags1 & (1 << 2)) rxTimeout = true;
        if (irqFlags2 & (1 << 3)) txDone = true;
        if (irqFlags2 & (1 << 2)) rxDone = true;
    }
}

void rfmTimeout(void) {
    if (!lora) {
        // workaround for timeout interrupt sometimes not occurring in FSK mode
        // https://electronics.stackexchange.com/q/743099/65699
        rxTimeout = true;
    }
}

void rfmSleep(void) {
    _rfmDelay5();
    setMode(RFM_MODE_SLEEP);
}

void rfmWake(void) {
    setMode(RFM_MODE_STDBY);
    // should better wait for ModeReady irq?
    _rfmDelay5();
}

void rfmSetNodeAddress(uint8_t address) {
    if (!lora) {
        regWrite(RFM_FSK_NODE_ADDR, address);
    }
}

void rfmSetOutputPower(int8_t dBm) {
    uint8_t pa = 0xc0; // +2 dBm with PA_BOOST
    // adjust power from 2 to +17 dBm
    pa |= (min(max(dBm - RFM_PA_OFF, RFM_PA_MIN), RFM_PA_MAX)) & 0x0f;
    regWrite(RFM_PA_CONFIG, pa);
}

int8_t rfmGetOutputPower(void) {
    return (regRead(RFM_PA_CONFIG) & 0x0f) + RFM_PA_OFF;
}

void rfmStartReceive(bool timeout) {
    timeoutEnableFSK(timeout);

    // get "PayloadReady" on DIO0
    regWrite(RFM_DIO_MAP1, regRead(RFM_DIO_MAP1) & ~0xc0);
    rxDone = false;

    setMode(RFM_MODE_RX);
}

RxFlags rfmPayloadReady(void) {
    RxFlags flags = {.ready = false, .rssi = 255, .crc = false};
    if (rxDone) {
        flags.ready = true;
        flags.rssi = divRoundNearest(regRead(RFM_FSK_RSSI_VALUE), 2);
        flags.crc = regRead(RFM_FSK_IRQ_FLAGS2) & (1 << 1);
        setMode(RFM_MODE_STDBY);
    }

    return flags;
}

size_t rfmReadPayload(uint8_t *payload, size_t size) {
    size_t len = regRead(RFM_FIFO);
    len = min(len, size);

    // TODO assume and ignore address for now (already filtered anyway)
    regRead(RFM_FIFO);

    _rfmSel();
    _rfmTx(RFM_FIFO);
    for (size_t i = 0; i < len; i++) {
        payload[i] = _rfmTx(RFM_FIFO);
    }
    _rfmDes();

    return len;
}

size_t rfmReceivePayload(uint8_t *payload, size_t size, bool enable) {
    rfmStartReceive(enable);

    // wait until "PayloadReady" or (forced) "Timeout"
    do {} while (!rxDone && !rxTimeout);

    if (rxDone) {
        timeoutEnableFSK(false);
    }

    setMode(RFM_MODE_STDBY);

    if (rxTimeout) {
        // full power as last resort, indicate timeout
        regWrite(RFM_PA_CONFIG, 0xff);

        return 0;
    }

    return rfmReadPayload(payload, size);
}

size_t rfmTransmitPayload(uint8_t *payload, size_t size, uint8_t node) {
    // message + address byte
    size_t len = min(size, RFM_FSK_MSG_SIZE) + 1;

    _rfmSel();
    _rfmTx(RFM_FIFO | 0x80);
    _rfmTx(len);
    _rfmTx(node);
    for (size_t i = 0; i < size; i++) {
        _rfmTx(payload[i]);
    }
    _rfmDes();

    // get "PacketSent" on DIO0 (default)
    regWrite(RFM_DIO_MAP1, regRead(RFM_DIO_MAP1) & ~0xc0);
    txDone = false;

    setMode(RFM_MODE_TX);

    // wait until "PacketSent"
    do {} while (!txDone);

    setMode(RFM_MODE_STDBY);

    return len;
}

void rfmLoRaStartRx(void) {
    // clear "RxDone" and "PayloadCrcError" interrupt
    regWrite(RFM_LORA_IRQ_FLAGS, 0x40 | 0x20);
    rxDone = false;

    // get "RxDone" on DIO0
    regWrite(RFM_DIO_MAP1, regRead(RFM_DIO_MAP1) & ~0xc0);

    // set FIFO address pointer to configured TX base address
    regWrite(RFM_LORA_FIFO_ADDR_PTR, regRead(RFM_LORA_FIFO_RX_ADDR));

    // TODO already is in continuous RX mode most of the time
    setMode(RFM_MODE_RX);
}

RxFlags rfmLoRaRxDone(void) {
    RxFlags flags = {.ready = false, .rssi = 255, .crc = false};
    if (rxDone) {
        flags.ready = true;
        flags.rssi = 157 - regRead(RFM_LORA_PCK_RSSI);
        flags.crc = !(regRead(RFM_LORA_IRQ_FLAGS) & (1 << 5));
    }

    return flags;
}

size_t rfmLoRaRxRead(uint8_t *payload, size_t size) {
    // set FIFO address pointer to current RX address
    regWrite(RFM_LORA_FIFO_ADDR_PTR, regRead(RFM_LORA_FIFO_CURR_ADDR));

    size_t len = regRead(RFM_LORA_RX_BYTES_NB);
    len = min(len, size);

    _rfmSel();
    _rfmTx(RFM_FIFO);
    for (size_t i = 0; i < len; i++) {
        payload[i] = _rfmTx(RFM_FIFO);
    }
    _rfmDes();

    return len;
}

size_t rfmLoRaRx(uint8_t *payload, size_t size) {
    // clear "RxTimeout" and "RxDone" interrupt
    // regWrite(RFM_LORA_IRQ_FLAGS, 0xc0);

    // get "RxTimeout" on DIO1 and "RxDone" on DIO0
    regWrite(RFM_DIO_MAP1, regRead(RFM_DIO_MAP1) & ~0xf0);
    rxTimeout = false;
    rxDone = false;

    // set FIFO address pointer to configured TX base address
    regWrite(RFM_LORA_FIFO_ADDR_PTR, regRead(RFM_LORA_FIFO_RX_ADDR));

    setMode(RFM_MODE_RXSINGLE);

    // wait until "RxDone" or "RxTimeout"
    do {} while (!rxDone && !rxTimeout);

    if (rxTimeout) {
        return 0;
    }

    return rfmLoRaRxRead(payload, size);
}

size_t rfmLoRaTx(uint8_t *payload, size_t size) {
    size_t len = min(size, RFM_LORA_MSG_SIZE);

    // set FIFO address pointer to configured TX base address
    regWrite(RFM_LORA_FIFO_ADDR_PTR, regRead(RFM_LORA_FIFO_TX_ADDR));

    regWrite(RFM_LORA_PAYLD_LEN, len);

    _rfmSel();
    _rfmTx(RFM_FIFO | 0x80);
    for (size_t i = 0; i < len; i++) {
        _rfmTx(payload[i]);
    }
    _rfmDes();

    // clear "TxDone" interrupt
    regWrite(RFM_LORA_IRQ_FLAGS, 0x08);

    // get "TxDone" on DIO0
    regWrite(RFM_DIO_MAP1, (regRead(RFM_DIO_MAP1) & ~0x80) | 0x40);
    txDone = false;

    setMode(RFM_MODE_TX);

    // wait until "TxDone"
    do {} while (!txDone);

    return len;
}
