#include "ch32fun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CH32V003_UART_IMPLEMENTATION
#include "ch32v003_uart.h"
#include "i2c_slave.h"

// The I2C slave library uses a one byte address so you can extend the size of this array up to 256 registers
// note that the register set is modified by interrupts, to prevent the compiler from accidently optimizing stuff
// away make sure to declare the register array volatile

uint8_t echo = 1;

volatile uint8_t edid_data[256] = {
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x05, 0xe3, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
0x00, 0x17, 0x01, 0x03, 0x80, 0x46, 0x27, 0x78, 0x0a, 0x63, 0x9d, 0xa1, 0x54, 0x52, 0x9e, 0x26,
0x0a, 0x47, 0x4a, 0xa1, 0x08, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x66, 0x21, 0x50, 0xb0, 0x51, 0x00, 0x1b, 0x30, 0x40, 0x70,
0x36, 0x00, 0xb9, 0x88, 0x21, 0x00, 0x00, 0x1e, 0xa9, 0x1a, 0x00, 0xa0, 0x50, 0x00, 0x16, 0x30,
0x30, 0x20, 0x37, 0x00, 0xb9, 0x88, 0x21, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x41,
0x4f, 0x43, 0x20, 0x4c, 0x43, 0x44, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd,
0x00, 0x37, 0x47, 0x0f, 0x46, 0x0f, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x96,

0x02, 0x03, 0x22, 0x71, 0x4f, 0x05, 0x04, 0x03, 0x02, 0x01, 0x90, 0x07, 0x06, 0x11, 0x12, 0x15,
0x16, 0x1f, 0x14, 0x13, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0c, 0x00,
0x10, 0x00, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0xb9, 0x88,
0x21, 0x00, 0x00, 0x1e, 0x8c, 0x0a, 0xa0, 0x14, 0x51, 0xf0, 0x16, 0x00, 0x26, 0x7c, 0x43, 0x00,
0xb9, 0x88, 0x21, 0x00, 0x00, 0x98, 0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e,
0x96, 0x00, 0xb9, 0x88, 0x21, 0x00, 0x00, 0x18, 0x01, 0x1d, 0x80, 0x18, 0x71, 0x1c, 0x16, 0x20,
0x58, 0x2c, 0x25, 0x00, 0xb9, 0x88, 0x21, 0x00, 0x00, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37
};

volatile uint8_t blink = 0;

void onRead(uint8_t reg) {
    //blink = 5;
}

uint16_t xtou16(const char *str)
{
    uint16_t res = 0;
    char c;

    while ((c = *str++)) { 
        char v = ((c & 0xF) + (c >> 6)) | ((c >> 3) & 0x8);
        res = (res << 4) | (uint16_t) v;
    }

    return res;
}

void cmd_get(char *str) {
   str[3] = 0;
   uint16_t offset = xtou16(str);
   if (offset < 256 - 8 + 1) {
       for (uint8_t i = 0; i < 8; i++) {
          UART_printf("%02X ", edid_data[offset + i]);
       }
       UART_putc('\n');
   } else {
       UART_printf("Offset too big. Max: F8");
   }
}

void cmd_put(char *str) {
   str[3] = 0;
   uint16_t offset = xtou16(str);
   if (offset < 256 - 8 + 1) {
       str += 4;
       for (uint8_t i = 0; i < 8; i++) {
           str[2] = 0;
           edid_data[offset + i] = xtou16(str);
           str += 3;
       }
   }
}

void cmd_echo(char *str) {
    echo = (str[0] == '1') ? 1 : 0;
    UART_printf("EHO %d\n", echo);
}

