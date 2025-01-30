#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "pic_bq2562x.h"

static const char *TAG = "BQ256x";

typedef struct {
    uint8_t address;
    uint8_t size;
    uint8_t start;
    uint8_t end;
} Register;

static i2c_master_dev_handle_t dev_handle;

const Register Charge_Current_Limit_ICHG =            { 0x02, 2, 5, 10 };

const Register Input_Current_Limit_IINDPM =           { 0x06, 2, 4, 11 };
const Register Input_Current_Limit_VINDPM =           { 0x08, 2, 5, 13 };

const Register Termination_Control_0_ITERM =          { 0x12, 2, 2, 7 };

const Register Charger_Control_TOPOFF_TMR =           { 0x14, 1, 3, 4 };

const Register Charger_Control_0_EN_CHG =             { 0x16, 1, 5, 5 };
const Register Charger_Control_0_WATCHDOG =           { 0x16, 1, 0, 1 };
const Register Charger_Control_0_EN_HIZ =             { 0x16, 1, 4, 4 };

const Register Charger_Control_2 =                    { 0x18, 1, 0, 7 };
const Register Charger_Control_2_BATFET_CTRL =        { 0x18, 1, 0, 1 };
const Register Charger_Control_2_BATFET_DLY =         { 0x18, 1, 2, 2 };
const Register Charger_Control_2_WVBUS =              { 0x18, 1, 3, 3 };

const Register Charger_Control_3_IBAT_PK =            { 0x19, 1, 6, 7 };

const Register NTC_Control_0 =                        { 0x1a, 1, 0, 7 };
const Register NTC_Control_0_TS_IGNORE =              { 0x1a, 1, 7, 7 };
const Register NTC_Control_0_TS_ISET_WARM =           { 0x1a, 1, 2, 3 };
const Register NTC_Control_0_TS_ISET_COOL =           { 0x1a, 1, 0, 1 };

const Register NTC_Control_1_TS_TH1_TH2_TH3 =         { 0x1b, 1, 5, 7 };
const Register NTC_Control_1_TS_TH4_TH5_TH6 =         { 0x1b, 1, 2, 4 };

const Register NTC_Control_2_TS_ISET_PREWARM =        { 0x1c, 1, 2, 3 };
const Register NTC_Control_2_TS_ISET_PRECOOL =        { 0x1c, 1, 0, 1 };

const Register Charger_Status_0 =                     { 0x1d, 1, 0, 7 };
const Register Charger_Status_0_ADC_DONE =            { 0x1d, 1, 6, 6 };

const Register Charger_Status_1 =                     { 0x1e, 1, 0, 7 };
const Register Charger_Status_1_VBUS_STAT =           { 0x1e, 1, 0, 2 };
const Register Charger_Status_1_CHG_STAT =            { 0x1e, 1, 3, 4 };

const Register FAULT_Status_0 =                       { 0x1f, 1, 0, 7 };
const Register Charger_Flag_0 =                       { 0x20, 1, 0, 7 };
const Register Charger_Flag_1 =                       { 0x21, 1, 0, 7 };
const Register FAULT_Flag_0 =                         { 0x22, 1, 0, 7 };
const Register Charger_Mask_0 =                       { 0x23, 1, 0, 7 };
const Register Charger_Mask_1 =                       { 0x24, 1, 0, 7 };
const Register FAULT_Mask_0 =                         { 0x25, 1, 0, 7 };

const Register ADC_Control =                          { 0x26, 1, 0, 7 };
const Register ADC_Control_ADC_AVG_INIT =             { 0x26, 1, 2, 2 };
const Register ADC_Control_ADC_AVG =                  { 0x26, 1, 3, 3 };
const Register ADC_Control_ADC_SAMPLE =               { 0x26, 1, 4, 5 };
const Register ADC_Control_ADC_RATE =                 { 0x26, 1, 6, 6 };
const Register ADC_Control_ADC_EN =                   { 0x26, 1, 7, 7 };

const Register ADC_Function_Disable_0 =               { 0x27, 1, 0, 7 };

