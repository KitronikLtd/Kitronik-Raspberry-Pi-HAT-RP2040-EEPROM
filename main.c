/*
MIT License

Copyright (c) 2023 Kitronik Ltd 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"

#include "pico/stdlib.h"

#define I2C_BAUDRATE 100000
#define I2C_ADDRESS 0x50
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1

// EEPROM data stored in bytes inside the program
const unsigned char eeprom[] = {
};
const uint16_t eeprom_length = 0;
uint16_t eeprom_address = 0;

unsigned char i2c_first_address_byte;
unsigned char i2c_second_address_byte;
bool i2c_first_address;

// Interrupt handler for emulating EEPROM
void i2c_handler() {

    // Get I2C struct for i2c0
    i2c_hw_t *hw = i2c_get_hw(i2c0);
    // Get interrupt state
    uint32_t interrupt_state = hw->intr_stat;

    if (interrupt_state == 0)
        return;

    // Handling basic I2C functionality
    if (interrupt_state & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) 
        hw->clr_tx_abrt;

    if (interrupt_state & I2C_IC_INTR_STAT_R_START_DET_BITS) 
        hw->clr_start_det;

    if (interrupt_state & I2C_IC_INTR_STAT_R_STOP_DET_BITS) 
        hw->clr_stop_det;

    // Read EEPROM address location from I2C
    // Raspberry Pi tells HAT EEPROM the address it wants to read from
    if (interrupt_state & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {

        assert(hw->status & I2C_IC_STATUS_RFNE_BITS); // Rx FIFO must not be empty

        if (i2c_first_address) {
            
            i2c_first_address_byte = (uint8_t) (hw->data_cmd & 0xff);
            eeprom_address = (uint8_t) i2c_first_address_byte;
        } else {

            i2c_second_address_byte = (uint8_t) (hw->data_cmd & 0xff);
            eeprom_address = (uint8_t) i2c_second_address_byte;
        }

        i2c_first_address = !i2c_first_address;
    }

    // Write EEPROM data from the read address to I2C
    if (interrupt_state & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {

        hw->clr_rd_req;
        assert(hw->status & I2C_IC_STATUS_TFNF_BITS); // Tx FIFO must not be full
        
        if (eeprom_address < sizeof(eeprom))
            hw->data_cmd = (uint8_t) eeprom[eeprom_address++];
    }
}

int main() {

    // Initialise I2C data pin
    gpio_init(I2C_SDA_PIN);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);

    // Initialise I2C clock pin
    gpio_init(I2C_SCL_PIN);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL_PIN);

    // Set I2C0 to be a peripheral device (hat eeprom)
    // Raspberry Pi will be the controler device
    i2c_init(i2c0, I2C_BAUDRATE);
    i2c_set_slave_mode(i2c0, true, I2C_ADDRESS);
    
    // Set our I2C handler for I2C interrupts
    i2c_hw_t *hw = i2c_get_hw(i2c0);
    hw->intr_mask = I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_RD_REQ_BITS | I2C_IC_RAW_INTR_STAT_TX_ABRT_BITS | I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_START_DET_BITS;
    irq_set_exclusive_handler(I2C0_IRQ, i2c_handler);
    irq_set_enabled(I2C0_IRQ, true);

    while (true) {
        tight_loop_contents();
    }

    return 0;
}