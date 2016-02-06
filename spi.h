#include <stm32f4xx_spi.h>

void csInit ();
void spiInit ( SPI_TypeDef *SPIx);
int spiReadWrite ( SPI_TypeDef * SPIx, uint8_t *rbuf, const uint8_t *tbuf, int cnt, uint16_t speed);
int spiReadWrite16 ( SPI_TypeDef * SPIx, uint16_t *rbuf, const uint16_t *tbuf, int cnt, uint16_t speed );

void spiTest2();
uint8_t SPI1_send(uint8_t data);

void spiTestMems ();
void mySPI_SendData(uint8_t adress, uint8_t data);
uint8_t mySPI_GetData(uint8_t adress);

#ifdef TEST_SPI
void spiTest();
#endif
