/********************************************************************************/
/*!
	@file			nt35510.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TK040F1510 TFT module(8/16bit mode).

    @section HISTORY
		2013.05.02	V1.00	Stable Release
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01  V4.00	Revised initialize routine.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "nt35510.h"
/* check header file version for fool proof */
#if NT35510_H != 0x0400
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void NT35510_reset(void)
{
	NT35510_RES_SET();							/* RES=H, RD=H, WR=H   	*/
	NT35510_RD_SET();
	NT35510_WR_SET();
	_delay_ms(20);								/* wait 20ms     		*/

	NT35510_RES_CLR();							/* RES=L, CS=L   		*/
	NT35510_CS_CLR();
	_delay_ms(10);								/* wait 10ms     		*/
	
	NT35510_RES_SET();						  	/* RES=H				*/
	_delay_ms(120);				    			/* wait 120ms     		*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void NT35510_wr_cmd(uint16_t cmd)
{
	NT35510_DC_CLR();							/* DC=L					*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	NT35510_CMD = (uint8_t)(cmd>>8);			/* upper 8bit command	*/
	NT35510_WR();								/* WR=L->H				*/
	NT35510_CMD = (uint8_t)cmd;					/* lower 8bit command	*/
#else
	NT35510_CMD = cmd;							/* 16bit command		*/
#endif
    NT35510_WR();								/* WR=L->H				*/

	NT35510_DC_SET();							/* DC=H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void NT35510_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	NT35510_DATA = (uint8_t)(dat>>8);			/* upper 8bit data		*/
	NT35510_WR();								/* WR=L->H				*/
	NT35510_DATA = (uint8_t)dat;				/* lower 8bit data		*/
#else
	NT35510_DATA = dat;							/* 16bit data 			*/
#endif
	NT35510_WR();								/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void NT35510_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		NT35510_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		NT35510_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void NT35510_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	NT35510_wr_cmd(0x2A00);				/* Horizontal RAM Start ADDR */
	NT35510_wr_dat((OFS_COL + x)>>8);
	NT35510_wr_cmd(0x2A01);
	NT35510_wr_dat(OFS_COL + x);
	NT35510_wr_cmd(0x2A02);				/* Horizontal RAM End ADDR */
	NT35510_wr_dat((OFS_COL + width)>>8);
	NT35510_wr_cmd(0x2A03);
	NT35510_wr_dat(OFS_COL + width);

	NT35510_wr_cmd(0x2B00);				/* Vertical RAM Start ADDR */
	NT35510_wr_dat((OFS_RAW + y)>>8);
	NT35510_wr_cmd(0x2B01);
	NT35510_wr_dat(OFS_RAW + y);
	NT35510_wr_cmd(0x2B02);				/* Vertical RAM End ADDR */
	NT35510_wr_dat((OFS_RAW + height)>>8);
	NT35510_wr_cmd(0x2B03);
	NT35510_wr_dat(OFS_RAW + height);

	NT35510_wr_cmd(0x2C00);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void NT35510_clear(void)
{
	volatile uint32_t n;

	NT35510_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		NT35510_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t NT35510_rd_cmd(uint16_t cmd)
{
	uint16_t val,temp;
	
	/* PAGE1 */
	NT35510_wr_cmd(0xF000);	NT35510_wr_dat(0x0055);
	NT35510_wr_cmd(0xF001);	NT35510_wr_dat(0x00AA);	
	NT35510_wr_cmd(0xF002);	NT35510_wr_dat(0x0052);	
	NT35510_wr_cmd(0xF003);	NT35510_wr_dat(0x0008);	
	NT35510_wr_cmd(0xF004);	NT35510_wr_dat(0x0001);	

	NT35510_wr_cmd(cmd);
	NT35510_WR_SET();
	
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif
    ReadLCDData(temp);
	temp <<= 8;

	NT35510_wr_cmd(cmd | 0x0001);
	NT35510_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(val);
#endif
    ReadLCDData(val);
	
	val = (val & 0x00FF) | temp;

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void NT35510_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	NT35510_reset();

	devicetype = NT35510_rd_cmd(0xC500);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x5510)
	{
#if 1
		/* Initialize NT35510 */
		/* PAGE1 */
		NT35510_wr_cmd(0xF000);	NT35510_wr_dat(0x0055);
		NT35510_wr_cmd(0xF001);	NT35510_wr_dat(0x00AA);	
		NT35510_wr_cmd(0xF002);	NT35510_wr_dat(0x0052);	
		NT35510_wr_cmd(0xF003);	NT35510_wr_dat(0x0008);	
		NT35510_wr_cmd(0xF004);	NT35510_wr_dat(0x0001);	
        /* VGMP/VGSP=4.5V/0V */
		NT35510_wr_cmd(0xBC01);	NT35510_wr_dat(0x0086);	
		NT35510_wr_cmd(0xBC02);	NT35510_wr_dat(0x006A);	
		NT35510_wr_cmd(0xBD01);	NT35510_wr_dat(0x0086);	
		NT35510_wr_cmd(0xBD02);	NT35510_wr_dat(0x006A);	
		NT35510_wr_cmd(0xBE01);	NT35510_wr_dat(0x0067);	
        /* Gamma (R+) */
		NT35510_wr_cmd(0xD100);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD101);	NT35510_wr_dat(0x005D);	
		NT35510_wr_cmd(0xD102);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD103);	NT35510_wr_dat(0x006B);	
		NT35510_wr_cmd(0xD104);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD105);	NT35510_wr_dat(0x0084);	
		NT35510_wr_cmd(0xD106);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD107);	NT35510_wr_dat(0x009C);	
		NT35510_wr_cmd(0xD108);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD109);	NT35510_wr_dat(0x00B1);	
		NT35510_wr_cmd(0xD10A);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD10B);	NT35510_wr_dat(0x00D9);	
		NT35510_wr_cmd(0xD10C);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD10D);	NT35510_wr_dat(0x00FD);	
		NT35510_wr_cmd(0xD10E);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD10F);	NT35510_wr_dat(0x0038);	
		NT35510_wr_cmd(0xD110);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD111);	NT35510_wr_dat(0x0068);	
		NT35510_wr_cmd(0xD112);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD113);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD114);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD115);	NT35510_wr_dat(0x00FB);	
		NT35510_wr_cmd(0xD116);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD117);	NT35510_wr_dat(0x0063);	
		NT35510_wr_cmd(0xD118);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD119);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD11A);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD11B);	NT35510_wr_dat(0x00BB);	
		NT35510_wr_cmd(0xD11C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD11D);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD11E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD11F);	NT35510_wr_dat(0x0046);	
		NT35510_wr_cmd(0xD120);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD121);	NT35510_wr_dat(0x0069);	
		NT35510_wr_cmd(0xD122);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD123);	NT35510_wr_dat(0x008F);	
		NT35510_wr_cmd(0xD124);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD125);	NT35510_wr_dat(0x00A4);	
		NT35510_wr_cmd(0xD126);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD127);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD128);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD129);	NT35510_wr_dat(0x00C7);	
		NT35510_wr_cmd(0xD12A);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD12B);	NT35510_wr_dat(0x00C9);	
		NT35510_wr_cmd(0xD12C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD12D);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD12E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD12F);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD130);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD131);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD132);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD133);	NT35510_wr_dat(0x00CC);	
        /* Gamma (G+) */
		NT35510_wr_cmd(0xD200);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD201);	NT35510_wr_dat(0x005D);	
		NT35510_wr_cmd(0xD202);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD203);	NT35510_wr_dat(0x006B);	
		NT35510_wr_cmd(0xD204);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD205);	NT35510_wr_dat(0x0084);	
		NT35510_wr_cmd(0xD206);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD207);	NT35510_wr_dat(0x009C);	
		NT35510_wr_cmd(0xD208);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD209);	NT35510_wr_dat(0x00B1);	
		NT35510_wr_cmd(0xD20A);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD20B);	NT35510_wr_dat(0x00D9);	
		NT35510_wr_cmd(0xD20C);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD20D);	NT35510_wr_dat(0x00FD);	
		NT35510_wr_cmd(0xD20E);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD20F);	NT35510_wr_dat(0x0038);	
		NT35510_wr_cmd(0xD210);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD211);	NT35510_wr_dat(0x0068);	
		NT35510_wr_cmd(0xD212);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD213);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD214);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD215);	NT35510_wr_dat(0x00FB);	
		NT35510_wr_cmd(0xD216);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD217);	NT35510_wr_dat(0x0063);	
		NT35510_wr_cmd(0xD218);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD219);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD21A);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD21B);	NT35510_wr_dat(0x00BB);	
		NT35510_wr_cmd(0xD21C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD21D);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD21E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD21F);	NT35510_wr_dat(0x0046);	
		NT35510_wr_cmd(0xD220);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD221);	NT35510_wr_dat(0x0069);	
		NT35510_wr_cmd(0xD222);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD223);	NT35510_wr_dat(0x008F);	
		NT35510_wr_cmd(0xD224);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD225);	NT35510_wr_dat(0x00A4);	
		NT35510_wr_cmd(0xD226);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD227);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD228);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD229);	NT35510_wr_dat(0x00C7);	
		NT35510_wr_cmd(0xD22A);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD22B);	NT35510_wr_dat(0x00C9);	
		NT35510_wr_cmd(0xD22C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD22D);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD22E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD22F);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD230);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD231);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD232);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD233);	NT35510_wr_dat(0x00CC);	
        /* Gamma (B+) */
		NT35510_wr_cmd(0xD300);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD301);	NT35510_wr_dat(0x005D);	
		NT35510_wr_cmd(0xD302);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD303);	NT35510_wr_dat(0x006B);	
		NT35510_wr_cmd(0xD304);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD305);	NT35510_wr_dat(0x0084);	
		NT35510_wr_cmd(0xD306);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD307);	NT35510_wr_dat(0x009C);	
		NT35510_wr_cmd(0xD308);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD309);	NT35510_wr_dat(0x00B1);	
		NT35510_wr_cmd(0xD30A);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD30B);	NT35510_wr_dat(0x00D9);	
		NT35510_wr_cmd(0xD30C);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD30D);	NT35510_wr_dat(0x00FD);	
		NT35510_wr_cmd(0xD30E);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD30F);	NT35510_wr_dat(0x0038);	
		NT35510_wr_cmd(0xD310);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD311);	NT35510_wr_dat(0x0068);	
		NT35510_wr_cmd(0xD312);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD313);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD314);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD315);	NT35510_wr_dat(0x00FB);	
		NT35510_wr_cmd(0xD316);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD317);	NT35510_wr_dat(0x0063);	
		NT35510_wr_cmd(0xD318);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD319);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD31A);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD31B);	NT35510_wr_dat(0x00BB);	
		NT35510_wr_cmd(0xD31C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD31D);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD31E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD31F);	NT35510_wr_dat(0x0046);	
		NT35510_wr_cmd(0xD320);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD321);	NT35510_wr_dat(0x0069);	
		NT35510_wr_cmd(0xD322);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD323);	NT35510_wr_dat(0x008F);	
		NT35510_wr_cmd(0xD324);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD325);	NT35510_wr_dat(0x00A4);	
		NT35510_wr_cmd(0xD326);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD327);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD328);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD329);	NT35510_wr_dat(0x00C7);	
		NT35510_wr_cmd(0xD32A);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD32B);	NT35510_wr_dat(0x00C9);	
		NT35510_wr_cmd(0xD32C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD32D);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD32E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD32F);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD330);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD331);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD332);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD333);	NT35510_wr_dat(0x00CC);	
        /* Gamma (R-) */
		NT35510_wr_cmd(0xD400);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD401);	NT35510_wr_dat(0x005D);	
		NT35510_wr_cmd(0xD402);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD403);	NT35510_wr_dat(0x006B);	
		NT35510_wr_cmd(0xD404);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD405);	NT35510_wr_dat(0x0084);	
		NT35510_wr_cmd(0xD406);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD407);	NT35510_wr_dat(0x009C);	
		NT35510_wr_cmd(0xD408);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD409);	NT35510_wr_dat(0x00B1);	
		NT35510_wr_cmd(0xD40A);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD40B);	NT35510_wr_dat(0x00D9);	
		NT35510_wr_cmd(0xD40C);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD40D);	NT35510_wr_dat(0x00FD);	
		NT35510_wr_cmd(0xD40E);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD40F);	NT35510_wr_dat(0x0038);	
		NT35510_wr_cmd(0xD410);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD411);	NT35510_wr_dat(0x0068);	
		NT35510_wr_cmd(0xD412);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD413);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD414);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD415);	NT35510_wr_dat(0x00FB);	
		NT35510_wr_cmd(0xD416);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD417);	NT35510_wr_dat(0x0063);	
		NT35510_wr_cmd(0xD418);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD419);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD41A);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD41B);	NT35510_wr_dat(0x00BB);	
		NT35510_wr_cmd(0xD41C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD41D);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD41E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD41F);	NT35510_wr_dat(0x0046);	
		NT35510_wr_cmd(0xD420);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD421);	NT35510_wr_dat(0x0069);	
		NT35510_wr_cmd(0xD422);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD423);	NT35510_wr_dat(0x008F);	
		NT35510_wr_cmd(0xD424);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD425);	NT35510_wr_dat(0x00A4);	
		NT35510_wr_cmd(0xD426);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD427);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD428);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD429);	NT35510_wr_dat(0x00C7);	
		NT35510_wr_cmd(0xD42A);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD42B);	NT35510_wr_dat(0x00C9);	
		NT35510_wr_cmd(0xD42C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD42D);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD42E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD42F);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD430);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD431);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD432);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD433);	NT35510_wr_dat(0x00CC);	
        /* Gamma (G-) */
		NT35510_wr_cmd(0xD500);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD501);	NT35510_wr_dat(0x005D);	
		NT35510_wr_cmd(0xD502);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD503);	NT35510_wr_dat(0x006B);	
		NT35510_wr_cmd(0xD504);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD505);	NT35510_wr_dat(0x0084);	
		NT35510_wr_cmd(0xD506);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD507);	NT35510_wr_dat(0x009C);	
		NT35510_wr_cmd(0xD508);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD509);	NT35510_wr_dat(0x00B1);	
		NT35510_wr_cmd(0xD50A);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD50B);	NT35510_wr_dat(0x00D9);	
		NT35510_wr_cmd(0xD50C);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD50D);	NT35510_wr_dat(0x00FD);	
		NT35510_wr_cmd(0xD50E);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD50F);	NT35510_wr_dat(0x0038);	
		NT35510_wr_cmd(0xD510);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD511);	NT35510_wr_dat(0x0068);	
		NT35510_wr_cmd(0xD512);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD513);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD514);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD515);	NT35510_wr_dat(0x00FB);	
		NT35510_wr_cmd(0xD516);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD517);	NT35510_wr_dat(0x0063);	
		NT35510_wr_cmd(0xD518);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD519);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD51A);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD51B);	NT35510_wr_dat(0x00BB);	
		NT35510_wr_cmd(0xD51C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD51D);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD51E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD51F);	NT35510_wr_dat(0x0046);	
		NT35510_wr_cmd(0xD520);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD521);	NT35510_wr_dat(0x0069);	
		NT35510_wr_cmd(0xD522);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD523);	NT35510_wr_dat(0x008F);	
		NT35510_wr_cmd(0xD524);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD525);	NT35510_wr_dat(0x00A4);	
		NT35510_wr_cmd(0xD526);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD527);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD528);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD529);	NT35510_wr_dat(0x00C7);	
		NT35510_wr_cmd(0xD52A);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD52B);	NT35510_wr_dat(0x00C9);	
		NT35510_wr_cmd(0xD52C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD52D);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD52E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD52F);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD530);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD531);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD532);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD533);	NT35510_wr_dat(0x00CC);	
        /* Gamma (B-) */
		NT35510_wr_cmd(0xD600);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD601);	NT35510_wr_dat(0x005D);	
		NT35510_wr_cmd(0xD602);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD603);	NT35510_wr_dat(0x006B);	
		NT35510_wr_cmd(0xD604);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD605);	NT35510_wr_dat(0x0084);	
		NT35510_wr_cmd(0xD606);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD607);	NT35510_wr_dat(0x009C);	
		NT35510_wr_cmd(0xD608);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD609);	NT35510_wr_dat(0x00B1);	
		NT35510_wr_cmd(0xD60A);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD60B);	NT35510_wr_dat(0x00D9);	
		NT35510_wr_cmd(0xD60C);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0xD60D);	NT35510_wr_dat(0x00FD);	
		NT35510_wr_cmd(0xD60E);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD60F);	NT35510_wr_dat(0x0038);	
		NT35510_wr_cmd(0xD610);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD611);	NT35510_wr_dat(0x0068);	
		NT35510_wr_cmd(0xD612);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD613);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD614);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xD615);	NT35510_wr_dat(0x00FB);	
		NT35510_wr_cmd(0xD616);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD617);	NT35510_wr_dat(0x0063);	
		NT35510_wr_cmd(0xD618);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD619);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD61A);	NT35510_wr_dat(0x0002);	
		NT35510_wr_cmd(0xD61B);	NT35510_wr_dat(0x00BB);	
		NT35510_wr_cmd(0xD61C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD61D);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD61E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD61F);	NT35510_wr_dat(0x0046);	
		NT35510_wr_cmd(0xD620);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD621);	NT35510_wr_dat(0x0069);	
		NT35510_wr_cmd(0xD622);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD623);	NT35510_wr_dat(0x008F);	
		NT35510_wr_cmd(0xD624);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD625);	NT35510_wr_dat(0x00A4);	
		NT35510_wr_cmd(0xD626);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD627);	NT35510_wr_dat(0x00B9);	
		NT35510_wr_cmd(0xD628);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD629);	NT35510_wr_dat(0x00C7);	
		NT35510_wr_cmd(0xD62A);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD62B);	NT35510_wr_dat(0x00C9);	
		NT35510_wr_cmd(0xD62C);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD62D);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD62E);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD62F);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD630);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD631);	NT35510_wr_dat(0x00CB);	
		NT35510_wr_cmd(0xD632);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xD633);	NT35510_wr_dat(0x00CC);	
        /* VGLX Ratio */
		NT35510_wr_cmd(0xBA00);	NT35510_wr_dat(0x0024);	
		NT35510_wr_cmd(0xBA01);	NT35510_wr_dat(0x0024);	
		NT35510_wr_cmd(0xBA02);	NT35510_wr_dat(0x0024);	
        /* VGH Ratio */
		NT35510_wr_cmd(0xB900);	NT35510_wr_dat(0x0024);	
		NT35510_wr_cmd(0xB901);	NT35510_wr_dat(0x0024);	
		NT35510_wr_cmd(0xB902);	NT35510_wr_dat(0x0024);	
		/* PAGE0 */
		NT35510_wr_cmd(0xF000);	NT35510_wr_dat(0x0055);          
		NT35510_wr_cmd(0xF001);	NT35510_wr_dat(0x00AA);	
		NT35510_wr_cmd(0xF002);	NT35510_wr_dat(0x0052);	
		NT35510_wr_cmd(0xF003);	NT35510_wr_dat(0x0008);	
		NT35510_wr_cmd(0xF004);	NT35510_wr_dat(0x0000);	
        /* Display control */
		NT35510_wr_cmd(0xB100);	NT35510_wr_dat(0x00CC);	
        /* Inversion mode: column */
		NT35510_wr_cmd(0xBC00);	NT35510_wr_dat(0x0005);	
		NT35510_wr_cmd(0xBC01);	NT35510_wr_dat(0x0005);	
		NT35510_wr_cmd(0xBC02);	NT35510_wr_dat(0x0005);	
        /* Source EQ control (Nova non-used) */
		NT35510_wr_cmd(0xB800);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0xB801);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xB802);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0xB803);	NT35510_wr_dat(0x0003);	
        /* Frame rate	(Nova non-used) */
		NT35510_wr_cmd(0xBD02);	NT35510_wr_dat(0x0007);	
		NT35510_wr_cmd(0xBD03);	NT35510_wr_dat(0x0031);	
		NT35510_wr_cmd(0xBE02);	NT35510_wr_dat(0x0007);	
		NT35510_wr_cmd(0xBE03);	NT35510_wr_dat(0x0031);	
		NT35510_wr_cmd(0xBF02);	NT35510_wr_dat(0x0007);	
		NT35510_wr_cmd(0xBF03);	NT35510_wr_dat(0x0031);	
        /* PAGE? */
		NT35510_wr_cmd(0xFF00);	NT35510_wr_dat(0x00AA);	
		NT35510_wr_cmd(0xFF01);	NT35510_wr_dat(0x0055);	
		NT35510_wr_cmd(0xFF02);	NT35510_wr_dat(0x0025);	
		NT35510_wr_cmd(0xFF03);	NT35510_wr_dat(0x0001);	
		/* ??? */
		NT35510_wr_cmd(0xF304);	NT35510_wr_dat(0x0011);	
		NT35510_wr_cmd(0xF306);	NT35510_wr_dat(0x0010);	
		NT35510_wr_cmd(0xF308);	NT35510_wr_dat(0x0000);	
		/* Tearing effect OFF */
		NT35510_wr_cmd(0x3500);	NT35510_wr_dat(0x0000);	
		/* RasSet */
		NT35510_wr_cmd(0x2A00);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0x2A01);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0x2A02);	NT35510_wr_dat(0x0001);	
		NT35510_wr_cmd(0x2A03);	NT35510_wr_dat(0x00DF);	
		/* CasSet*/
		NT35510_wr_cmd(0x2B00);	NT35510_wr_dat(0x0000);  
		NT35510_wr_cmd(0x2B01);	NT35510_wr_dat(0x0000);	
		NT35510_wr_cmd(0x2B02);	NT35510_wr_dat(0x0003);	
		NT35510_wr_cmd(0x2B03);	NT35510_wr_dat(0x001F);	
		
