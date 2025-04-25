/********************************************************************************/
/*!
	@file			rm68120.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KGM1066A0			(RM68120)	8/16bit mode.				@n
					 -FPC-CPTWV4058-JXY	(RM68180)	8bit mode only.

    @section HISTORY
		2016.06.01	V1.00	Stable Release.
		2016.11.04	V2.00	Revised read idcode routine.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "rm68120.h"
/* check header file version for fool proof */
#if RM68120_H != 0x0400
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
inline void RM68120_reset(void)
{
	RM68120_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	RM68120_RD_SET();
	RM68120_WR_SET();
	_delay_ms(20);								/* wait 20ms     			*/

	RM68120_RES_CLR();							/* RES=L, CS=L   			*/
	RM68120_CS_CLR();
	_delay_ms(1);								/* wait 1ms     			*/
	
	RM68120_RES_SET();						  	/* RES=H					*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void RM68120_wr_cmd(uint16_t cmd)
{
	RM68120_DC_CLR();							/* DC=L					*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	RM68120_CMD = (uint8_t)(cmd>>8);			/* upper 8bit command	*/
	RM68120_WR();								/* WR=L->H				*/
	RM68120_CMD = (uint8_t)cmd;					/* lower 8bit command	*/
#else
	RM68120_CMD = cmd;							/* 16bit command		*/
#endif
    RM68120_WR();								/* WR=L->H				*/

	RM68120_DC_SET();							/* DC=H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void RM68120_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	RM68120_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	RM68120_WR();								/* WR=L->H					*/
	RM68120_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	RM68120_DATA = dat;							/* 16bit data 				*/
#endif
	RM68120_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void RM68120_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		RM68120_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		RM68120_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void RM68120_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	RM68120_wr_cmd(0x2A00);				/* Horizontal RAM Start ADDR */
	RM68120_wr_dat((OFS_COL + x)>>8);
	RM68120_wr_cmd(0x2A01);
	RM68120_wr_dat(OFS_COL + x);
	RM68120_wr_cmd(0x2A02);				/* Horizontal RAM End ADDR */
	RM68120_wr_dat((OFS_COL + width)>>8);
	RM68120_wr_cmd(0x2A03);
	RM68120_wr_dat(OFS_COL + width);

	RM68120_wr_cmd(0x2B00);				/* Vertical RAM Start ADDR */
	RM68120_wr_dat((OFS_RAW + y)>>8);
	RM68120_wr_cmd(0x2B01);
	RM68120_wr_dat(OFS_RAW + y);
	RM68120_wr_cmd(0x2B02);				/* Vertical RAM End ADDR */
	RM68120_wr_dat((OFS_RAW + height)>>8);
	RM68120_wr_cmd(0x2B03);
	RM68120_wr_dat(OFS_RAW + height);

	RM68120_wr_cmd(0x2C00);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void RM68120_clear(void)
{
	volatile uint32_t n;

	RM68120_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		RM68120_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t RM68120_rd_cmd(uint16_t cmd)
{
	uint16_t val,temp;

	RM68120_wr_cmd(0xF000);	RM68120_wr_dat(0x0055);
	RM68120_wr_cmd(0xF001);	RM68120_wr_dat(0x00AA);
	RM68120_wr_cmd(0xF002);	RM68120_wr_dat(0x0052);
	RM68120_wr_cmd(0xF003);	RM68120_wr_dat(0x0008);
	RM68120_wr_cmd(0xF004);	RM68120_wr_dat(0x0001);

	RM68120_wr_cmd(cmd);
	RM68120_WR_SET();

    ReadLCDData(temp);

	RM68120_wr_cmd(cmd | 0x0001);
	RM68120_WR_SET();

    ReadLCDData(val);
	val <<= 8;
	
	val = (val | (0x00FF & temp));

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void RM68120_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	RM68120_reset();

	devicetype = RM68120_rd_cmd(0xC500);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8120)
	{
		/* ENABLE PAGE 1 */
		RM68120_wr_cmd(0xF000);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xF001);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xF002);	RM68120_wr_dat(0x0052);
		RM68120_wr_cmd(0xF003);	RM68120_wr_dat(0x0008);
		RM68120_wr_cmd(0xF004);	RM68120_wr_dat(0x0001);
		
		/* GAMMA SETING  RED */
		RM68120_wr_cmd(0xD100);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD101);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD102);	RM68120_wr_dat(0x001B);
		RM68120_wr_cmd(0xD103);	RM68120_wr_dat(0x0044);
		RM68120_wr_cmd(0xD104);	RM68120_wr_dat(0x0062);
		RM68120_wr_cmd(0xD105);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD106);	RM68120_wr_dat(0x007B);
		RM68120_wr_cmd(0xD107);	RM68120_wr_dat(0x00A1);
		RM68120_wr_cmd(0xD108);	RM68120_wr_dat(0x00C0);
		RM68120_wr_cmd(0xD109);	RM68120_wr_dat(0x00EE);
		RM68120_wr_cmd(0xD10A);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD10B);	RM68120_wr_dat(0x0010);
		RM68120_wr_cmd(0xD10C);	RM68120_wr_dat(0x002C);
		RM68120_wr_cmd(0xD10D);	RM68120_wr_dat(0x0043);
		RM68120_wr_cmd(0xD10E);	RM68120_wr_dat(0x0057);
		RM68120_wr_cmd(0xD10F);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD110);	RM68120_wr_dat(0x0068);
		RM68120_wr_cmd(0xD111);	RM68120_wr_dat(0x0078);
		RM68120_wr_cmd(0xD112);	RM68120_wr_dat(0x0087);
		RM68120_wr_cmd(0xD113);	RM68120_wr_dat(0x0094);
		RM68120_wr_cmd(0xD114);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD115);	RM68120_wr_dat(0x00A0);
		RM68120_wr_cmd(0xD116);	RM68120_wr_dat(0x00AC);
		RM68120_wr_cmd(0xD117);	RM68120_wr_dat(0x00B6);
		RM68120_wr_cmd(0xD118);	RM68120_wr_dat(0x00C1);
		RM68120_wr_cmd(0xD119);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD11A);	RM68120_wr_dat(0x00CB);
		RM68120_wr_cmd(0xD11B);	RM68120_wr_dat(0x00CD);
		RM68120_wr_cmd(0xD11C);	RM68120_wr_dat(0x00D6);
		RM68120_wr_cmd(0xD11D);	RM68120_wr_dat(0x00DF);
		RM68120_wr_cmd(0xD11E);	RM68120_wr_dat(0x0095);
		RM68120_wr_cmd(0xD11F);	RM68120_wr_dat(0x00E8);
		RM68120_wr_cmd(0xD120);	RM68120_wr_dat(0x00F1);
		RM68120_wr_cmd(0xD121);	RM68120_wr_dat(0x00FA);
		RM68120_wr_cmd(0xD122);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD123);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD124);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD125);	RM68120_wr_dat(0x0013);
		RM68120_wr_cmd(0xD126);	RM68120_wr_dat(0x001D);
		RM68120_wr_cmd(0xD127);	RM68120_wr_dat(0x0026);
		RM68120_wr_cmd(0xD128);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD129);	RM68120_wr_dat(0x0030);
		RM68120_wr_cmd(0xD12A);	RM68120_wr_dat(0x003C);
		RM68120_wr_cmd(0xD12B);	RM68120_wr_dat(0x004A);
		RM68120_wr_cmd(0xD12C);	RM68120_wr_dat(0x0063);
		RM68120_wr_cmd(0xD12D);	RM68120_wr_dat(0x00EA);
		RM68120_wr_cmd(0xD12E);	RM68120_wr_dat(0x0079);
		RM68120_wr_cmd(0xD12F);	RM68120_wr_dat(0x00A6);
		RM68120_wr_cmd(0xD130);	RM68120_wr_dat(0x00D0);
		RM68120_wr_cmd(0xD131);	RM68120_wr_dat(0x0020);
		RM68120_wr_cmd(0xD132);	RM68120_wr_dat(0x000F);
		RM68120_wr_cmd(0xD133);	RM68120_wr_dat(0x008E);
		RM68120_wr_cmd(0xD134);	RM68120_wr_dat(0x00FF);
		
		/* GAMMA SETING GREEN */
		RM68120_wr_cmd(0xD200);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD201);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD202);	RM68120_wr_dat(0x001B);
		RM68120_wr_cmd(0xD203);	RM68120_wr_dat(0x0044);
		RM68120_wr_cmd(0xD204);	RM68120_wr_dat(0x0062);
		RM68120_wr_cmd(0xD205);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD206);	RM68120_wr_dat(0x007B);
		RM68120_wr_cmd(0xD207);	RM68120_wr_dat(0x00A1);
		RM68120_wr_cmd(0xD208);	RM68120_wr_dat(0x00C0);
		RM68120_wr_cmd(0xD209);	RM68120_wr_dat(0x00EE);
		RM68120_wr_cmd(0xD20A);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD20B);	RM68120_wr_dat(0x0010);
		RM68120_wr_cmd(0xD20C);	RM68120_wr_dat(0x002C);
		RM68120_wr_cmd(0xD20D);	RM68120_wr_dat(0x0043);
		RM68120_wr_cmd(0xD20E);	RM68120_wr_dat(0x0057);
		RM68120_wr_cmd(0xD20F);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD210);	RM68120_wr_dat(0x0068);
		RM68120_wr_cmd(0xD211);	RM68120_wr_dat(0x0078);
		RM68120_wr_cmd(0xD212);	RM68120_wr_dat(0x0087);
		RM68120_wr_cmd(0xD213);	RM68120_wr_dat(0x0094);
		RM68120_wr_cmd(0xD214);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD215);	RM68120_wr_dat(0x00A0);
		RM68120_wr_cmd(0xD216);	RM68120_wr_dat(0x00AC);
		RM68120_wr_cmd(0xD217);	RM68120_wr_dat(0x00B6);
		RM68120_wr_cmd(0xD218);	RM68120_wr_dat(0x00C1);
		RM68120_wr_cmd(0xD219);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD21A);	RM68120_wr_dat(0x00CB);
		RM68120_wr_cmd(0xD21B);	RM68120_wr_dat(0x00CD);
		RM68120_wr_cmd(0xD21C);	RM68120_wr_dat(0x00D6);
		RM68120_wr_cmd(0xD21D);	RM68120_wr_dat(0x00DF);
		RM68120_wr_cmd(0xD21E);	RM68120_wr_dat(0x0095);
		RM68120_wr_cmd(0xD21F);	RM68120_wr_dat(0x00E8);
		RM68120_wr_cmd(0xD220);	RM68120_wr_dat(0x00F1);
		RM68120_wr_cmd(0xD221);	RM68120_wr_dat(0x00FA);
		RM68120_wr_cmd(0xD222);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD223);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD224);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD225);	RM68120_wr_dat(0x0013);
		RM68120_wr_cmd(0xD226);	RM68120_wr_dat(0x001D);
		RM68120_wr_cmd(0xD227);	RM68120_wr_dat(0x0026);
		RM68120_wr_cmd(0xD228);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD229);	RM68120_wr_dat(0x0030);
		RM68120_wr_cmd(0xD22A);	RM68120_wr_dat(0x003C);
		RM68120_wr_cmd(0xD22B);	RM68120_wr_dat(0x004A);
		RM68120_wr_cmd(0xD22C);	RM68120_wr_dat(0x0063);
		RM68120_wr_cmd(0xD22D);	RM68120_wr_dat(0x00EA);
		RM68120_wr_cmd(0xD22E);	RM68120_wr_dat(0x0079);
		RM68120_wr_cmd(0xD22F);	RM68120_wr_dat(0x00A6);
		RM68120_wr_cmd(0xD230);	RM68120_wr_dat(0x00D0);
		RM68120_wr_cmd(0xD231);	RM68120_wr_dat(0x0020);
		RM68120_wr_cmd(0xD232);	RM68120_wr_dat(0x000F);
		RM68120_wr_cmd(0xD233);	RM68120_wr_dat(0x008E);
		RM68120_wr_cmd(0xD234);	RM68120_wr_dat(0x00FF);
		
		/* GAMMA SETING BLUE */
		RM68120_wr_cmd(0xD300);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD301);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD302);	RM68120_wr_dat(0x001B);
		RM68120_wr_cmd(0xD303);	RM68120_wr_dat(0x0044);
		RM68120_wr_cmd(0xD304);	RM68120_wr_dat(0x0062);
		RM68120_wr_cmd(0xD305);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD306);	RM68120_wr_dat(0x007B);
		RM68120_wr_cmd(0xD307);	RM68120_wr_dat(0x00A1);
		RM68120_wr_cmd(0xD308);	RM68120_wr_dat(0x00C0);
		RM68120_wr_cmd(0xD309);	RM68120_wr_dat(0x00EE);
		RM68120_wr_cmd(0xD30A);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD30B);	RM68120_wr_dat(0x0010);
		RM68120_wr_cmd(0xD30C);	RM68120_wr_dat(0x002C);
		RM68120_wr_cmd(0xD30D);	RM68120_wr_dat(0x0043);
		RM68120_wr_cmd(0xD30E);	RM68120_wr_dat(0x0057);
		RM68120_wr_cmd(0xD30F);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD310);	RM68120_wr_dat(0x0068);
		RM68120_wr_cmd(0xD311);	RM68120_wr_dat(0x0078);
		RM68120_wr_cmd(0xD312);	RM68120_wr_dat(0x0087);
		RM68120_wr_cmd(0xD313);	RM68120_wr_dat(0x0094);
		RM68120_wr_cmd(0xD314);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD315);	RM68120_wr_dat(0x00A0);
		RM68120_wr_cmd(0xD316);	RM68120_wr_dat(0x00AC);
		RM68120_wr_cmd(0xD317);	RM68120_wr_dat(0x00B6);
		RM68120_wr_cmd(0xD318);	RM68120_wr_dat(0x00C1);
		RM68120_wr_cmd(0xD319);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD31A);	RM68120_wr_dat(0x00CB);
		RM68120_wr_cmd(0xD31B);	RM68120_wr_dat(0x00CD);
		RM68120_wr_cmd(0xD31C);	RM68120_wr_dat(0x00D6);
		RM68120_wr_cmd(0xD31D);	RM68120_wr_dat(0x00DF);
		RM68120_wr_cmd(0xD31E);	RM68120_wr_dat(0x0095);
		RM68120_wr_cmd(0xD31F);	RM68120_wr_dat(0x00E8);
		RM68120_wr_cmd(0xD320);	RM68120_wr_dat(0x00F1);
		RM68120_wr_cmd(0xD321);	RM68120_wr_dat(0x00FA);
		RM68120_wr_cmd(0xD322);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD323);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD324);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD325);	RM68120_wr_dat(0x0013);
		RM68120_wr_cmd(0xD326);	RM68120_wr_dat(0x001D);
		RM68120_wr_cmd(0xD327);	RM68120_wr_dat(0x0026);
		RM68120_wr_cmd(0xD328);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD329);	RM68120_wr_dat(0x0030);
		RM68120_wr_cmd(0xD32A);	RM68120_wr_dat(0x003C);
		RM68120_wr_cmd(0xD32B);	RM68120_wr_dat(0x004A);
		RM68120_wr_cmd(0xD32C);	RM68120_wr_dat(0x0063);
		RM68120_wr_cmd(0xD32D);	RM68120_wr_dat(0x00EA);
		RM68120_wr_cmd(0xD32E);	RM68120_wr_dat(0x0079);
		RM68120_wr_cmd(0xD32F);	RM68120_wr_dat(0x00A6);
		RM68120_wr_cmd(0xD330);	RM68120_wr_dat(0x00D0);
		RM68120_wr_cmd(0xD331);	RM68120_wr_dat(0x0020);
		RM68120_wr_cmd(0xD332);	RM68120_wr_dat(0x000F);
		RM68120_wr_cmd(0xD333);	RM68120_wr_dat(0x008E);
		RM68120_wr_cmd(0xD334);	RM68120_wr_dat(0x00FF);
		
		/* GAMMA SETING  RED */
		RM68120_wr_cmd(0xD400);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD401);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD402);	RM68120_wr_dat(0x001B);
		RM68120_wr_cmd(0xD403);	RM68120_wr_dat(0x0044);
		RM68120_wr_cmd(0xD404);	RM68120_wr_dat(0x0062);
		RM68120_wr_cmd(0xD405);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD406);	RM68120_wr_dat(0x007B);
		RM68120_wr_cmd(0xD407);	RM68120_wr_dat(0x00A1);
		RM68120_wr_cmd(0xD408);	RM68120_wr_dat(0x00C0);
		RM68120_wr_cmd(0xD409);	RM68120_wr_dat(0x00EE);
		RM68120_wr_cmd(0xD40A);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD40B);	RM68120_wr_dat(0x0010);
		RM68120_wr_cmd(0xD40C);	RM68120_wr_dat(0x002C);
		RM68120_wr_cmd(0xD40D);	RM68120_wr_dat(0x0043);
		RM68120_wr_cmd(0xD40E);	RM68120_wr_dat(0x0057);
		RM68120_wr_cmd(0xD40F);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD410);	RM68120_wr_dat(0x0068);
		RM68120_wr_cmd(0xD411);	RM68120_wr_dat(0x0078);
		RM68120_wr_cmd(0xD412);	RM68120_wr_dat(0x0087);
		RM68120_wr_cmd(0xD413);	RM68120_wr_dat(0x0094);
		RM68120_wr_cmd(0xD414);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD415);	RM68120_wr_dat(0x00A0);
		RM68120_wr_cmd(0xD416);	RM68120_wr_dat(0x00AC);
		RM68120_wr_cmd(0xD417);	RM68120_wr_dat(0x00B6);
		RM68120_wr_cmd(0xD418);	RM68120_wr_dat(0x00C1);
		RM68120_wr_cmd(0xD419);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD41A);	RM68120_wr_dat(0x00CB);
		RM68120_wr_cmd(0xD41B);	RM68120_wr_dat(0x00CD);
		RM68120_wr_cmd(0xD41C);	RM68120_wr_dat(0x00D6);
		RM68120_wr_cmd(0xD41D);	RM68120_wr_dat(0x00DF);
		RM68120_wr_cmd(0xD41E);	RM68120_wr_dat(0x0095);
		RM68120_wr_cmd(0xD41F);	RM68120_wr_dat(0x00E8);
		RM68120_wr_cmd(0xD420);	RM68120_wr_dat(0x00F1);
		RM68120_wr_cmd(0xD421);	RM68120_wr_dat(0x00FA);
		RM68120_wr_cmd(0xD422);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD423);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD424);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD425);	RM68120_wr_dat(0x0013);
		RM68120_wr_cmd(0xD426);	RM68120_wr_dat(0x001D);
		RM68120_wr_cmd(0xD427);	RM68120_wr_dat(0x0026);
		RM68120_wr_cmd(0xD428);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD429);	RM68120_wr_dat(0x0030);
		RM68120_wr_cmd(0xD42A);	RM68120_wr_dat(0x003C);
		RM68120_wr_cmd(0xD42B);	RM68120_wr_dat(0x004A);
		RM68120_wr_cmd(0xD42C);	RM68120_wr_dat(0x0063);
		RM68120_wr_cmd(0xD42D);	RM68120_wr_dat(0x00EA);
		RM68120_wr_cmd(0xD42E);	RM68120_wr_dat(0x0079);
		RM68120_wr_cmd(0xD42F);	RM68120_wr_dat(0x00A6);
		RM68120_wr_cmd(0xD430);	RM68120_wr_dat(0x00D0);
		RM68120_wr_cmd(0xD431);	RM68120_wr_dat(0x0020);
		RM68120_wr_cmd(0xD432);	RM68120_wr_dat(0x000F);
		RM68120_wr_cmd(0xD433);	RM68120_wr_dat(0x008E);
		RM68120_wr_cmd(0xD434);	RM68120_wr_dat(0x00FF);
		
		/* GAMMA SETING GREEN */
		RM68120_wr_cmd(0xD500);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD501);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD502);	RM68120_wr_dat(0x001B);
		RM68120_wr_cmd(0xD503);	RM68120_wr_dat(0x0044);
		RM68120_wr_cmd(0xD504);	RM68120_wr_dat(0x0062);
		RM68120_wr_cmd(0xD505);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD506);	RM68120_wr_dat(0x007B);
		RM68120_wr_cmd(0xD507);	RM68120_wr_dat(0x00A1);
		RM68120_wr_cmd(0xD508);	RM68120_wr_dat(0x00C0);
		RM68120_wr_cmd(0xD509);	RM68120_wr_dat(0x00EE);
		RM68120_wr_cmd(0xD50A);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD50B);	RM68120_wr_dat(0x0010);
		RM68120_wr_cmd(0xD50C);	RM68120_wr_dat(0x002C);
		RM68120_wr_cmd(0xD50D);	RM68120_wr_dat(0x0043);
		RM68120_wr_cmd(0xD50E);	RM68120_wr_dat(0x0057);
		RM68120_wr_cmd(0xD50F);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD510);	RM68120_wr_dat(0x0068);
		RM68120_wr_cmd(0xD511);	RM68120_wr_dat(0x0078);
		RM68120_wr_cmd(0xD512);	RM68120_wr_dat(0x0087);
		RM68120_wr_cmd(0xD513);	RM68120_wr_dat(0x0094);
		RM68120_wr_cmd(0xD514);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD515);	RM68120_wr_dat(0x00A0);
		RM68120_wr_cmd(0xD516);	RM68120_wr_dat(0x00AC);
		RM68120_wr_cmd(0xD517);	RM68120_wr_dat(0x00B6);
		RM68120_wr_cmd(0xD518);	RM68120_wr_dat(0x00C1);
		RM68120_wr_cmd(0xD519);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD51A);	RM68120_wr_dat(0x00CB);
		RM68120_wr_cmd(0xD51B);	RM68120_wr_dat(0x00CD);
		RM68120_wr_cmd(0xD51C);	RM68120_wr_dat(0x00D6);
		RM68120_wr_cmd(0xD51D);	RM68120_wr_dat(0x00DF);
		RM68120_wr_cmd(0xD51E);	RM68120_wr_dat(0x0095);
		RM68120_wr_cmd(0xD51F);	RM68120_wr_dat(0x00E8);
		RM68120_wr_cmd(0xD520);	RM68120_wr_dat(0x00F1);
		RM68120_wr_cmd(0xD521);	RM68120_wr_dat(0x00FA);
		RM68120_wr_cmd(0xD522);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD523);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD524);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD525);	RM68120_wr_dat(0x0013);
		RM68120_wr_cmd(0xD526);	RM68120_wr_dat(0x001D);
		RM68120_wr_cmd(0xD527);	RM68120_wr_dat(0x0026);
		RM68120_wr_cmd(0xD528);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD529);	RM68120_wr_dat(0x0030);
		RM68120_wr_cmd(0xD52A);	RM68120_wr_dat(0x003C);
		RM68120_wr_cmd(0xD52B);	RM68120_wr_dat(0x004A);
		RM68120_wr_cmd(0xD52C);	RM68120_wr_dat(0x0063);
		RM68120_wr_cmd(0xD52D);	RM68120_wr_dat(0x00EA);
		RM68120_wr_cmd(0xD52E);	RM68120_wr_dat(0x0079);
		RM68120_wr_cmd(0xD52F);	RM68120_wr_dat(0x00A6);
		RM68120_wr_cmd(0xD530);	RM68120_wr_dat(0x00D0);
		RM68120_wr_cmd(0xD531);	RM68120_wr_dat(0x0020);
		RM68120_wr_cmd(0xD532);	RM68120_wr_dat(0x000F);
		RM68120_wr_cmd(0xD533);	RM68120_wr_dat(0x008E);
		RM68120_wr_cmd(0xD534);	RM68120_wr_dat(0x00FF);
		
		/* GAMMA SETING BLUE */
		RM68120_wr_cmd(0xD600);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD601);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD602);	RM68120_wr_dat(0x001B);
		RM68120_wr_cmd(0xD603);	RM68120_wr_dat(0x0044);
		RM68120_wr_cmd(0xD604);	RM68120_wr_dat(0x0062);
		RM68120_wr_cmd(0xD605);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD606);	RM68120_wr_dat(0x007B);
		RM68120_wr_cmd(0xD607);	RM68120_wr_dat(0x00A1);
		RM68120_wr_cmd(0xD608);	RM68120_wr_dat(0x00C0);
		RM68120_wr_cmd(0xD609);	RM68120_wr_dat(0x00EE);
		RM68120_wr_cmd(0xD60A);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD60B);	RM68120_wr_dat(0x0010);
		RM68120_wr_cmd(0xD60C);	RM68120_wr_dat(0x002C);
		RM68120_wr_cmd(0xD60D);	RM68120_wr_dat(0x0043);
		RM68120_wr_cmd(0xD60E);	RM68120_wr_dat(0x0057);
		RM68120_wr_cmd(0xD60F);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD610);	RM68120_wr_dat(0x0068);
		RM68120_wr_cmd(0xD611);	RM68120_wr_dat(0x0078);
		RM68120_wr_cmd(0xD612);	RM68120_wr_dat(0x0087);
		RM68120_wr_cmd(0xD613);	RM68120_wr_dat(0x0094);
		RM68120_wr_cmd(0xD614);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD615);	RM68120_wr_dat(0x00A0);
		RM68120_wr_cmd(0xD616);	RM68120_wr_dat(0x00AC);
		RM68120_wr_cmd(0xD617);	RM68120_wr_dat(0x00B6);
		RM68120_wr_cmd(0xD618);	RM68120_wr_dat(0x00C1);
		RM68120_wr_cmd(0xD619);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD61A);	RM68120_wr_dat(0x00CB);
		RM68120_wr_cmd(0xD61B);	RM68120_wr_dat(0x00CD);
		RM68120_wr_cmd(0xD61C);	RM68120_wr_dat(0x00D6);
		RM68120_wr_cmd(0xD61D);	RM68120_wr_dat(0x00DF);
		RM68120_wr_cmd(0xD61E);	RM68120_wr_dat(0x0095);
		RM68120_wr_cmd(0xD61F);	RM68120_wr_dat(0x00E8);
		RM68120_wr_cmd(0xD620);	RM68120_wr_dat(0x00F1);
		RM68120_wr_cmd(0xD621);	RM68120_wr_dat(0x00FA);
		RM68120_wr_cmd(0xD622);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD623);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD624);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD625);	RM68120_wr_dat(0x0013);
		RM68120_wr_cmd(0xD626);	RM68120_wr_dat(0x001D);
		RM68120_wr_cmd(0xD627);	RM68120_wr_dat(0x0026);
		RM68120_wr_cmd(0xD628);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xD629);	RM68120_wr_dat(0x0030);
		RM68120_wr_cmd(0xD62A);	RM68120_wr_dat(0x003C);
		RM68120_wr_cmd(0xD62B);	RM68120_wr_dat(0x004A);
		RM68120_wr_cmd(0xD62C);	RM68120_wr_dat(0x0063);
		RM68120_wr_cmd(0xD62D);	RM68120_wr_dat(0x00EA);
		RM68120_wr_cmd(0xD62E);	RM68120_wr_dat(0x0079);
		RM68120_wr_cmd(0xD62F);	RM68120_wr_dat(0x00A6);
		RM68120_wr_cmd(0xD630);	RM68120_wr_dat(0x00D0);
		RM68120_wr_cmd(0xD631);	RM68120_wr_dat(0x0020);
		RM68120_wr_cmd(0xD632);	RM68120_wr_dat(0x000F);
		RM68120_wr_cmd(0xD633);	RM68120_wr_dat(0x008E);
		RM68120_wr_cmd(0xD634);	RM68120_wr_dat(0x00FF);
		
		/* AVDD VOLTAGE SETTING */
		RM68120_wr_cmd(0xB000);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB001);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB002);	RM68120_wr_dat(0x0005);
		
		/* AVEE VOLTAGE SETTING */
		RM68120_wr_cmd(0xB100);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB101);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB102);	RM68120_wr_dat(0x0005);
		
		/* AVDD Boosting */
		RM68120_wr_cmd(0xB600);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB601);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB603);	RM68120_wr_dat(0x0034);
		
		/* AVEE Boosting */
		RM68120_wr_cmd(0xB700);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB701);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB702);	RM68120_wr_dat(0x0024);
		
		/* VCL Boosting */
		RM68120_wr_cmd(0xB800);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB801);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB802);	RM68120_wr_dat(0x0024);
		
		/* VGLX VOLTAGE SETTING */
		RM68120_wr_cmd(0xBA00);	RM68120_wr_dat(0x0014);
		RM68120_wr_cmd(0xBA01);	RM68120_wr_dat(0x0014);
		RM68120_wr_cmd(0xBA02);	RM68120_wr_dat(0x0014);
		
		/* VCL Boosting */
		RM68120_wr_cmd(0xB900);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB901);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB902);	RM68120_wr_dat(0x0024);
		
		/* Gamma Voltage */
		RM68120_wr_cmd(0xBC00);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xBC01);	RM68120_wr_dat(0x00A0);		/* vgmp=5.0 */
		RM68120_wr_cmd(0xBC02);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xBD00);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xBD01);	RM68120_wr_dat(0x00A0);		/* vgmn=5.0 */
		RM68120_wr_cmd(0xBD02);	RM68120_wr_dat(0x0000);
		
		/* VCOM Setting */
		RM68120_wr_cmd(0xBE01);	RM68120_wr_dat(0x003D);		/* 3 */
		
		/* ENABLE PAGE 0 */
		RM68120_wr_cmd(0xF000);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xF001);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xF002);	RM68120_wr_dat(0x0052);
		RM68120_wr_cmd(0xF003);	RM68120_wr_dat(0x0008);
		RM68120_wr_cmd(0xF004);	RM68120_wr_dat(0x0000);
		
		/* Vivid Color FuNCTIon Control */
		RM68120_wr_cmd(0xB400);	RM68120_wr_dat(0x0010);
		
		/* Z-INVERSION */
		RM68120_wr_cmd(0xBC00);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xBC01);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xBC02);	RM68120_wr_dat(0x0005);
		
		/**************** ADD on 20111021**********************/
		RM68120_wr_cmd(0xB700);	RM68120_wr_dat(0x0022);		/* GATE EQ CONTROL */
		RM68120_wr_cmd(0xB701);	RM68120_wr_dat(0x0022);		/* GATE EQ CONTROL */
		
		RM68120_wr_cmd(0xC80B);	RM68120_wr_dat(0x002A);		/* DISPLAY TIMING CONTROL */
		RM68120_wr_cmd(0xC80C);	RM68120_wr_dat(0x002A);		/* DISPLAY TIMING CONTROL */
		RM68120_wr_cmd(0xC80F);	RM68120_wr_dat(0x002A);		/* DISPLAY TIMING CONTROL */
		RM68120_wr_cmd(0xC810);	RM68120_wr_dat(0x002A);		/* DISPLAY TIMING CONTROL */
		/**************** ADD on 20111021**********************/
		/* PWM_ENH_OE =1 */
		RM68120_wr_cmd(0xD000);	RM68120_wr_dat(0x0001);
		/* DM_SEL =1 */
		RM68120_wr_cmd(0xB300);	RM68120_wr_dat(0x0010);
		/* VBPDA=07h */
		RM68120_wr_cmd(0xBD02);	RM68120_wr_dat(0x0007);
		/* VBPDb=07h */
		RM68120_wr_cmd(0xBE02);	RM68120_wr_dat(0x0007);
		/* VBPDc=07h */
		RM68120_wr_cmd(0xBF02);	RM68120_wr_dat(0x0007);
		/* ENABLE PAGE 2 */
		RM68120_wr_cmd(0xF000);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xF001);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xF002);	RM68120_wr_dat(0x0052);
		RM68120_wr_cmd(0xF003);	RM68120_wr_dat(0x0008);
		RM68120_wr_cmd(0xF004);	RM68120_wr_dat(0x0002);
		/* SDREG0 =0 */
		RM68120_wr_cmd(0xC301);	RM68120_wr_dat(0x00A9);
		/* DS=14 */
		RM68120_wr_cmd(0xFE01);	RM68120_wr_dat(0x0094);
		/* OSC =60h */
		RM68120_wr_cmd(0xF600);	RM68120_wr_dat(0x0060);
		/* TE ON */
		RM68120_wr_cmd(0x3500);	RM68120_wr_dat(0x0000);
		
		RM68120_wr_cmd(0x3A00);	RM68120_wr_dat(0x0055);		/* 55=65K   66=262K */
		
		/* SLEEP OUT */
		RM68120_wr_cmd(0x1100);
		_delay_ms(120);
		
		/* DISPLY ON */
		RM68120_wr_cmd(0x2900);
		_delay_ms(100);
	}

	else if(devicetype == 0x6818)
	{
		/* Initialize RM68180 */
		/* Start Initial Sequence */
		/* LV2 Page 1 enable */
		RM68120_wr_cmd(0xF000);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xF001);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xF002);	RM68120_wr_dat(0x0052);
		RM68120_wr_cmd(0xF003);	RM68120_wr_dat(0x0008);
		RM68120_wr_cmd(0xF004);	RM68120_wr_dat(0x0001);
		
		RM68120_wr_cmd(0x0E00);	RM68120_wr_dat(0x0027);
		
		/* AVDD Set AVDD 5.2V */
		RM68120_wr_cmd(0xB000);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB001);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB002);	RM68120_wr_dat(0x0005);
		
		/* AVEE -5.2V */
		RM68120_wr_cmd(0xB100);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB101);	RM68120_wr_dat(0x0005);
		RM68120_wr_cmd(0xB102);	RM68120_wr_dat(0x0005);
		
		/* AVDD ratio */
		RM68120_wr_cmd(0xB600);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB601);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB602);	RM68120_wr_dat(0x0034);
		
		/* AVEE ratio */
		RM68120_wr_cmd(0xB700);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB701);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB702);	RM68120_wr_dat(0x0034);
		
		/* VCL ratio */
		RM68120_wr_cmd(0xB800);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB801);	RM68120_wr_dat(0x0024);
		RM68120_wr_cmd(0xB802);	RM68120_wr_dat(0x0024);
		
		/* VGH ratio */
		RM68120_wr_cmd(0xB900);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB901);	RM68120_wr_dat(0x0034);
		RM68120_wr_cmd(0xB902);	RM68120_wr_dat(0x0034);
		
		/* VGLX tatio */
		RM68120_wr_cmd(0xBA00);	RM68120_wr_dat(0x0004);
		RM68120_wr_cmd(0xBA01);	RM68120_wr_dat(0x0004);
		RM68120_wr_cmd(0xBA02);	RM68120_wr_dat(0x0004);
		
		/* VGMN/VGSN 4.5V/0V */
		RM68120_wr_cmd(0xBC00);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xBC01);	RM68120_wr_dat(0x0096);
		RM68120_wr_cmd(0xBC02);	RM68120_wr_dat(0x0002);
		
		/* VGMN/VGSN -4.5V/0V */
		RM68120_wr_cmd(0xBD00);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xBD01);	RM68120_wr_dat(0x0098);
		RM68120_wr_cmd(0xBD02);	RM68120_wr_dat(0x0000);
		
		/* VCOM -1.25V */
		RM68120_wr_cmd(0xBE00);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xBE01);	RM68120_wr_dat(0x0075);
		
		RM68120_wr_cmd(0xCC00);	RM68120_wr_dat(0x0005);
	
		/* Gamma Setting */
		RM68120_wr_cmd(0xD100);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD101);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD102);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD103);	RM68120_wr_dat(0x0006);
		RM68120_wr_cmd(0xD104);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD105);	RM68120_wr_dat(0x001A);
		RM68120_wr_cmd(0xD106);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD107);	RM68120_wr_dat(0x003F);
		RM68120_wr_cmd(0xD108);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD109);	RM68120_wr_dat(0x0066);
		RM68120_wr_cmd(0xD10A);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD10B);	RM68120_wr_dat(0x00AD);
		RM68120_wr_cmd(0xD10C);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD10D);	RM68120_wr_dat(0x00E1);
		RM68120_wr_cmd(0xD10E);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD10F);	RM68120_wr_dat(0x0029);
		RM68120_wr_cmd(0xD110);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD111);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD112);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD113);	RM68120_wr_dat(0x0097);
		RM68120_wr_cmd(0xD114);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD115);	RM68120_wr_dat(0x00C2);
		RM68120_wr_cmd(0xD116);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD117);	RM68120_wr_dat(0x00FF);
		RM68120_wr_cmd(0xD118);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD119);	RM68120_wr_dat(0x002D);
		RM68120_wr_cmd(0xD11A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD11B);	RM68120_wr_dat(0x002F);
		RM68120_wr_cmd(0xD11C);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD11D);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD11E);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD11F);	RM68120_wr_dat(0x0081);
		RM68120_wr_cmd(0xD120);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD121);	RM68120_wr_dat(0x0098);
		RM68120_wr_cmd(0xD122);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD123);	RM68120_wr_dat(0x00B3);
		RM68120_wr_cmd(0xD124);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD125);	RM68120_wr_dat(0x00C4);
		RM68120_wr_cmd(0xD126);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD127);	RM68120_wr_dat(0x00DB);
		RM68120_wr_cmd(0xD128);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD129);	RM68120_wr_dat(0x00E9);
		RM68120_wr_cmd(0xD12A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD12B);	RM68120_wr_dat(0x00FD);
		RM68120_wr_cmd(0xD12C);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD12D);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD12E);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD12F);	RM68120_wr_dat(0x0021);
		RM68120_wr_cmd(0xD130);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD131);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD132);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD133);	RM68120_wr_dat(0x00F8);
		
		RM68120_wr_cmd(0xD200);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD201);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD202);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD203);	RM68120_wr_dat(0x0006);
		RM68120_wr_cmd(0xD204);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD205);	RM68120_wr_dat(0x001A);
		RM68120_wr_cmd(0xD206);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD207);	RM68120_wr_dat(0x003F);
		RM68120_wr_cmd(0xD208);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD209);	RM68120_wr_dat(0x0066);
		RM68120_wr_cmd(0xD20A);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD20B);	RM68120_wr_dat(0x00AD);
		RM68120_wr_cmd(0xD20C);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD20D);	RM68120_wr_dat(0x00E1);
		RM68120_wr_cmd(0xD20E);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD20F);	RM68120_wr_dat(0x0029);
		RM68120_wr_cmd(0xD210);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD211);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD212);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD213);	RM68120_wr_dat(0x0097);
		RM68120_wr_cmd(0xD214);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD215);	RM68120_wr_dat(0x00C2);
		RM68120_wr_cmd(0xD216);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD217);	RM68120_wr_dat(0x00FF);
		RM68120_wr_cmd(0xD218);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD219);	RM68120_wr_dat(0x002D);
		RM68120_wr_cmd(0xD21A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD21B);	RM68120_wr_dat(0x002F);
		RM68120_wr_cmd(0xD21C);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD21D);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD21E);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD21F);	RM68120_wr_dat(0x0081);
		RM68120_wr_cmd(0xD220);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD221);	RM68120_wr_dat(0x0098);
		RM68120_wr_cmd(0xD222);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD223);	RM68120_wr_dat(0x00B3);
		RM68120_wr_cmd(0xD224);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD225);	RM68120_wr_dat(0x00C4);
		RM68120_wr_cmd(0xD226);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD227);	RM68120_wr_dat(0x00DB);
		RM68120_wr_cmd(0xD228);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD229);	RM68120_wr_dat(0x00E9);
		RM68120_wr_cmd(0xD22A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD22B);	RM68120_wr_dat(0x00FD);
		RM68120_wr_cmd(0xD22C);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD22D);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD22E);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD22F);	RM68120_wr_dat(0x0021);
		RM68120_wr_cmd(0xD230);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD231);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD232);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD233);	RM68120_wr_dat(0x00F8);
		
		RM68120_wr_cmd(0xD300);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD301);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD302);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD303);	RM68120_wr_dat(0x0006);
		RM68120_wr_cmd(0xD304);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD305);	RM68120_wr_dat(0x001A);
		RM68120_wr_cmd(0xD306);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD307);	RM68120_wr_dat(0x003F);
		RM68120_wr_cmd(0xD308);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD309);	RM68120_wr_dat(0x0066);
		RM68120_wr_cmd(0xD30A);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD30B);	RM68120_wr_dat(0x00AD);
		RM68120_wr_cmd(0xD30C);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD30D);	RM68120_wr_dat(0x00E1);
		RM68120_wr_cmd(0xD30E);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD30F);	RM68120_wr_dat(0x0029);
		RM68120_wr_cmd(0xD310);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD311);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD312);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD313);	RM68120_wr_dat(0x0097);
		RM68120_wr_cmd(0xD314);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD315);	RM68120_wr_dat(0x00C2);
		RM68120_wr_cmd(0xD316);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD317);	RM68120_wr_dat(0x00FF);
		RM68120_wr_cmd(0xD318);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD319);	RM68120_wr_dat(0x002D);
		RM68120_wr_cmd(0xD31A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD31B);	RM68120_wr_dat(0x002F);
		RM68120_wr_cmd(0xD31C);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD31D);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD31E);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD31F);	RM68120_wr_dat(0x0081);
		RM68120_wr_cmd(0xD320);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD321);	RM68120_wr_dat(0x0098);
		RM68120_wr_cmd(0xD322);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD323);	RM68120_wr_dat(0x00B3);
		RM68120_wr_cmd(0xD324);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD325);	RM68120_wr_dat(0x00C4);
		RM68120_wr_cmd(0xD326);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD327);	RM68120_wr_dat(0x00DB);
		RM68120_wr_cmd(0xD328);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD329);	RM68120_wr_dat(0x00E9);
		RM68120_wr_cmd(0xD32A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD32B);	RM68120_wr_dat(0x00FD);
		RM68120_wr_cmd(0xD32C);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD32D);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD32E);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD32F);	RM68120_wr_dat(0x0021);
		RM68120_wr_cmd(0xD330);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD331);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD332);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD333);	RM68120_wr_dat(0x00F8);
		
		RM68120_wr_cmd(0xD400);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD401);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD402);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD403);	RM68120_wr_dat(0x0006);
		RM68120_wr_cmd(0xD404);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD405);	RM68120_wr_dat(0x001A);
		RM68120_wr_cmd(0xD406);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD407);	RM68120_wr_dat(0x003F);
		RM68120_wr_cmd(0xD408);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD409);	RM68120_wr_dat(0x0066);
		RM68120_wr_cmd(0xD40A);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD40B);	RM68120_wr_dat(0x00AD);
		RM68120_wr_cmd(0xD40C);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD40D);	RM68120_wr_dat(0x00E1);
		RM68120_wr_cmd(0xD40E);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD40F);	RM68120_wr_dat(0x0029);
		RM68120_wr_cmd(0xD410);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD411);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD412);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD413);	RM68120_wr_dat(0x0097);
		RM68120_wr_cmd(0xD414);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD415);	RM68120_wr_dat(0x00C2);
		RM68120_wr_cmd(0xD416);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD417);	RM68120_wr_dat(0x00FF);
		RM68120_wr_cmd(0xD418);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD419);	RM68120_wr_dat(0x002D);
		RM68120_wr_cmd(0xD41A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD41B);	RM68120_wr_dat(0x002F);
		RM68120_wr_cmd(0xD41C);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD41D);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD41E);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD41F);	RM68120_wr_dat(0x0081);
		RM68120_wr_cmd(0xD420);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD421);	RM68120_wr_dat(0x0098);
		RM68120_wr_cmd(0xD422);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD423);	RM68120_wr_dat(0x00B3);
		RM68120_wr_cmd(0xD424);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD425);	RM68120_wr_dat(0x00C4);
		RM68120_wr_cmd(0xD426);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD427);	RM68120_wr_dat(0x00DB);
		RM68120_wr_cmd(0xD428);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD429);	RM68120_wr_dat(0x00E9);
		RM68120_wr_cmd(0xD42A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD42B);	RM68120_wr_dat(0x00FD);
		RM68120_wr_cmd(0xD42C);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD42D);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD42E);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD42F);	RM68120_wr_dat(0x0021);
		RM68120_wr_cmd(0xD430);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD431);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD432);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD433);	RM68120_wr_dat(0x00F8);
		
		RM68120_wr_cmd(0xD500);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD501);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD502);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD503);	RM68120_wr_dat(0x0006);
		RM68120_wr_cmd(0xD504);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD505);	RM68120_wr_dat(0x001A);
		RM68120_wr_cmd(0xD506);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD507);	RM68120_wr_dat(0x003F);
		RM68120_wr_cmd(0xD508);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD509);	RM68120_wr_dat(0x0066);
		RM68120_wr_cmd(0xD50A);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD50B);	RM68120_wr_dat(0x00AD);
		RM68120_wr_cmd(0xD50C);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD50D);	RM68120_wr_dat(0x00E1);
		RM68120_wr_cmd(0xD50E);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD50F);	RM68120_wr_dat(0x0029);
		RM68120_wr_cmd(0xD510);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD511);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD512);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD513);	RM68120_wr_dat(0x0097);
		RM68120_wr_cmd(0xD514);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD515);	RM68120_wr_dat(0x00C2);
		RM68120_wr_cmd(0xD516);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD517);	RM68120_wr_dat(0x00FF);
		RM68120_wr_cmd(0xD518);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD519);	RM68120_wr_dat(0x002D);
		RM68120_wr_cmd(0xD51A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD51B);	RM68120_wr_dat(0x002F);
		RM68120_wr_cmd(0xD51C);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD51D);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD51E);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD51F);	RM68120_wr_dat(0x0081);
		RM68120_wr_cmd(0xD520);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD521);	RM68120_wr_dat(0x0098);
		RM68120_wr_cmd(0xD522);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD523);	RM68120_wr_dat(0x00B3);
		RM68120_wr_cmd(0xD524);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD525);	RM68120_wr_dat(0x00C4);
		RM68120_wr_cmd(0xD526);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD527);	RM68120_wr_dat(0x00DB);
		RM68120_wr_cmd(0xD528);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD529);	RM68120_wr_dat(0x00E9);
		RM68120_wr_cmd(0xD52A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD52B);	RM68120_wr_dat(0x00FD);
		RM68120_wr_cmd(0xD52C);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD52D);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD52E);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD52F);	RM68120_wr_dat(0x0021);
		RM68120_wr_cmd(0xD530);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD531);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD532);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD533);	RM68120_wr_dat(0x00F8);
		
		RM68120_wr_cmd(0xD600);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD601);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD602);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD603);	RM68120_wr_dat(0x0006);
		RM68120_wr_cmd(0xD604);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD605);	RM68120_wr_dat(0x001A);
		RM68120_wr_cmd(0xD606);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD607);	RM68120_wr_dat(0x003F);
		RM68120_wr_cmd(0xD608);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD609);	RM68120_wr_dat(0x0066);
		RM68120_wr_cmd(0xD60A);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD60B);	RM68120_wr_dat(0x00AD);
		RM68120_wr_cmd(0xD60C);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xD60D);	RM68120_wr_dat(0x00E1);
		RM68120_wr_cmd(0xD60E);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD60F);	RM68120_wr_dat(0x0029);
		RM68120_wr_cmd(0xD610);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD611);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD612);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD613);	RM68120_wr_dat(0x0097);
		RM68120_wr_cmd(0xD614);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD615);	RM68120_wr_dat(0x00C2);
		RM68120_wr_cmd(0xD616);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xD617);	RM68120_wr_dat(0x00FF);
		RM68120_wr_cmd(0xD618);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD619);	RM68120_wr_dat(0x002D);
		RM68120_wr_cmd(0xD61A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD61B);	RM68120_wr_dat(0x002F);
		RM68120_wr_cmd(0xD61C);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD61D);	RM68120_wr_dat(0x0058);
		RM68120_wr_cmd(0xD61E);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD61F);	RM68120_wr_dat(0x0081);
		RM68120_wr_cmd(0xD620);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD621);	RM68120_wr_dat(0x0098);
		RM68120_wr_cmd(0xD622);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD623);	RM68120_wr_dat(0x00B3);
		RM68120_wr_cmd(0xD624);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD625);	RM68120_wr_dat(0x00C4);
		RM68120_wr_cmd(0xD626);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD627);	RM68120_wr_dat(0x00DB);
		RM68120_wr_cmd(0xD628);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD629);	RM68120_wr_dat(0x00E9);
		RM68120_wr_cmd(0xD62A);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xD62B);	RM68120_wr_dat(0x00FD);
		RM68120_wr_cmd(0xD62C);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD62D);	RM68120_wr_dat(0x000B);
		RM68120_wr_cmd(0xD62E);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD62F);	RM68120_wr_dat(0x0021);
		RM68120_wr_cmd(0xD630);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD631);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xD632);	RM68120_wr_dat(0x0003);
		RM68120_wr_cmd(0xD633);	RM68120_wr_dat(0x00F8);
		
		/* LV2 Page 0 enable */
		RM68120_wr_cmd(0xF000);	RM68120_wr_dat(0x0055);
		RM68120_wr_cmd(0xF001);	RM68120_wr_dat(0x00AA);
		RM68120_wr_cmd(0xF002);	RM68120_wr_dat(0x0052);
		RM68120_wr_cmd(0xF003);	RM68120_wr_dat(0x0008);
		RM68120_wr_cmd(0xF004);	RM68120_wr_dat(0x0000);
		
		/* RGB interface signal control */
		RM68120_wr_cmd(0xB000);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0xB001);	RM68120_wr_dat(0x001C);
		RM68120_wr_cmd(0xB002);	RM68120_wr_dat(0x001C);
		RM68120_wr_cmd(0xB003);	RM68120_wr_dat(0x0010);
		RM68120_wr_cmd(0xB004);	RM68120_wr_dat(0x0010);
		
		/* Display control */
		RM68120_wr_cmd(0xB100);	RM68120_wr_dat(0x00FC);/* RAM Keep */
		
		RM68120_wr_cmd(0xB400);	RM68120_wr_dat(0x0010);/* CE On */
		
		/* Source hold time */
		RM68120_wr_cmd(0xB600);	RM68120_wr_dat(0x0005);
		
		/* Source EQ control (Mode 2) */
		RM68120_wr_cmd(0xB800);	RM68120_wr_dat(0x0001);
		RM68120_wr_cmd(0xB801);	RM68120_wr_dat(0x0004);
		RM68120_wr_cmd(0xB802);	RM68120_wr_dat(0x0004);
		RM68120_wr_cmd(0xB803);	RM68120_wr_dat(0x0004);
		
		/* Inversion mode (2-dot) */
		RM68120_wr_cmd(0xBC00);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xBC01);	RM68120_wr_dat(0x0002);
		RM68120_wr_cmd(0xBC02);	RM68120_wr_dat(0x0002);
	
		/* Timing control 4H w/4-Delay */
		RM68120_wr_cmd(0xC900);	RM68120_wr_dat(0x00C0);
		RM68120_wr_cmd(0xC901);	RM68120_wr_dat(0x0001);
	#if 0
		RM68120_wr_cmd(0xC902);	RM68120_wr_dat(0x0050);
		RM68120_wr_cmd(0xC903);	RM68120_wr_dat(0x0050);
		RM68120_wr_cmd(0xC904);	RM68120_wr_dat(0x0050);
	#endif
		RM68120_wr_cmd(0x3600);	RM68120_wr_dat(0x0000);
		RM68120_wr_cmd(0x3A00);	RM68120_wr_dat(0x0055);		/* 55=65K   66=262K */
		
		RM68120_wr_cmd(0x1100);	RM68120_wr_dat(0x0000);
		_delay_ms(120);
		
		RM68120_wr_cmd(0x2900);	RM68120_wr_dat(0x0000);
		_delay_ms(100);
	}

	else { for(;;);} /* Invalid Device Code!! */

	RM68120_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	RM68120_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		RM68120_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
