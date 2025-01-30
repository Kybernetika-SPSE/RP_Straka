#ifndef PIC_BQ2562X_H
#define PIC_BQ2562X_H
#include "driver/i2c_types.h"
#include <complex.h>
#include <stdint.h>
#include <stdbool.h>

// I2C
#define BQ2562X_I2C_ADDRESS 0x6a
#define BQ2562X_I2C_CLOCK 100000

// Constants
#define BQ2562X_MIN_CHARGING_CURRENT 40
#define BQ2562X_MAX_CHARGING_CURRENT 2000

#define BQ2562X_MIN_VINDPM_VOLTAGE 4600
#define BQ2562X_MAX_VINDPM_VOLTAGE 16800
#define BQ2562X_RESET_VINDPM_VOLTAGE 4600

#define BQ2562X_MIN_IINDPM_CURRENT 100
#define BQ2562X_MAX_IINDPM_CURRENT 3200

#define BQ2562X_MIN_ITERM_CURRENT 5
#define BQ2562X_MAX_ITERM_CURRENT 310

// Enumerations
typedef enum {
    BQ2562X_VBUS_NONE = 0b000,
    BQ2562X_VBUS_ADAPTER = 0b100,
} BQ2562x_VBUSStat;

typedef enum {
    BQ2562X_CHARGE_STAT_TERMINATED = 0x00,
    BQ2562X_CHARGE_STAT_TRICKLE = 0x01,
    BQ2562X_CHARGE_STAT_TAPER = 0x02,
    BQ2562X_CHARGE_STAT_TOPOFF = 0x03,
} BQ2562x_ChargeStat;

typedef enum {
    BQ2562X_WATCHDOG_DISABLED = 0x00,
    BQ2562X_WATCHDOG_50S = 0x01,
    BQ2562X_WATCHDOG_100S = 0x02,
    BQ2562X_WATCHDOG_200S = 0x03,
} BQ2562x_WatchdogTimer;

typedef enum {
    BQ2562X_ADC_RATE_CONTINUOUS = 0x00,
    BQ2562X_ADC_RATE_ONESHOT = 0x01,
} BQ2562x_ADCRate;

typedef enum {
    BQ2562X_ADC_SAMPLING_12_BITS = 0x00,
    BQ2562X_ADC_SAMPLING_11_BITS = 0x01,
    BQ2562X_ADC_SAMPLING_10_BITS = 0x02,
    BQ2562X_ADC_SAMPLING_9_BITS = 0x03,
} BQ2562x_ADCSampling;

typedef enum {
    BQ2562X_ADC_AVERAGE_SINGLE = 0x00,
    BQ2562X_ADC_AVERAGE_RUNNING = 0x01,
} BQ2562x_ADCAverage;

typedef enum {
    BQ2562X_ADC_AVERAGE_INIT_EXISTING = 0x00,
    BQ2562X_ADC_AVERAGE_INIT_NEW = 0x01,
} BQ2562x_ADCAverageInit;

typedef enum {
    BQ2562X_BATFET_CTRL_NORMAL = 0x00,
    BQ2562X_BATFET_CTRL_SHUTDOWN = 0x01,
    BQ2562X_BATFET_CTRL_SHIP_MODE = 0x02,
    BQ2562X_BATFET_CTRL_SYSTEM_RESET = 0x03,
} BQ2562x_BATFETControl;

typedef enum {
    BQ2562X_BATFET_DELAY_20MS = 0x00,
    BQ2562X_BATFET_DELAY_10S = 0x01,
} BQ2562x_BATFETDelay;

typedef enum {
    BQ2562X_ADC_IBUS = 7,
    BQ2562X_ADC_IBAT = 6,
    BQ2562X_ADC_VBUS = 5,
    BQ2562X_ADC_VBAT = 4,
    BQ2562X_ADC_VSYS = 3,
    BQ2562X_ADC_TS = 2,
    BQ2562X_ADC_TDIE = 1,
    BQ2562X_ADC_VPMID = 0,
} BQ2562x_Adc;


typedef enum {
    BQ2562x_Interrupt_VBUS,
    BQ2562x_Interrupt_TS
} BQ2562x_Interrupt;

typedef enum {
    BQ2562x_Timer_Disabled = 0x00,
    BQ2562x_Timer17Min = 0x01,
    BQ2562x_Timer35Min = 0x02,
    BQ2562x_Timer52Min = 0x03
} BQ2562x_TopOffTimer;

typedef enum {
    BQ2562x_IBATPk_Limit1_5A = 0x00,
    BQ2562x_IBATPk_Limit3A = 0x01,
    BQ2562x_IBATPk_Limit6A = 0x02,
    BQ2562x_IBATPk_Limit12A = 0x03
} BQ2562x_IBATPkLimit;

typedef enum {
    BQ2562x_TH1_0_TH2_5_TH3_15 = 0x00,
} BQ2562x_TH123Setting;

typedef enum {
    BQ2562x_TH4_35_TH5_40_TH6_50 = 0x04,
} BQ2562x_TH456Setting;

typedef enum {
    BQ2562x_Temp_Cold,
    BQ2562x_Temp_Cool,
    BQ2562x_Temp_Precool,
    BQ2562x_Temp_Normal,
    BQ2562x_Temp_Prewarm,
    BQ2562x_Temp_Warm,
    BQ2562x_Temp_Hot
} BQ2562x_TempPoint;

typedef enum {
    BQ2562x_TempIset_Suspend = 0x00,
    BQ2562x_TempIset_Ichg20 = 0x01,
    BQ2562x_TempIset_Ichg40 = 0x02,
    BQ2562x_TempIset_NoChange = 0x03
} BQ2562x_TempIset;


// Function declarations
void BQ2562x_init(i2c_master_bus_handle_t bus_handle);
void BQ2562x_setupADC_defaults();

bool BQ2562x_getWD();
uint16_t BQ2562x_getVBUS();
int16_t BQ2562x_getIBUS();
uint16_t BQ2562x_getVBAT();
int16_t BQ2562x_getIBAT();
bool BQ2562x_getADCDone();
bool BQ2562x_getTSEnabled();
float BQ2562x_getTSBias();
BQ2562x_VBUSStat BQ2562x_getVBUSStat();
BQ2562x_ChargeStat BQ2562x_getChargeStat();
uint8_t BQ2562x_getPartInformation();
uint8_t BQ2562x_readCurentLimit();

void BQ2562x_setWD(BQ2562x_WatchdogTimer timer);
void BQ2562x_enableCharging(bool enable);
void BQ2562x_enableTS(bool enable);
void BQ2562x_enableHIZ(bool enable);
void BQ2562x_enableInterrupts(bool enable);
void BQ2562x_enableInterrupt(BQ2562x_Interrupt interrupt, bool enable);
void BQ2562x_enableWVBUS(bool enable);
void BQ2562x_enableADC(BQ2562x_Adc adc, bool enable);
void BQ2562x_setChargeCurrent(uint16_t current);
void BQ2562x_setBATFETControl(BQ2562x_BATFETControl control);
void BQ2562x_setBATFETDelay(BQ2562x_BATFETDelay delay);
void BQ2562x_setVINDPM(uint16_t voltage);
void BQ2562x_setIINDPM(uint16_t current);
void BQ2562x_setITERM(uint16_t current);
void BQ2562x_setTopOff(BQ2562x_TopOffTimer timer);
void BQ2562x_setIbatPk(BQ2562x_IBATPkLimit limit);
void BQ2562x_setTempIset(BQ2562x_TempPoint point, BQ2562x_TempIset iset);

#endif
