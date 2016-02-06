
#include <stm32f4xx.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <misc.h>
#include "spi.h"


#define SPI_SLOW SPI_BaudRatePrescaler_64
#define SPI_MEDIUM SPI_BaudRatePrescaler_8
#define SPI_FAST SPI_BaudRatePrescaler_2


/////////////////////////////////////////
// from Discover STM32 microcontrollers..
//////////////////////////////////////////

void csInit ()
{
    GPIO_InitTypeDef GPIO_InitStructure ;
    GPIO_StructInit (& GPIO_InitStructure );

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}


void spiInit ( SPI_TypeDef *SPIx)
{
    SPI_InitTypeDef SPI_InitStructure ;
    GPIO_InitTypeDef GPIO_InitStructure ;
    
    GPIO_StructInit (& GPIO_InitStructure );
    SPI_StructInit (& SPI_InitStructure );
    
    if (SPIx == SPI1) {
        
        // Enable clock for GPIOA
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);    

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);    */
        
        GPIOE->BSRRL |= GPIO_Pin_3; // set PE3 high

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex ;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master ;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b ;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //SPI_CPOL_Low ;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge ;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

	//SPI_DataSizeConfig (SPIx , SPI_DataSize_16b );

	SPI_Init (SPIx , & SPI_InitStructure );
	SPI_Cmd (SPIx , ENABLE );

    } 
    else {
        return ;
    }
      
}


int spiReadWrite ( SPI_TypeDef * SPIx , uint8_t *rbuf, const uint8_t *tbuf , int cnt , uint16_t speed)
{
    int i;
    SPIx ->CR1 = (SPIx ->CR1 & ~ SPI_BaudRatePrescaler_256 ) | speed;  //speeds [ speed ];
    for (i = 0; i < cnt; i++){
        if (tbuf) {
            SPI_I2S_SendData (SPIx , *tbuf ++);
        } 
        else {
            SPI_I2S_SendData (SPIx , 0xff);
        }
        while ( SPI_I2S_GetFlagStatus (SPIx , SPI_I2S_FLAG_RXNE ) == RESET);
        if (rbuf) {
            *rbuf ++ = SPI_I2S_ReceiveData (SPIx);
        } 
        else     {
            SPI_I2S_ReceiveData (SPIx);
        }
    }
    return i;
}



int spiReadWrite16 ( SPI_TypeDef * SPIx, uint16_t *rbuf, const uint16_t *tbuf, int cnt, uint16_t speed )
{
    //TODO
}







//////////////////////////////////////////////
// https://github.com/g4lvanix/STM32F4-examples
/////////////////////////////////////////////
uint8_t received_val = 0;

void spiTest2() {
    
    csInit ();
    spiInit(SPI1);

    while(1){
            
            GPIOE->BSRRH |= GPIO_Pin_3; // set PE3 (CS) low
            SPI1_send(0xAA);  // transmit data
            received_val = SPI1_send(0x00); // transmit dummy byte and receive data
            GPIOE->BSRRL |= GPIO_Pin_3; // set PE3 (CS) high
    }
}


uint8_t SPI1_send(uint8_t data){

	SPI1->DR = data; // write data to be transmitted to the SPI data register
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
	while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
	while( SPI1->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
	return SPI1->DR; // return received data from SPI data register
}


/////////////////////////////////////////////
// http://www.lxtronic.com/index.php/basic-spi-simple-read-write
/////////////////////////////////////////////
void spiTestMems () 
{

	csInit ();
	spiInit(SPI1);

	int8_t data;

	mySPI_SendData(0x20, 0xC0); //LIS302D Config

	while(1)
 	{
		data = mySPI_GetData(0x29);
	}

}


void mySPI_SendData(uint8_t adress, uint8_t data)
{
 
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	SPI_I2S_SendData(SPI1, adress);
	
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	SPI_I2S_ReceiveData(SPI1);
 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	SPI_I2S_SendData(SPI1, data);
	
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	SPI_I2S_ReceiveData(SPI1);
 
	GPIO_SetBits(GPIOE, GPIO_Pin_3);
}

uint8_t mySPI_GetData(uint8_t adress)
{
 
	GPIO_ResetBits(GPIOE, GPIO_Pin_3); 
 
	adress = 0x80 | adress;
 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	SPI_I2S_SendData(SPI1, adress);
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	SPI_I2S_ReceiveData(SPI1); //Clear RXNE bit
 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	SPI_I2S_SendData(SPI1, 0x00); //Dummy byte to generate clock
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
 
	GPIO_SetBits(GPIOE, GPIO_Pin_3);
	return  SPI_I2S_ReceiveData(SPI1);
}





#ifdef TEST_SPI
/////////////////////////////////////////
// from Discover STM32 microcontrollers..
//////////////////////////////////////////

uint8_t txbuf [4], rxbuf [4];
uint16_t txbuf16 [4], rxbuf16 [4];

void spiTest() {
    int i, j;
    csInit (); // Initialize chip select PC03
    spiInit (SPI1);
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++)
            txbuf [j] = i*4 + j;
        GPIO_WriteBit (GPIOE , GPIO_Pin_3 , 0);
        spiReadWrite (SPI1 , rxbuf , txbuf , 4, SPI_SLOW );
        GPIO_WriteBit (GPIOE , GPIO_Pin_3 , 1);
        for (j = 0; j < 4; j++)
            if (rxbuf [j] != txbuf [j])
                assert_failed (__FILE__ , __LINE__ );
    }
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++)
            txbuf16 [j] = i*4 + j + (i << 8);
        GPIO_WriteBit (GPIOE , GPIO_Pin_3 , 0);
        spiReadWrite16 (SPI1 , rxbuf16 , txbuf16 , 4, SPI_SLOW );
        GPIO_WriteBit (GPIOE , GPIO_Pin_3 , 1);
        for (j = 0; j < 4; j++)
            if ( rxbuf16 [j] != txbuf16 [j])
                assert_failed (__FILE__ , __LINE__ );
    }   
    
}
#endif