const Register IBUS_ADC =                             { 0x28, 2, 1, 15 };
const Register IBAT_ADC =                             { 0x2a, 2, 2, 15 };
const Register VBUS_ADC =                             { 0x2c, 2, 2, 14 };
const Register VBAT_ADC =                             { 0x30, 2, 1, 12 };
const Register TS_ADC =                               { 0x34, 2, 0, 11 };

const Register Part_Information =                     { 0x38, 1, 0, 7 };
const Register Part_Information_PN =                  { 0x38, 1, 3, 5 };
Register const Part_Information_DEV_REV =             { 0x38, 1, 0, 2 };


static uint16_t BQ2562x_readReg(Register reg) {
    uint16_t data = 0;
    i2c_master_transmit_receive(dev_handle, (uint8_t[]){reg.address}, 1, (uint8_t *)(&data), reg.size, -1);
    int l_shift = (((reg.size * 8) - 1) - reg.end);
    data <<= l_shift;
    data >>= l_shift + reg.start;
    return data;
}

static void BQ2562x_writeReg(Register reg, uint16_t data) {
    uint8_t l_bit = (reg.size * 8) - 1;
    uint16_t bytes = 0;
    bytes = BQ2562x_readReg((Register){reg.address, reg.size, 0, l_bit});

    uint8_t bits = reg.end - reg.start + 1;
    uint16_t mask = ((1 << bits) - 1) << reg.start;
    bytes = (bytes & ~mask) | ((data << reg.start) & mask);

    i2c_master_transmit(dev_handle, (uint8_t[]){reg.address, bytes}, reg.size+1, -1);
}

static float BQ2562x_map(uint16_t raw, float step, uint16_t min, uint16_t max) {
    return ((min && max) && (raw >= min && raw <= max)) ? ((max - raw + 1) * (-1) * step) : (raw * step);
}

// public functions

void BQ2562x_init(i2c_master_bus_handle_t bus_handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BQ2562X_I2C_ADDRESS,
        .scl_speed_hz = BQ2562X_I2C_CLOCK
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    ESP_LOGI(TAG, "Init done!");
}

bool BQ2562x_getWD() {
    return BQ2562x_readReg(Charger_Control_0_WATCHDOG);
}

uint16_t BQ2562x_getVBUS()
{
    uint16_t value = BQ2562x_readReg(VBUS_ADC);
    return round(BQ2562x_map(value, 3.97f, 0x0, 0xffff));
}

int16_t BQ2562x_getIBUS()
{
    uint16_t value = BQ2562x_readReg(IBUS_ADC);
    return round(BQ2562x_map(value, 2.0f, 0x7830, 0x7fff));
}

uint16_t BQ2562x_getVBAT()
{
    uint16_t value = BQ2562x_readReg(VBAT_ADC);
    return round(BQ2562x_map(value, 1.99f, 0x0, 0xffff));
}

int16_t BQ2562x_getIBAT()
{
    static uint16_t invalid = 0x2000;
    uint16_t value = BQ2562x_readReg(IBAT_ADC);
    if (value != invalid)
    {
        return round(BQ2562x_map(value, 4.0f, 0x38ad, 0x3fff));
    }
    return invalid;
}

bool BQ2562x_getADCDone()
{
    bool value = BQ2562x_readReg(Charger_Status_0_ADC_DONE);
    return value;
}

bool BQ2562x_getTSEnabled()
{
    bool tsIgnore = false, tsDis = false;
    tsIgnore = BQ2562x_readReg(NTC_Control_0_TS_IGNORE);
    Register reg = ADC_Function_Disable_0;
    reg.start = reg.end = (uint8_t)(BQ2562X_ADC_TS);
    tsDis = BQ2562x_readReg(reg);
    return (!tsIgnore) && (!tsDis);
}

float BQ2562x_getTSBias()
{
    uint16_t value = BQ2562x_readReg(TS_ADC);
    return BQ2562x_map(value, 0.0961f, 0x0, 0xffff) / 100.0f;
}

