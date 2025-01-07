#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>
#include "ioe_tca6408a.h"

static i2c_master_dev_handle_t dev_handle;

void ioe_init(i2c_master_bus_handle_t bus_handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = IOE_I2C_ADDRESS,
        .scl_speed_hz = IOE_I2C_CLOCK,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
}

uint8_t ioe_get_reg(enum ioe_regs reg) {
    uint8_t reg_buf = 0;
    i2c_master_transmit_receive(dev_handle, (uint8_t[]){reg}, 1, &reg_buf, 1, -1);
    return reg_buf;
}

void ioe_set_reg(enum ioe_regs reg, uint8_t byte) {
    i2c_master_transmit(dev_handle, (uint8_t[]){reg, byte}, 2, -1);
}

uint8_t ioe_get_reg_pin(enum ioe_regs reg, enum ioe_pins pin) {
    uint8_t reg_buf = ioe_get_reg(reg);
    reg_buf &= (1 << pin);
    return reg_buf >> pin;
}

void ioe_set_reg_pin(enum ioe_regs reg, enum ioe_pins pin, uint8_t value) {
    uint8_t reg_buf = ioe_get_reg(reg);
    if (value != 0) {
        // Set the bit (make it 1)
        reg_buf |= (1 << pin);
    }
    else {
        // Clear the bit (make it 0)
        reg_buf &= ~(1 << pin);
    }
    ioe_set_reg(reg, reg_buf);
}

uint8_t ioe_read_pin_input(enum ioe_pins pin) {
    if (ioe_get_reg_pin(ioe_reg_direction, pin) == ioe_dir_input) { //Check if pin is input
        return ioe_get_reg_pin(ioe_reg_input, pin);
    }
    else { //If pin isn't input return 255
        return 255;
    }
}
