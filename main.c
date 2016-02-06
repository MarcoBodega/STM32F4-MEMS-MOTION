
#include "uart.h"
#include <stm32f4xx.h>

// please redefine HSE_VALUE and PLL_M following this link:
//http://stm32f4-discovery.com/2015/01/properly-set-clock-speed-stm32f4xx-devices/

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//#define myUSART USART1
#define myUSART 1
//#define mySPI 1	

uint8_t buf[QUEUE_SIZE];
uint8_t nbyte = 32; //size_t

uint8_t cmd[16];
uint8_t cmdi; //size_t

uint8_t cmd_feedback[32];

void timInit();

// avoid newlib
int strlen2(uint8_t *s);
int strlen(char *s);
int strcmp(const char* s1, const char* s2);
char *strcpy(char *dst, char *src);
char *strcat(char *dest, const char *src) ;
void get_dec_str (uint8_t* str, uint8_t len, uint32_t val);
void itoa(char *buf, int base, int d);


#define Who_Am_I        0x0F
#define Ctrl_Reg1       0x20
#define Ctrl_Reg2       0x21
#define Ctrl_Reg3       0x22
#define HP_filter_reset 0x23
#define Status_Reg      0x27
#define OutX            0x29
#define OutY            0x2B
#define OutZ            0x2D

uint8_t* dataX;
uint8_t* dataY;
uint8_t* dataZ;


uint8_t out[32];
uint8_t tmp[5];



int main(int argc, char* argv[])
{
        
        csInit ();
	spiInit(SPI1);
        
        mySPI_SendData(Ctrl_Reg1, 0x40); //0x40 = 0100000, 0xC0 = 11000000 = 400 Hz output data rate), active
    
	uart_open(myUSART, 9600, 0);
	uint8_t ri;
	cmdi = 0;
        
        timInit();


	// Infinite loop
	while (1)
	{
        }
	uart_close(myUSART);
}


//////////////////////////////////////////////
//////////////////////////////////////////////


void TIM3_IRQHandler (void)
{
    TIM_ClearITPendingBit (TIM3 , TIM_IT_Update );
    
    dataX = mySPI_GetData(OutX);
    dataY = mySPI_GetData(OutY);
    dataZ = mySPI_GetData(OutZ);
    
    out[0] = '\0';
    strcpy(out,"X=");
    tmp[0] = '\0';
    itoa(tmp, 'd', dataX);
    strcat(out, tmp);
    strcat(out, ",Y=");
    tmp[0] = '\0';
    itoa(tmp, 'd', dataY);
    strcat(out, tmp);
    strcat(out, ",Z=");
    tmp[0] = '\0';
    itoa(tmp, 'd', dataZ); 
    strcat(out, tmp);
    strcat(out, "\r\n");
    uart_write(myUSART, out, strlen(out));
}


//////////////////////////////////////////////
//////////////////////////////////////////////


void timInit() {
    
	NVIC_InitTypeDef NVIC_InitStructure ;
	NVIC_InitStructure . NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure . NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure . NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure . NVIC_IRQChannelCmd = ENABLE ;
	NVIC_Init (& NVIC_InitStructure );

        RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM3 , ENABLE );
        
        // configure timer TIM3 for tick every 1ms
	// frequency = 100 hz with 168,000,000 hz system clock
	// 168,000,000/16800 = 10000
	// 10000/1000 = 10
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure ;
	TIM_TimeBaseStructInit (& TIM_TimeBaseStructure );
	TIM_TimeBaseStructure.TIM_Prescaler = 16800 -1; ;    //10KHz
	TIM_TimeBaseStructure.TIM_Period = 1000 -1;        //10Hz
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ;
	TIM_TimeBaseInit (TIM3 , & TIM_TimeBaseStructure );
	TIM_SelectOutputTrigger (TIM3 , TIM_TRGOSource_Update );

        TIM_Cmd (TIM3 , ENABLE ); 	// Enable the tick timer
	TIM_ITConfig (TIM3 , TIM_IT_Update , ENABLE ); 	// Enable Timer Interrupt , enable timer
              

}

//////////////////////////////////////////////
//////////////////////////////////////////////


int strlen2(uint8_t *s)
{
    uint8_t *p = s;
    while (*p != '\0')
        p++;
    return p - s;
}

int strlen(char *s)
{
    char *p = s;
    while (*p != '\0')
        p++;
    return p - s;
}

char *strcpy(char *dst, char *src)
{
    while((*dst++ = *src++)!= '\0')
        ; // <<== Very important!!!
    return dst;
}

int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

char *strcat(char *dest, const char *src) 
{
	char *s1 = dest;
	const char *s2 = src;
	char c;

	do {
		c = *s1++;
	} while (c != '\0');

	s1 -= 2;
	do {
		c = *s2++;
		*++s1 = c;
	} while (c != '\0');
	return dest;
}



/** 
 * Convert the integer D to a string and save the string in BUF. If
 * BASE is equal to 'd', interpret that D is decimal, and if BASE is
 * equal to 'x', interpret that D is hexadecimal.
 */
void itoa(char *buf, int base, int d) {
        char *p = buf;
        char *p1, *p2;
        unsigned long ud = d;
        int divisor = 10;

        /* If %d is specified and D is minus, put `-' in the head.  */
        if (base == 'd' && d < 0) {
                *p++ = '-';
                buf++;
                ud = -d;
        } else if (base == 'x') {
                divisor = 16;
        }

        /* Divide UD by DIVISOR until UD == 0.  */
        do {
                int remainder = ud % divisor;

                *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
        } while (ud /= divisor);

        /* Terminate BUF.  */
        *p = 0;

        /* Reverse BUF.  */
        p1 = buf;
        p2 = p - 1;
        while (p1 < p2) {
                char tmp = *p1;
                *p1 = *p2;
                *p2 = tmp;
                p1++;
                p2--;
        }
}


void get_dec_str (uint8_t* str, uint8_t len, uint32_t val)
{
	uint8_t i;
	for(i=1; i<=len; i++)
	{
		str[len-i] = (uint8_t) ((val % 10UL) + '0');
		val/=10;
	}

	str[i-1] = '\0';
}



#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
    // Infinite loop
    // Use GDB to find out why we're here 
    while (1);
}
#endif



#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