BQ2562x_VBUSStat BQ2562x_getVBUSStat()
{
    uint8_t value = BQ2562x_readReg(Charger_Status_1_VBUS_STAT);
    return (value == (uint8_t)(BQ2562X_VBUS_ADAPTER)) ? BQ2562X_VBUS_ADAPTER : BQ2562X_VBUS_NONE;
}

BQ2562x_ChargeStat BQ2562x_getChargeStat()
{
    uint8_t value = BQ2562x_readReg(Charger_Status_1_CHG_STAT);
    BQ2562x_ChargeStat stat = 0;
    switch (value)
    {
    case (uint8_t)(BQ2562X_CHARGE_STAT_TRICKLE):
        stat = BQ2562X_CHARGE_STAT_TRICKLE;
        break;
    case (uint8_t)(BQ2562X_CHARGE_STAT_TAPER):
        stat = BQ2562X_CHARGE_STAT_TAPER;
        break;
    case (uint8_t)(BQ2562X_CHARGE_STAT_TOPOFF):
        stat = BQ2562X_CHARGE_STAT_TOPOFF;
        break;
    case (uint8_t)(BQ2562X_CHARGE_STAT_TERMINATED):
    default:
        stat = BQ2562X_CHARGE_STAT_TERMINATED;
        break;
    }
    return stat;
}

uint8_t BQ2562x_getPartInformation()
{
    return BQ2562x_readReg(Part_Information);
}

uint8_t BQ2562x_readCurentLimit()
{
    return BQ2562x_readReg(Charge_Current_Limit_ICHG);
}

// TODO: Tady jsem skončil...

void BQ2562x_setWD(BQ2562x_WatchdogTimer timer)
{
    BQ2562x_writeReg(Charger_Control_0_WATCHDOG, (uint8_t)(timer));
}

void BQ2562x_enableCharging(bool enable)
{
    BQ2562x_writeReg(Charger_Control_0_EN_CHG, enable);
}

void BQ2562x_enableTS(bool enable)
{
    BQ2562x_writeReg(NTC_Control_0_TS_IGNORE, !enable);  // &&
    BQ2562x_enableADC(BQ2562X_ADC_TS, enable);
}

void BQ2562x_enableHIZ(bool enable)
{
    BQ2562x_writeReg(Charger_Control_0_EN_HIZ, enable);
}

void BQ2562x_enableInterrupts(bool enable)
{
    uint8_t value = enable ? 0 : 0xFF;
    BQ2562x_writeReg(Charger_Mask_0, value);
    BQ2562x_writeReg(Charger_Mask_1, value);
    BQ2562x_writeReg(FAULT_Mask_0, value);
}

void BQ2562x_enableInterrupt(BQ2562x_Interrupt interrupt, bool enable)
{
    if (interrupt == BQ2562x_Interrupt_VBUS)
    {
        Register reg = Charger_Mask_1;
        reg.start = reg.end = (uint8_t)(0);
        BQ2562x_writeReg(reg, !enable);
    }
    else if (interrupt == BQ2562x_Interrupt_TS)
    {
        Register reg = FAULT_Mask_0;
        reg.start = reg.end = (uint8_t)(0);
        return BQ2562x_writeReg(reg, !enable);
    } // supports only a few interrupts for now
}

void BQ2562x_enableWVBUS(bool enable)
{
    BQ2562x_writeReg(Charger_Control_2_WVBUS, enable);
}

void BQ2562x_enableADC(BQ2562x_Adc adc, bool enable)
{
    Register reg = ADC_Function_Disable_0;
    reg.start = reg.end = (uint8_t)(adc);
    return BQ2562x_writeReg(reg, !enable);
}

void BQ2562x_setChargeCurrent(uint16_t current)
{
    if (current >= BQ2562X_MIN_CHARGING_CURRENT && current <= BQ2562X_MAX_CHARGING_CURRENT)
    {
        uint16_t value = round(BQ2562x_map(current, 1/40.0f, 0x0, 0xffff));
        BQ2562x_writeReg(Charge_Current_Limit_ICHG, value);
    }
}