#else	/* Obsoleted SF-TC500H-9658A-N module ini*/
		/* PAGE1 */
		NT35510_wr_cmd(0xF000);	NT35510_wr_dat(0x0055);
		NT35510_wr_cmd(0xF001);	NT35510_wr_dat(0x00AA);
		NT35510_wr_cmd(0xF002);	NT35510_wr_dat(0x0052);
		NT35510_wr_cmd(0xF003);	NT35510_wr_dat(0x0008);
		NT35510_wr_cmd(0xF004);	NT35510_wr_dat(0x0001);
		/* Set AVDD 5.2V */
		NT35510_wr_cmd(0xB000);	NT35510_wr_dat(0x000D);
		NT35510_wr_cmd(0xB001);	NT35510_wr_dat(0x000D);
		NT35510_wr_cmd(0xB002);	NT35510_wr_dat(0x000D);
		/* Set AVEE 5.2V */
		NT35510_wr_cmd(0xB100);	NT35510_wr_dat(0x000D);
		NT35510_wr_cmd(0xB101);	NT35510_wr_dat(0x000D);
		NT35510_wr_cmd(0xB102);	NT35510_wr_dat(0x000D);
		/* Set VCL -2.5V */
		NT35510_wr_cmd(0xB200);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xB201);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xB202);	NT35510_wr_dat(0x0000);
		/* Set AVDD Ratio */
		NT35510_wr_cmd(0xB600);	NT35510_wr_dat(0x0044);
		NT35510_wr_cmd(0xB601);	NT35510_wr_dat(0x0044);
		NT35510_wr_cmd(0xB602);	NT35510_wr_dat(0x0044);
		/* Set AVEE Ratio */
		NT35510_wr_cmd(0xB700);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xB701);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xB702);	NT35510_wr_dat(0x0034);
		/* Set VCL -2.5V */
		NT35510_wr_cmd(0xB800);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xB801);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xB802);	NT35510_wr_dat(0x0034);
		/* Control VGH booster voltage rang */
		NT35510_wr_cmd(0xBF00);	NT35510_wr_dat(0x0001);		/* VGH:7~18V */
		/* VGH=15V(1V/step)	Free pump */
		NT35510_wr_cmd(0xB300);	NT35510_wr_dat(0x000F);		/* 08 */
		NT35510_wr_cmd(0xB301);	NT35510_wr_dat(0x000F);		/* 08 */
		NT35510_wr_cmd(0xB302);	NT35510_wr_dat(0x000F);		/* 08 */
		/* VGH Ratio */
		NT35510_wr_cmd(0xB900);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xB901);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xB902);	NT35510_wr_dat(0x0034);
		/* VGL_REG=-10(1V/step) */
		NT35510_wr_cmd(0xB500);	NT35510_wr_dat(0x0008);
		NT35510_wr_cmd(0xB501);	NT35510_wr_dat(0x0008);
		NT35510_wr_cmd(0xB502);	NT35510_wr_dat(0x0008);

		NT35510_wr_cmd(0xC200);	NT35510_wr_dat(0x0003);
		/* VGLX Ratio */
		NT35510_wr_cmd(0xBA00);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xBA01);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xBA02);	NT35510_wr_dat(0x0034);
		/* VGMP/VGSP=4.5V/0V */
		NT35510_wr_cmd(0xBC00);	NT35510_wr_dat(0x0000);		/* 00 */
		NT35510_wr_cmd(0xBC01);	NT35510_wr_dat(0x0078);		/* C8 =5.5V/90=4.8V */
		NT35510_wr_cmd(0xBC02);	NT35510_wr_dat(0x0000);		/* 01 */
		/* VGMN/VGSN=-4.5V/0V */
		NT35510_wr_cmd(0xBD00);	NT35510_wr_dat(0x0000);		/* 00 */
		NT35510_wr_cmd(0xBD01);	NT35510_wr_dat(0x0078);		/* 90 */
		NT35510_wr_cmd(0xBD02);	NT35510_wr_dat(0x0000);
		/* Vcom=-1.4V(12.5mV/step) */
		NT35510_wr_cmd(0xBE00);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xBE01);	NT35510_wr_dat(0x006F);		/* HSD:64;Novatek:50=-1.0V, 80 */
		/* Gamma (R+) */
		NT35510_wr_cmd(0xD100);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD101);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD102);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD103);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD104);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD105);	NT35510_wr_dat(0x003A);
		NT35510_wr_cmd(0xD106);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD107);	NT35510_wr_dat(0x004A);
		NT35510_wr_cmd(0xD108);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD109);	NT35510_wr_dat(0x005C);
		NT35510_wr_cmd(0xD10A);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD10B);	NT35510_wr_dat(0x0081);
		NT35510_wr_cmd(0xD10C);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD10D);	NT35510_wr_dat(0x00A6);
		NT35510_wr_cmd(0xD10E);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD10F);	NT35510_wr_dat(0x00E5);
		NT35510_wr_cmd(0xD110);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD111);	NT35510_wr_dat(0x0013);
		NT35510_wr_cmd(0xD112);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD113);	NT35510_wr_dat(0x0054);
		NT35510_wr_cmd(0xD114);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD115);	NT35510_wr_dat(0x0082);
		NT35510_wr_cmd(0xD116);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD117);	NT35510_wr_dat(0x00CA);
		NT35510_wr_cmd(0xD118);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD119);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD11A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD11B);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD11C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD11D);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD11E);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD11F);	NT35510_wr_dat(0x0067);
		NT35510_wr_cmd(0xD120);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD121);	NT35510_wr_dat(0x0084);
		NT35510_wr_cmd(0xD122);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD123);	NT35510_wr_dat(0x00A4);
		NT35510_wr_cmd(0xD124);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD125);	NT35510_wr_dat(0x00B7);
		NT35510_wr_cmd(0xD126);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD127);	NT35510_wr_dat(0x00CF);
		NT35510_wr_cmd(0xD128);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD129);	NT35510_wr_dat(0x00DE);
		NT35510_wr_cmd(0xD12A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD12B);	NT35510_wr_dat(0x00F2);
		NT35510_wr_cmd(0xD12C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD12D);	NT35510_wr_dat(0x00FE);
		NT35510_wr_cmd(0xD12E);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD12F);	NT35510_wr_dat(0x0010);
		NT35510_wr_cmd(0xD130);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD131);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD132);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD133);	NT35510_wr_dat(0x006D);
		/* Gamma (G+) */
		NT35510_wr_cmd(0xD200);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD201);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD202);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD203);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD204);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD205);	NT35510_wr_dat(0x003A);
		NT35510_wr_cmd(0xD206);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD207);	NT35510_wr_dat(0x004A);
		NT35510_wr_cmd(0xD208);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD209);	NT35510_wr_dat(0x005C);
		NT35510_wr_cmd(0xD20A);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD20B);	NT35510_wr_dat(0x0081);
		NT35510_wr_cmd(0xD20C);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD20D);	NT35510_wr_dat(0x00A6);
		NT35510_wr_cmd(0xD20E);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD20F);	NT35510_wr_dat(0x00E5);
		NT35510_wr_cmd(0xD210);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD211);	NT35510_wr_dat(0x0013);
		NT35510_wr_cmd(0xD212);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD213);	NT35510_wr_dat(0x0054);
		NT35510_wr_cmd(0xD214);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD215);	NT35510_wr_dat(0x0082);
		NT35510_wr_cmd(0xD216);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD217);	NT35510_wr_dat(0x00CA);
		NT35510_wr_cmd(0xD218);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD219);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD21A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD21B);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD21C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD21D);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD21E);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD21F);	NT35510_wr_dat(0x0067);
		NT35510_wr_cmd(0xD220);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD221);	NT35510_wr_dat(0x0084);
		NT35510_wr_cmd(0xD222);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD223);	NT35510_wr_dat(0x00A4);
		NT35510_wr_cmd(0xD224);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD225);	NT35510_wr_dat(0x00B7);
		NT35510_wr_cmd(0xD226);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD227);	NT35510_wr_dat(0x00CF);
		NT35510_wr_cmd(0xD228);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD229);	NT35510_wr_dat(0x00DE);
		NT35510_wr_cmd(0xD22A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD22B);	NT35510_wr_dat(0x00F2);
		NT35510_wr_cmd(0xD22C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD22D);	NT35510_wr_dat(0x00FE);
		NT35510_wr_cmd(0xD22E);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD22F);	NT35510_wr_dat(0x0010);
		NT35510_wr_cmd(0xD230);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD231);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD232);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD233);	NT35510_wr_dat(0x006D);
		/* Gamma (B+) */
		NT35510_wr_cmd(0xD300);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD301);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD302);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD303);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD304);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD305);	NT35510_wr_dat(0x003A);
		NT35510_wr_cmd(0xD306);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD307);	NT35510_wr_dat(0x004A);
		NT35510_wr_cmd(0xD308);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD309);	NT35510_wr_dat(0x005C);
		NT35510_wr_cmd(0xD30A);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD30B);	NT35510_wr_dat(0x0081);
		NT35510_wr_cmd(0xD30C);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD30D);	NT35510_wr_dat(0x00A6);
		NT35510_wr_cmd(0xD30E);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD30F);	NT35510_wr_dat(0x00E5);
		NT35510_wr_cmd(0xD310);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD311);	NT35510_wr_dat(0x0013);
		NT35510_wr_cmd(0xD312);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD313);	NT35510_wr_dat(0x0054);
		NT35510_wr_cmd(0xD314);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD315);	NT35510_wr_dat(0x0082);
		NT35510_wr_cmd(0xD316);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD317);	NT35510_wr_dat(0x00CA);
		NT35510_wr_cmd(0xD318);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD319);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD31A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD31B);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD31C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD31D);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD31E);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD31F);	NT35510_wr_dat(0x0067);
		NT35510_wr_cmd(0xD320);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD321);	NT35510_wr_dat(0x0084);
		NT35510_wr_cmd(0xD322);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD323);	NT35510_wr_dat(0x00A4);
		NT35510_wr_cmd(0xD324);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD325);	NT35510_wr_dat(0x00B7);
		NT35510_wr_cmd(0xD326);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD327);	NT35510_wr_dat(0x00CF);
		NT35510_wr_cmd(0xD328);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD329);	NT35510_wr_dat(0x00DE);
		NT35510_wr_cmd(0xD32A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD32B);	NT35510_wr_dat(0x00F2);
		NT35510_wr_cmd(0xD32C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD32D);	NT35510_wr_dat(0x00FE);
		NT35510_wr_cmd(0xD32E);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD32F);	NT35510_wr_dat(0x0010);
		NT35510_wr_cmd(0xD330);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD331);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD332);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD333);	NT35510_wr_dat(0x006D);
		/* Gamma (R-) */
		NT35510_wr_cmd(0xD400);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD401);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD402);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD403);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD404);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD405);	NT35510_wr_dat(0x003A);
		NT35510_wr_cmd(0xD406);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD407);	NT35510_wr_dat(0x004A);
		NT35510_wr_cmd(0xD408);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD409);	NT35510_wr_dat(0x005C);
		NT35510_wr_cmd(0xD40A);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD40B);	NT35510_wr_dat(0x0081);
		NT35510_wr_cmd(0xD40C);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD40D);	NT35510_wr_dat(0x00A6);
		NT35510_wr_cmd(0xD40E);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD40F);	NT35510_wr_dat(0x00E5);
		NT35510_wr_cmd(0xD410);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD411);	NT35510_wr_dat(0x0013);
		NT35510_wr_cmd(0xD412);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD413);	NT35510_wr_dat(0x0054);
		NT35510_wr_cmd(0xD414);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD415);	NT35510_wr_dat(0x0082);
		NT35510_wr_cmd(0xD416);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD417);	NT35510_wr_dat(0x00CA);
		NT35510_wr_cmd(0xD418);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD419);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD41A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD41B);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD41C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD41D);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD41E);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD41F);	NT35510_wr_dat(0x0067);
		NT35510_wr_cmd(0xD420);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD421);	NT35510_wr_dat(0x0084);
		NT35510_wr_cmd(0xD422);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD423);	NT35510_wr_dat(0x00A4);
		NT35510_wr_cmd(0xD424);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD425);	NT35510_wr_dat(0x00B7);
		NT35510_wr_cmd(0xD426);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD427);	NT35510_wr_dat(0x00CF);
		NT35510_wr_cmd(0xD428);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD429);	NT35510_wr_dat(0x00DE);
		NT35510_wr_cmd(0xD42A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD42B);	NT35510_wr_dat(0x00F2);
		NT35510_wr_cmd(0xD42C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD42D);	NT35510_wr_dat(0x00FE);
		NT35510_wr_cmd(0xD42E);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD42F);	NT35510_wr_dat(0x0010);
		NT35510_wr_cmd(0xD430);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD431);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD432);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD433);	NT35510_wr_dat(0x006D);
		/* Gamma (G-) */
		NT35510_wr_cmd(0xD500);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD501);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD502);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD503);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD504);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD505);	NT35510_wr_dat(0x003A);
		NT35510_wr_cmd(0xD506);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD507);	NT35510_wr_dat(0x004A);
		NT35510_wr_cmd(0xD508);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD509);	NT35510_wr_dat(0x005C);
		NT35510_wr_cmd(0xD50A);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD50B);	NT35510_wr_dat(0x0081);
		NT35510_wr_cmd(0xD50C);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD50D);	NT35510_wr_dat(0x00A6);
		NT35510_wr_cmd(0xD50E);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD50F);	NT35510_wr_dat(0x00E5);
		NT35510_wr_cmd(0xD510);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD511);	NT35510_wr_dat(0x0013);
		NT35510_wr_cmd(0xD512);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD513);	NT35510_wr_dat(0x0054);
		NT35510_wr_cmd(0xD514);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD515);	NT35510_wr_dat(0x0082);
		NT35510_wr_cmd(0xD516);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD517);	NT35510_wr_dat(0x00CA);
		NT35510_wr_cmd(0xD518);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD519);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD51A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD51B);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD51C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD51D);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD51E);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD51F);	NT35510_wr_dat(0x0067);
		NT35510_wr_cmd(0xD520);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD521);	NT35510_wr_dat(0x0084);
		NT35510_wr_cmd(0xD522);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD523);	NT35510_wr_dat(0x00A4);
		NT35510_wr_cmd(0xD524);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD525);	NT35510_wr_dat(0x00B7);
		NT35510_wr_cmd(0xD526);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD527);	NT35510_wr_dat(0x00CF);
		NT35510_wr_cmd(0xD528);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD529);	NT35510_wr_dat(0x00DE);
		NT35510_wr_cmd(0xD52A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD52B);	NT35510_wr_dat(0x00F2);
		NT35510_wr_cmd(0xD52C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD52D);	NT35510_wr_dat(0x00FE);
		NT35510_wr_cmd(0xD52E);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD52F);	NT35510_wr_dat(0x0010);
		NT35510_wr_cmd(0xD530);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD531);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD532);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD533);	NT35510_wr_dat(0x006D);
		/* Gamma (B-) */
		NT35510_wr_cmd(0xD600);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD601);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD602);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD603);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD604);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD605);	NT35510_wr_dat(0x003A);
		NT35510_wr_cmd(0xD606);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD607);	NT35510_wr_dat(0x004A);
		NT35510_wr_cmd(0xD608);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD609);	NT35510_wr_dat(0x005C);
		NT35510_wr_cmd(0xD60A);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD60B);	NT35510_wr_dat(0x0081);
		NT35510_wr_cmd(0xD60C);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD60D);	NT35510_wr_dat(0x00A6);
		NT35510_wr_cmd(0xD60E);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD60F);	NT35510_wr_dat(0x00E5);
		NT35510_wr_cmd(0xD610);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD611);	NT35510_wr_dat(0x0013);
		NT35510_wr_cmd(0xD612);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD613);	NT35510_wr_dat(0x0054);
		NT35510_wr_cmd(0xD614);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD615);	NT35510_wr_dat(0x0082);
		NT35510_wr_cmd(0xD616);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD617);	NT35510_wr_dat(0x00CA);
		NT35510_wr_cmd(0xD618);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD619);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0xD61A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD61B);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xD61C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD61D);	NT35510_wr_dat(0x0034);
		NT35510_wr_cmd(0xD61E);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD61F);	NT35510_wr_dat(0x0067);
		NT35510_wr_cmd(0xD620);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD621);	NT35510_wr_dat(0x0084);
		NT35510_wr_cmd(0xD622);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD623);	NT35510_wr_dat(0x00A4);
		NT35510_wr_cmd(0xD624);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD625);	NT35510_wr_dat(0x00B7);
		NT35510_wr_cmd(0xD626);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD627);	NT35510_wr_dat(0x00CF);
		NT35510_wr_cmd(0xD628);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD629);	NT35510_wr_dat(0x00DE);
		NT35510_wr_cmd(0xD62A);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD62B);	NT35510_wr_dat(0x00F2);
		NT35510_wr_cmd(0xD62C);	NT35510_wr_dat(0x0002);
		NT35510_wr_cmd(0xD62D);	NT35510_wr_dat(0x00FE);
		NT35510_wr_cmd(0xD62E);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD62F);	NT35510_wr_dat(0x0010);
		NT35510_wr_cmd(0xD630);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD631);	NT35510_wr_dat(0x0033);
		NT35510_wr_cmd(0xD632);	NT35510_wr_dat(0x0003);
		NT35510_wr_cmd(0xD633);	NT35510_wr_dat(0x006D);
		/* PAGE0 */
		NT35510_wr_cmd(0xF000);	NT35510_wr_dat(0x0055);
		NT35510_wr_cmd(0xF001);	NT35510_wr_dat(0x00AA);
		NT35510_wr_cmd(0xF002);	NT35510_wr_dat(0x0052);
		NT35510_wr_cmd(0xF003);	NT35510_wr_dat(0x0008);	
		NT35510_wr_cmd(0xF004);	NT35510_wr_dat(0x0000); 
		
		NT35510_wr_cmd(0xB400);	NT35510_wr_dat(0x0010);
		/* 480x800 */
		NT35510_wr_cmd(0xB500);	NT35510_wr_dat(0x0050);
		/* Dispay control */
		NT35510_wr_cmd(0xB100);	NT35510_wr_dat(0x00CC);	
		NT35510_wr_cmd(0xB101);	NT35510_wr_dat(0x0000);		/*  S1->S1440:00;S1440->S1:02 */
		/* Source hold time (Nova non-used) */
		NT35510_wr_cmd(0xB600);	NT35510_wr_dat(0x0005);
		/* Gate EQ control	 (Nova non-used) */
		NT35510_wr_cmd(0xB700);	NT35510_wr_dat(0x0077);  	/* HSD:70;Nova:77 */	 
		NT35510_wr_cmd(0xB701);	NT35510_wr_dat(0x0077);		/* HSD:70;Nova:77 */
		/* Source EQ control (Nova non-used) */
		NT35510_wr_cmd(0xB800);	NT35510_wr_dat(0x0001);  
		NT35510_wr_cmd(0xB801);	NT35510_wr_dat(0x0003);		/* HSD:05;Nova:07 */
		NT35510_wr_cmd(0xB802);	NT35510_wr_dat(0x0003);		/* HSD:05;Nova:07 */
		NT35510_wr_cmd(0xB803);	NT35510_wr_dat(0x0003);		/* HSD:05;Nova:07 */
		/* Inversion mode: column */
		NT35510_wr_cmd(0xBC00);	NT35510_wr_dat(0x0002);		/* 00: column */
		NT35510_wr_cmd(0xBC01);	NT35510_wr_dat(0x0000);		/* 01:1dot */
		NT35510_wr_cmd(0xBC02);	NT35510_wr_dat(0x0000); 
		/* Frame rate	(Nova non-used) */
		NT35510_wr_cmd(0xBD00);	NT35510_wr_dat(0x0001);
		NT35510_wr_cmd(0xBD01);	NT35510_wr_dat(0x0084);
		NT35510_wr_cmd(0xBD02);	NT35510_wr_dat(0x001C);		/* HSD:06;Nova:1C */
		NT35510_wr_cmd(0xBD03);	NT35510_wr_dat(0x001C);		/* HSD:04;Nova:1C */
		NT35510_wr_cmd(0xBD04);	NT35510_wr_dat(0x0000);
		/* LGD timing control(4H/4-Delay) */
		NT35510_wr_cmd(0xC900);	NT35510_wr_dat(0x00D0);		/* 3H:0x50;4H:0xD0 */
		NT35510_wr_cmd(0xC901);	NT35510_wr_dat(0x0002);		/* HSD:05;Nova:02 */
		NT35510_wr_cmd(0xC902);	NT35510_wr_dat(0x0050);		/* HSD:05;Nova:50 */
		NT35510_wr_cmd(0xC903);	NT35510_wr_dat(0x0050);		/* HSD:05;Nova:50	;STV delay time */
		NT35510_wr_cmd(0xC904);	NT35510_wr_dat(0x0050);		/* HSD:05;Nova:50	;CLK delay time */
#endif
		NT35510_wr_cmd(0x3600);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0x3500);	NT35510_wr_dat(0x0000);
		NT35510_wr_cmd(0x3A00);	NT35510_wr_dat(0x0055);		/* 55=65K   66=262K */
		
		/* Sleep out */
		NT35510_wr_cmd(0x1100);
		_delay_ms(150);	/* At least 120mSec */
		
		/* Display on */
		NT35510_wr_cmd(0x2900);
	}

	else { for(;;);} /* Invalid Device Code!! */

	NT35510_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	NT35510_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		NT35510_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
