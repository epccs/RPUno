#ifndef SpiCmd_H
#define SpiCmd_H

extern void EnableSpi(void);
extern void spi_init(void);

extern volatile uint8_t spi_data;

#endif // SpiCmd_H