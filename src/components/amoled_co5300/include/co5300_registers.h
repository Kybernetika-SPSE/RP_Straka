#ifndef CO5300_REGISTERS_H
#define CO5300_REGISTERS_H


// --- ---- --- ---- --- ---- --- //
//            COMMANDS            //
// --- ---- --- ---- --- ---- --- //

#define CMD_ENTER_SINGLE_SPI 0xFF
#define CMD_ENTER_DUAL_SPI 0x3B
#define CMD_ENTER_QUAD_SPI 0x38
#define CMD_WRITE 0x02
#define CMD_READ 0x03
#define CMD_WRITE_QUAD_SPI_24CLK 0x32 //<- address is sending by 1 wire
#define CMD_WRITE_QUAD_SPI_6CLK 0x12  //<- address is sending by 4 wires
#define CMD_WRITE_DUAL_SPI_24CLK 0x72
#define CMD_WRITE_DUAL_SPI_6CLK 0x52


// --- ---- --- ---- --- ---- --- //
//           ADDRESSES            //
// --- ---- --- ---- --- ---- --- //

#define ADR_SLEEP_OUT 0x11
#define ADR_DISPLAY_ON 0x29
#define ADR_SET_COLUMN_START 0x2A
#define ADR_SET_ROW_START 0x2B
#define ADR_START_PIXEL_WRITE 0x2C
#define ADR_CONTROL_TERING_PIN 0x35
#define ADR_SET_SCAN_DIRECTION 0x36
#define ADR_SET_PIXEL_FORMAT 0x3A
#define ADR_SET_BRIGHTNESS 0x51
#define ADR_WRITE_CTRL 0x53
#define ADR_WRITE_HBM 0x63
#define ADR_CONTROL_SRAM 0xC4 //enable or disable sram
#define ADR_SELECT_PAGE 0xFE


#endif
