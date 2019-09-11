#include "eink.h"

#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF


const unsigned char lut_full_update[] = {
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

const unsigned char lut_partial_update[] = {
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void EINK_Reset(void)
{
	EINK_set_pin_state(EINK_RST_PIN, 1);
    EINK_delay_ms(200);
    EINK_set_pin_state(EINK_RST_PIN, 0);
    EINK_delay_ms(200);
    EINK_set_pin_state(EINK_RST_PIN, 1);
    EINK_delay_ms(200);
}

static void EINK_SendCommand(unsigned char Reg)
{
	EINK_set_pin_state(EINK_DC_PIN, 0);
    EINK_set_pin_state(EINK_CS_PIN, 0);
    EINK_spi_write(Reg);
    EINK_set_pin_state(EINK_CS_PIN, 1);
}

static void EINK_SendData(unsigned char Data)
{
    EINK_set_pin_state(EINK_DC_PIN, 1);
    EINK_set_pin_state(EINK_CS_PIN, 0);
    EINK_spi_write(Data);
    EINK_set_pin_state(EINK_CS_PIN, 1);
}


void EINK_WaitUntilIdle(void)
{
    while(EINK_read_busy() == 1) {      //LOW: idle, HIGH: busy
        EINK_delay_ms(100);
    }
}

static void EINK_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
    EINK_SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
    EINK_SendData((Xstart >> 3) & 0xFF);
    EINK_SendData((Xend >> 3) & 0xFF);
    EINK_SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
    EINK_SendData(Ystart & 0xFF);
    EINK_SendData((Ystart >> 8) & 0xFF);
    EINK_SendData(Yend & 0xFF);
    EINK_SendData((Yend >> 8) & 0xFF);
}

static void EINK_SetCursor(uint16_t Xstart, uint16_t Ystart)
{
    EINK_SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    EINK_SendData((Xstart >> 3) & 0xFF);
    EINK_SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    EINK_SendData(Ystart & 0xFF);
    EINK_SendData((Ystart >> 8) & 0xFF);

}

static void EINK_TurnOnDisplay(void)
{
    EINK_SendCommand(DISPLAY_UPDATE_CONTROL_2);
    EINK_SendData(0xC4);
    EINK_SendCommand(MASTER_ACTIVATION);
    EINK_SendCommand(TERMINATE_FRAME_READ_WRITE);
    EINK_WaitUntilIdle();
}

void EINK_Init(void)
{
    EINK_Reset();
    EINK_SendCommand(DRIVER_OUTPUT_CONTROL);
    EINK_SendData((EINK_HEIGHT - 1) & 0xFF);
    EINK_SendData(((EINK_HEIGHT - 1) >> 8) & 0xFF);
    EINK_SendData(0x00);                     // GD = 0; SM = 0; TB = 0;
    EINK_SendCommand(BOOSTER_SOFT_START_CONTROL);
    EINK_SendData(0xD7);
    EINK_SendData(0xD6);
    EINK_SendData(0x9D);
    EINK_SendCommand(WRITE_VCOM_REGISTER);
    EINK_SendData(0xA8);                     // VCOM 7C
    EINK_SendCommand(SET_DUMMY_LINE_PERIOD);
    EINK_SendData(0x1A);                     // 4 dummy lines per gate
    EINK_SendCommand(SET_GATE_TIME);
    EINK_SendData(0x08);                     // 2us per line
    EINK_SendCommand(DATA_ENTRY_MODE_SETTING);
    EINK_SendData(0x03);
    EINK_SendCommand(WRITE_LUT_REGISTER);
    for (uint16_t i = 0; i < 30; i++) {
        EINK_SendData(lut_full_update[i]);
    }
}

void EINK_Clear(void)
{
    uint16_t Width, Height;
    Width = (EINK_WIDTH % 8 == 0)? (EINK_WIDTH / 8 ): (EINK_WIDTH / 8 + 1);
    Height = EINK_HEIGHT;
    EINK_SetWindows(0, 0, EINK_WIDTH, EINK_HEIGHT);
    for (uint16_t j = 0; j < Height; j++) {
        EINK_SetCursor(0, j);
        EINK_SendCommand(WRITE_RAM);
        for (uint16_t i = 0; i < Width; i++) {
            EINK_SendData(0XFF);
        }
    }
    EINK_TurnOnDisplay();
}


void EINK_Display(uint8_t *Image)
{
    uint16_t Width, Height;
    Width = (EINK_WIDTH % 8 == 0)? (EINK_WIDTH / 8 ): (EINK_WIDTH / 8 + 1);
    Height = EINK_HEIGHT;
    uint32_t Addr = 0;
    EINK_SetWindows(0, 0, EINK_WIDTH, EINK_HEIGHT);
    for (uint16_t j = 0; j < Height; j++) {
        EINK_SetCursor(0, j);
        EINK_SendCommand(WRITE_RAM);
        for (uint16_t i = 0; i < Width; i++) {
            Addr = i + j * Width;
            EINK_SendData(Image[Addr]);
        }
    }
    EINK_TurnOnDisplay();
}


void EINK_Sleep(void)
{
    EINK_SendCommand(DEEP_SLEEP_MODE);
    EINK_SendData(0x01);
}