void BQ2562x_setBATFETControl(BQ2562x_BATFETControl control)
{
    uint8_t value = (uint8_t)(control);
    BQ2562x_writeReg(Charger_Control_2_BATFET_CTRL, value);
}

void BQ2562x_setBATFETDelay(BQ2562x_BATFETDelay delay)
{
    uint8_t value = (uint8_t)(delay);
    BQ2562x_writeReg(Charger_Control_2_BATFET_DLY, value);
}

void BQ2562x_setVINDPM(uint16_t voltage)
{
    if (voltage >= BQ2562X_MIN_VINDPM_VOLTAGE && voltage <= BQ2562X_MAX_VINDPM_VOLTAGE)
    {
        uint16_t value = round(BQ2562x_map(voltage, 1/40.0f, 0x0, 0xffff));
        BQ2562x_writeReg(Input_Current_Limit_VINDPM, value);
    }
}

void BQ2562x_setIINDPM(uint16_t current)
{
    if (current >= BQ2562X_MIN_IINDPM_CURRENT && current <= BQ2562X_MAX_IINDPM_CURRENT)
    {
        uint16_t value = round(BQ2562x_map(current, 1/20.0f, 0x0, 0xffff));
        BQ2562x_writeReg(Input_Current_Limit_IINDPM, value);
    }
}

void BQ2562x_setITERM(uint16_t current)
{
    if (current >= BQ2562X_MIN_ITERM_CURRENT && current <= BQ2562X_MAX_ITERM_CURRENT)
    {
        uint16_t value = round(BQ2562x_map(current, 1/5.0f, 0x0, 0xffff));
        BQ2562x_writeReg(Termination_Control_0_ITERM, value);
    }
}

void BQ2562x_setTopOff(BQ2562x_TopOffTimer timer)
{
    uint8_t value = (uint8_t)(timer);
    BQ2562x_writeReg(Charger_Control_TOPOFF_TMR, value);
}

void BQ2562x_setIbatPk(BQ2562x_IBATPkLimit limit)
{
    uint8_t value = (uint8_t)(limit);
    BQ2562x_writeReg(Charger_Control_3_IBAT_PK, value);
}

void BQ2562x_setTH123(BQ2562x_TH123Setting setting)
{
    BQ2562x_writeReg(NTC_Control_1_TS_TH1_TH2_TH3, (uint8_t)(setting));
}

void BQ2562x_setTH456(BQ2562x_TH456Setting setting)
{
    BQ2562x_writeReg(NTC_Control_1_TS_TH4_TH5_TH6, (uint8_t)(setting));
}

void BQ2562x_setTempIset(BQ2562x_TempPoint point, BQ2562x_TempIset iset)
{
    Register reg;

    switch (point)
    {
    case BQ2562x_Temp_Prewarm:
        reg = NTC_Control_2_TS_ISET_PREWARM;
        break;
    case BQ2562x_Temp_Precool:
        reg = NTC_Control_2_TS_ISET_PRECOOL;
        break;
    case BQ2562x_Temp_Warm:
        reg = NTC_Control_0_TS_ISET_WARM;
        break;
    case BQ2562x_Temp_Cool:
        reg = NTC_Control_0_TS_ISET_COOL;
        break;
    default:
        return;
        break;
    }

    BQ2562x_writeReg(reg, (uint8_t)(iset));
}

void BQ2562x_setupADC_defaults()
{
    uint8_t value = true << ADC_Control_ADC_EN.start;
    if (value)
    {
        value |= (uint8_t)(BQ2562X_ADC_RATE_CONTINUOUS) << ADC_Control_ADC_RATE.start;
        value |= (uint8_t)(BQ2562X_ADC_SAMPLING_12_BITS) << ADC_Control_ADC_SAMPLE.start;
        value |= (uint8_t)(BQ2562X_ADC_AVERAGE_SINGLE) << ADC_Control_ADC_AVG.start;
        value |= (uint8_t)(BQ2562X_ADC_AVERAGE_INIT_EXISTING) << ADC_Control_ADC_AVG_INIT.start;
    }
    BQ2562x_writeReg(ADC_Control, value);
}
