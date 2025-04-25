/********************************************************************************/
/*!
	@file			hx8367a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P3440-W-E TFT module(8/16bit mode).

    @section HISTORY
		2015.05.16	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8367a.h"
/* check header file version for fool proof */
#if HX8367A_H != 0x0300
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#ifdef  USE_HX8367A_SPI_TFT
#ifndef TFT_SDA_READ
 #warning Display_ChangeSDA_If() is NOT Implemented!!
 #define Display_ChangeSDA_If(...) ;
#endif
#endif

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void HX8367A_reset(void)
{
#ifdef USE_HX8367A_TFT
	HX8367A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8367A_RD_SET();
	HX8367A_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/
	
	HX8367A_RES_CLR();							/* RES=L, CS=L   			*/
	HX8367A_CS_CLR();

#elif  USE_HX8367A_SPI_TFT
	HX8367A_RES_SET();							/* RES=H, CS=H				*/
	HX8367A_CS_SET();
	HX8367A_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	HX8367A_RES_CLR();							/* RES=L		   			*/
#endif

	_delay_ms(10);								/* wait 10ms     			*/
	
	HX8367A_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait 125ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_HX8367A_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8367A_wr_cmd(uint8_t cmd)
{
	HX8367A_DC_CLR();							/* DC=L						*/

	HX8367A_CMD = cmd;							/* cmd(8bit)				*/
	HX8367A_WR();								/* WR=L->H					*/

	HX8367A_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8367A_wr_dat(uint8_t dat)
{
	HX8367A_DATA = dat;							/* data						*/
	HX8367A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8367A_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8367A_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	HX8367A_WR();								/* WR=L->H					*/
	HX8367A_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	HX8367A_DATA = gram;						/* 16bit data 				*/
#endif
	HX8367A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8367A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8367A_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8367A_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8367A_rd_cmd(uint8_t cmd)
{
	uint8_t val;

	HX8367A_wr_cmd(cmd);
	HX8367A_WR_SET();

	_delay_ms(1);		/* Some Wait Must be Need on HX8347A, Nemui-San Said So... */
    ReadLCDData(val);

	val &= 0x00FF;
	return val;
}


#elif USE_HX8367A_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8367A_wr_cmd(uint8_t cmd)
{
	HX8367A_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef HX8367ASPI_3WIREMODE
	SendSPI(START_WR_CMD);
#endif
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	HX8367A_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8367A_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef HX8367ASPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8367A_wr_gram(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef HX8367ASPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif
	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8367A_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */
#ifdef HX8367ASPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt );
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
	}
#endif

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8367A_rd_cmd(uint8_t cmd)
{
	/* Does not implemented yet! */
	return 0x67;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8367A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8367A_wr_cmd(0x02);				/* Horizontal RAM Start ADDR2 */
	HX8367A_wr_dat((OFS_COL + x)>>8);
	HX8367A_wr_cmd(0x03);				/* Horizontal RAM Start ADDR1 */
	HX8367A_wr_dat(OFS_COL + x);
	HX8367A_wr_cmd(0x04);				/* Horizontal RAM End ADDR2 */
	HX8367A_wr_dat((OFS_COL + width)>>8);
	HX8367A_wr_cmd(0x05);				/* Horizontal RAM End ADDR1 */
	HX8367A_wr_dat(OFS_COL + width);
	HX8367A_wr_cmd(0x06);				/* Vertical RAM Start ADDR2 */
	HX8367A_wr_dat((OFS_RAW + y)>>8);
	HX8367A_wr_cmd(0x07);				/* Vertical RAM Start ADDR1 */
	HX8367A_wr_dat(OFS_RAW + y);
	HX8367A_wr_cmd(0x08);				/* Vertical RAM End ADDR2 */
	HX8367A_wr_dat((OFS_RAW + height)>>8);
	HX8367A_wr_cmd(0x09);				/* Vertical RAM End ADDR1 */
	HX8367A_wr_dat(OFS_RAW + height);

	HX8367A_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8367A_clear(void)
{
	volatile uint32_t n;

	HX8367A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8367A_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8367A_init(void)
{
	uint8_t id8367a;

	Display_IoInit_If();

	HX8367A_reset();

	/* Check Device Code */
	id8367a = HX8367A_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(id8367a == 0x67)
	{
		/* Initialize HX8367A */
		/* Driving ability Setting */
		HX8367A_wr_cmd(0xEA);
		HX8367A_wr_dat(0x00);			/* Command page 0 */
		HX8367A_wr_cmd(0xEB);
		HX8367A_wr_dat(0x00);			/* SUB_SEL=0xF6 */
		
		/* Power saving for HX8367-A */
		HX8367A_wr_cmd(0xEC);
		HX8367A_wr_dat(0x3C);			/* Command page 0 */
		HX8367A_wr_cmd(0xED);
		HX8367A_wr_dat(0xC4);			/* GENON=0x00 */
		HX8367A_wr_cmd(0xE8);
		HX8367A_wr_dat(0x48);			/* EQVCI_M1=0x00 */
		HX8367A_wr_cmd(0xE9);
		HX8367A_wr_dat(0x38);			/* EQGND_M1=0x1C */
		HX8367A_wr_cmd(0xF1);
		HX8367A_wr_dat(0x01);			/* EQVCI_M0=0x1C */
		HX8367A_wr_cmd(0xF2);
		HX8367A_wr_dat(0x00);			/* EQGND_M0=0x1C */
		HX8367A_wr_cmd(0x27);
		HX8367A_wr_dat(0xA3);			/* For GRAM read/write speed */
		HX8367A_wr_cmd(0x2E);
		HX8367A_wr_dat(0x76);			/* For Gate timing, prevent the display abnormal in RGB I/F */
		HX8367A_wr_cmd(0x60);
		HX8367A_wr_dat(0x08);
		HX8367A_wr_cmd(0x29);
		HX8367A_wr_dat(0x01);
		HX8367A_wr_cmd(0x2B);
		HX8367A_wr_dat(0x02);
		HX8367A_wr_cmd(0x36);
		HX8367A_wr_dat(0x09);
		
		/* Gamma */
		HX8367A_wr_cmd(0x40);
		HX8367A_wr_dat(0x00);
		HX8367A_wr_cmd(0x41);
		HX8367A_wr_dat(0x01);
		HX8367A_wr_cmd(0x42);
		HX8367A_wr_dat(0x01);
		HX8367A_wr_cmd(0x43);
		HX8367A_wr_dat(0x12);
		HX8367A_wr_cmd(0x44);
		HX8367A_wr_dat(0x10);
		HX8367A_wr_cmd(0x45);
		HX8367A_wr_dat(0x24);
		HX8367A_wr_cmd(0x46);
		HX8367A_wr_dat(0x05);
		HX8367A_wr_cmd(0x47);
		HX8367A_wr_dat(0x5B);
		HX8367A_wr_cmd(0x48);
		HX8367A_wr_dat(0x03);
		HX8367A_wr_cmd(0x49);
		HX8367A_wr_dat(0x11);
		HX8367A_wr_cmd(0x4A);
		HX8367A_wr_dat(0x17);
		HX8367A_wr_cmd(0x4B);
		HX8367A_wr_dat(0x18);
		HX8367A_wr_cmd(0x4C);
		HX8367A_wr_dat(0x19);
		HX8367A_wr_cmd(0x50);
		HX8367A_wr_dat(0x1B);
		HX8367A_wr_cmd(0x51);
		HX8367A_wr_dat(0x2F);
		HX8367A_wr_cmd(0x52);
		HX8367A_wr_dat(0x2D);
		HX8367A_wr_cmd(0x53);
		HX8367A_wr_dat(0x3E);
		HX8367A_wr_cmd(0x54);
		HX8367A_wr_dat(0x3E);
		HX8367A_wr_cmd(0x55);
		HX8367A_wr_dat(0x3F);
		HX8367A_wr_cmd(0x56);
		HX8367A_wr_dat(0x30);
		HX8367A_wr_cmd(0x57);
		HX8367A_wr_dat(0x6E);
		HX8367A_wr_cmd(0x58);
		HX8367A_wr_dat(0x06);
		HX8367A_wr_cmd(0x59);
		HX8367A_wr_dat(0x07);
		HX8367A_wr_cmd(0x5A);
		HX8367A_wr_dat(0x08);
		HX8367A_wr_cmd(0x5B);
		HX8367A_wr_dat(0x0E);
		HX8367A_wr_cmd(0x5C);
		HX8367A_wr_dat(0x1C);
		HX8367A_wr_cmd(0x5D);
		HX8367A_wr_dat(0xCC);
		
		/* Power Setting */
		HX8367A_wr_cmd(0xE2);
		HX8367A_wr_dat(0x03);
		HX8367A_wr_cmd(0x1B);
		HX8367A_wr_dat(0x1D);
		HX8367A_wr_cmd(0x1A);
		HX8367A_wr_dat(0x01);
		HX8367A_wr_cmd(0x24);
		HX8367A_wr_dat(0x37);			/* Set VCOMH voltage, VHH=0x64 */
		HX8367A_wr_cmd(0x25);
		HX8367A_wr_dat(0x4F);			/* Set VCOML voltage, VML=0x71 */
		HX8367A_wr_cmd(0x23);
		HX8367A_wr_dat(0x6A);			/* Set VCOM offset, VMF=0x52 */
		
		/* Power on Setting */
		HX8367A_wr_cmd(0x18);
		HX8367A_wr_dat(0x3A);			/* OSC_EN=1, Start to Oscillate */
		HX8367A_wr_cmd(0x19);
		HX8367A_wr_dat(0x01);			/* AP=011 */
		HX8367A_wr_cmd(0x01);
		HX8367A_wr_dat(0x00);			/* Normal display(Exit Deep standby mode) */
		HX8367A_wr_cmd(0x1F);
		HX8367A_wr_dat(0x88);			/* Exit standby mode and Step-up circuit 1 enable */
										/* GAS_EN=1, VCOMG=0, PON=0, DK=0, XDK=0, DDVDH_TRI=0, STB=0 */
		_delay_ms(5);
		HX8367A_wr_cmd(0x1F);
		HX8367A_wr_dat(0x80);			/* Step-up circuit 2 enable */
										/* GAS_EN=1, VCOMG=0, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0 */
		_delay_ms(5);
		HX8367A_wr_cmd(0x1F);
		HX8367A_wr_dat(0x90);
		_delay_ms(5);
		HX8367A_wr_cmd(0x1F);
		HX8367A_wr_dat(0xD0);			/* GAS_EN=1, VCOMG=1, PON=1, DK=0, XDK=1, DDVDH_TRI=0, STB=0 */
		_delay_ms(5);
		
		/* Display ON Setting */
		HX8367A_wr_cmd(0x17);
		HX8367A_wr_dat(0x05);			/* GON=1, DTE=1, D[1:0]=10 */
		_delay_ms(40);
		HX8367A_wr_cmd(0x28);
		HX8367A_wr_dat(0x38);			/* GON=1, DTE=1, D[1:0]=11 */
		HX8367A_wr_cmd(0x28);
		HX8367A_wr_dat(0x3C);			/* 16-bit/pixel */
		
		HX8367A_wr_cmd(0x36);
		HX8367A_wr_dat(0x01);    		/* REV_P, SM_P, GS_P, BGR_P, SS_P */
		HX8367A_wr_cmd(0x16);
		HX8367A_wr_dat((0<<7)|(1<<6)|(0<<5)|(0<<4)|(0<<3));		/* MX, MY, RGB mode */
	}

	else { for(;;);}					/* Invalid Device Code!! */

	HX8367A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8367A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8367A_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