void cmd_hpd(char *str) {
    switch(str[0]) {
        case 'H':
            funDigitalWrite(PC3, FUN_LOW);  // low means override, high means passtrough
            funDigitalWrite(PD7, FUN_HIGH);
        break;
        case 'L':
            funDigitalWrite(PC3, FUN_LOW);  // low means override, high means passtrough
            funDigitalWrite(PD7, FUN_LOW);
        break;
        case 'T':
            funDigitalWrite(PC3, FUN_LOW);  // low means override, high means passtrough
            funDigitalWrite(PD7, FUN_LOW);
            Delay_Ms(250);
            funDigitalWrite(PD7, FUN_HIGH);
        break;
        case 'g':
            UART_printf("HPD %d\n", funDigitalRead(PC4));
        break;
        case 't':
            funDigitalWrite(PC3, FUN_HIGH);  // low means override, high means passtrough
        break;
    }
}

int write_page(uint32_t *ptr, uint8_t *data);

void cmd_store(char *str) {
   for(uint8_t i = 0; i < 4; i++) {
       write_page((uint32_t *)(0x08003700 + i * 64), &edid_data[i * 64]);
   }
}

void load_edid_from_flash() {
  uint8_t *ptr = (uint8_t *)0x08003700;
  memcpy(edid_data, ptr, 256);
  //for(uint16_t i = 0; i < 256; i++) UART_printf("%02X ", edid_data[i]);
}

void processLine(char *str) {
  str[3] = 0;
  if (strcmp(str, "GET") == 0) {
      cmd_get(&str[4]);
  } else if (strcmp(str, "PUT") == 0) {
      cmd_put(&str[4]);
  } else if (strcmp(str, "EHO") == 0) {
      cmd_echo(&str[4]);
  } else if (strcmp(str, "HPD") == 0) {
      cmd_hpd(&str[4]);
  } else if (strcmp(str, "STO") == 0) {
      cmd_store(&str[4]);
  } else if (strcmp(str, "XXX") == 0) {
      load_edid_from_flash();
  }
}


int main() {
    SystemInit();
    funGpioInitAll();
    
    funPinMode(PD7, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);  // override HDP (nRST pin have to be configured as GPIO in optionbytes)
    funDigitalWrite(PD7, FUN_LOW);  // override HDP level
    funPinMode(PC3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);  // HPD override/passtrough
    funDigitalWrite(PC3, FUN_LOW);  // low means override, high means passtrough
    
    funPinMode(PD4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);  // I2C override/passtrough
    funDigitalWrite(PD4, FUN_HIGH);  // low means override, high means passtrough
    
    funPinMode(PC0, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);  // LED
    funDigitalWrite(PC0, FUN_HIGH);  // active low
    
    uint16_t x = 0;
    while(0) {
            if(x++==0) {
        funDigitalWrite(PC0, FUN_LOW);
          Delay_Ms(10);
          funDigitalWrite(PC0, FUN_HIGH);
        }
    }
    
    funPinMode(PC4, GPIO_CNF_IN_FLOATING);
    
    UART_init();
    
    while(0) {
        Delay_Ms(1000);
        UART_printf("uwu\n");
    }
  
    // Initialize I2C slave
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
    SetupI2CSlave(0x50, edid_data, sizeof(edid_data), NULL, onRead, true);
    
    Delay_Ms(100);
    funDigitalWrite(PD7, FUN_HIGH);
    funDigitalWrite(PC3, FUN_HIGH);
    
    blink = 5;

    uint16_t c;
    uint8_t i = 0;
    char line_buff[64];
    
    load_edid_from_flash();
    
    while(1) {
        if(x++==0) {
        funDigitalWrite(PC0, FUN_LOW);
          Delay_Ms(10);
          funDigitalWrite(PC0, FUN_HIGH);
        }
        c = UART_getc();
        if(c != UART_NO_DATA) {
            c &= 0x7F;
            if (echo) UART_putc(c);
            if (c == 8) {  // backspace
                i -= 1;
            } else if (i < 64) {
                if (c != '\n' && c != '\r') {
                    line_buff[i++] = c;
                } else if (i) {
                    line_buff[i] = 0;
                    UART_putc('\n');
                    processLine(line_buff);
                    i = 0;
                }
            } else {
                UART_printf("Line too long\n");
                i = 0;
            }
        }
    }

    while (1) {};
}


