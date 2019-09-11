#ifndef EINK_H
#define EINK_H

#include <stdint.h>

#define EINK_WIDTH       200
#define EINK_HEIGHT      200

#define EINK_RST_PIN  0
#define EINK_DC_PIN   1
#define EINK_CS_PIN   3

void EINK_Init(void);
void EINK_Clear(void);
void EINK_Display(uint8_t *Image);
void EINK_Sleep(void);

//need to implement
extern void EINK_delay_ms(int ms);
extern void EINK_set_pin_state(unsigned char pin, char state);
extern char EINK_read_busy(void);
extern void EINK_spi_write(unsigned char data);

#endif
