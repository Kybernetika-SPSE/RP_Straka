#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>
#include "ioe_tca6408a.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep
#include "freertos/task.h"
#include "esp_log.h"


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

// Pin on the IOE, pointer to callback
void ioe_add_pin_interrupt(uint8_t pin, void* callback) {
    assert(pin < IOE_NUM_PINS);

    ioe_irq_callbacks[pin] = callback;
}

// Debugging functions
static void ioe_test_interrupt(void* arg) {
    ESP_LOGI("IOE", "Interrupt callback");
    uint8_t reg_input = ioe_get_reg(ioe_reg_input);
    uint8_t reg_direction = ioe_get_reg(ioe_reg_direction);

    for (int i = 0; i < IOE_NUM_PINS; i++) {
        if (((reg_direction>>i) & ioe_dir_input) && ((reg_input>>i) & 1)) { // If pin is 1 and is input
            ESP_LOGI("IOE", "Interrupt callback on pin %d", i);
            if (ioe_irq_callbacks[i] != NULL){
                ioe_irq_callbacks[i]();
            }
        }
    }
    vTaskDelete(NULL);
}

// Will this work???
//static void IRAM_ATTR ioe_interrupt_callback(void* arg){
static void ioe_interrupt_callback(void* arg){
    xTaskCreate(ioe_test_interrupt, "ioe_test_interrupt", 2048, arg, 10, NULL);
}

// Actual pin of the SoC
void ioe_init_interrupt(uint8_t pin) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = 1ULL<<pin,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = true
    };

    gpio_config(&io_conf);
    gpio_isr_handler_add(pin, ioe_interrupt_callback, NULL);
}