int write_page(uint32_t *ptr, uint8_t *data)
{
	int start;
	int stop;
	int testok = 1;

	UART_printf( "Starting\n" );

	// Unkock flash - be aware you need extra stuff for the bootloader.
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;

	// For unlocking programming, in general.
	FLASH->MODEKEYR = FLASH_KEY1;
	FLASH->MODEKEYR = FLASH_KEY2;

	UART_printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );
	if( FLASH->CTLR & 0x8080 ) 
	{
		UART_printf( "Flash still locked\n" );
		while(1);
	}

	//uint32_t * ptr = (uint32_t*)0x08003700;
	//uint32_t * ptr = (uint32_t*)0x08003fc0;
	UART_printf( "Memory at: %08lx: %08lx %08lx\n", (uint32_t)ptr, ptr[0], ptr[1] );

	UART_printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );

	//Erase Page
	FLASH->CTLR = CR_PAGE_ER;
	FLASH->ADDR = (intptr_t)ptr;
	FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );  // Takes about 3ms.
	stop = SysTick->CNT;

	UART_printf( "FLASH->STATR = %08lx -> %d cycles for page erase\n", FLASH->STATR, stop - start );
	UART_printf( "Erase complete\n" );


	UART_printf( "Memory at %p: %08lx %08lx\n", ptr, ptr[0], ptr[1] );

	if( ptr[0] != 0xffffffff )
	{
		UART_printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}


	// Clear buffer and prep for flashing.
	FLASH->CTLR = CR_PAGE_PG;  // synonym of FTPG.
	FLASH->CTLR = CR_BUF_RST | CR_PAGE_PG;
	FLASH->ADDR = (intptr_t)ptr;  // This can actually happen about anywhere toward the end here.


	// Note: It takes about 6 clock cycles for this to finish.
	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );  // No real need for this.
	stop = SysTick->CNT;
	UART_printf( "FLASH->STATR = %08lx -> %d cycles for buffer reset\n", FLASH->STATR, stop - start );


	int i;
	start = SysTick->CNT;
	/*
	for( i = 0; i < 16; i++ )
	{
		ptr[i] = 0xabcd1234 + i; //Write to the memory
		FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
		while( FLASH->STATR & FLASH_STATR_BSY );  // Only needed if running from RAM.
	}*/
	
	for( i = 0; i < 16; i++ )
	{
		ptr[i] = ((uint32_t*)data)[i]; //Write to the memory
		FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
		while( FLASH->STATR & FLASH_STATR_BSY );  // Only needed if running from RAM.
	}
	stop = SysTick->CNT;
	UART_printf( "Write: %d cycles for writing data in\n", stop - start );

	// Actually write the flash out. (Takes about 3ms)
	FLASH->CTLR = CR_PAGE_PG|CR_STRT_Set;

	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );
	stop = SysTick->CNT;
	UART_printf( "FLASH->STATR = %08lx -> %d cycles for page write\n", FLASH->STATR, stop - start );

	UART_printf( "FLASH->STATR = %08lx\n", FLASH->STATR );

	UART_printf( "Memory at: %08lx: %08lx %08lx\n", (uint32_t)ptr, ptr[0], ptr[1] );

/*
	if( ptr[0] != 0xabcd1234 )
	{
		UART_printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}
*/
/*
        for( uint8_t ii = 0; ii < 16; ii++ ) {
	  for( i = 0; i < 16; i++ )
		  UART_printf( "%02x ", ptr[ii*16 + i] );
          UART_putc('\n');
        }
        */
        
        	for( i = 0; i < 16; i++ )
		printf( "%08lx ", ptr[i] );
	//UART_printf( "\n\nTest results: %s\n", testok?"PASS":"FAIL" );
	//while(1);
	return testok;
}

