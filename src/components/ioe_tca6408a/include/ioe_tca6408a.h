#ifndef IOE_TCA6406_H
#define IOE_TCA6406_H
#include "driver/i2c_types.h"
#include <complex.h>
#include <stdint.h>

#define IOE_I2C_ADDRESS 0x21
#define IOE_I2C_CLOCK 100000

enum ioe_regs {
    ioe_reg_input,
    ioe_reg_output,
    ioe_reg_inverse,
    ioe_reg_direction
};

enum ioe_direction{
    ioe_dir_output,
    ioe_dir_input
};

// Move pins definition to separate board component
enum ioe_pins {
    ioe_imu_int,
    ioe_dp_res,
    ioe_nfc_int,
    ioe_u1v8_en,
    ioe_oxy_int,
    ioe_lvl_shift_en,
    ioe_vic_int,
    ioe_pic_int
};

void ioe_init(i2c_master_bus_handle_t bus_handle);
uint8_t ioe_get_reg(enum ioe_regs reg);
void ioe_set_reg(enum ioe_regs reg, uint8_t byte);
uint8_t ioe_get_reg_pin(enum ioe_regs reg, enum ioe_pins pin);
void ioe_set_reg_pin(enum ioe_regs reg, enum ioe_pins pin, uint8_t value);
uint8_t ioe_read_pin_input(enum ioe_pins pin);
#endif
