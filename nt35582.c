/********************************************************************************/
/*!
	@file			nt35582.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -LT043ANYB02				(NT35582)	8/16bit mode.

    @section HISTORY
		2015.05.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "nt35582.h"
/* check header file version for fool proof */
#if NT35582_H != 0x0300
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
inline void NT35582_reset(void)
{
	NT35582_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	NT35582_RD_SET();
	NT35582_WR_SET();
	_delay_ms(20);								/* wait 20ms     			*/

	NT35582_RES_CLR();							/* RES=L, CS=L   			*/
	NT35582_CS_CLR();
	_delay_ms(1);								/* wait 1ms     			*/
	
	NT35582_RES_SET();						  	/* RES=H					*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void NT35582_wr_cmd(uint16_t cmd)
{
	NT35582_DC_CLR();							/* DC=L					*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	NT35582_CMD = (uint8_t)(cmd>>8);			/* upper 8bit command	*/
	NT35582_WR();								/* WR=L->H				*/
	NT35582_CMD = (uint8_t)cmd;					/* lower 8bit command	*/
#else
	NT35582_CMD = cmd;							/* 16bit command		*/
#endif
    NT35582_WR();								/* WR=L->H				*/

	NT35582_DC_SET();							/* DC=H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void NT35582_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	NT35582_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	NT35582_WR();								/* WR=L->H					*/
	NT35582_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	NT35582_DATA = dat;							/* 16bit data 				*/
#endif
	NT35582_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void NT35582_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		NT35582_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		NT35582_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void NT35582_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	NT35582_wr_cmd(0x2A00);					/* Horizontal RAM Start ADDR */
	NT35582_wr_dat((OFS_COL + x)>>8);
	NT35582_wr_cmd(0x2A01);
	NT35582_wr_dat(OFS_COL + x);
	NT35582_wr_cmd(0x2A02);					/* Horizontal RAM End ADDR */
	NT35582_wr_dat((OFS_COL + width)>>8);
	NT35582_wr_cmd(0x2A03);
	NT35582_wr_dat(OFS_COL + width);

	NT35582_wr_cmd(0x2B00);					/* Vertical RAM Start ADDR */
	NT35582_wr_dat((OFS_RAW + y)>>8);
	NT35582_wr_cmd(0x2B01);
	NT35582_wr_dat(OFS_RAW + y);
	NT35582_wr_cmd(0x2B02);					/* Vertical RAM End ADDR */
	NT35582_wr_dat((OFS_RAW + height)>>8);
	NT35582_wr_cmd(0x2B03);
	NT35582_wr_dat(OFS_RAW + height);

	NT35582_wr_cmd(0x2C00);					/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void NT35582_clear(void)
{
	volatile uint32_t n;

	NT35582_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		NT35582_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t NT35582_rd_cmd(uint16_t cmd)
{
	uint16_t val,temp;

	NT35582_wr_cmd(cmd);
	NT35582_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif
    ReadLCDData(temp);
	temp <<= 8;

	NT35582_wr_cmd(cmd | 0x0100);
	NT35582_WR_SET();

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
void NT35582_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	NT35582_reset();

	devicetype = NT35582_rd_cmd(0x1080);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x5582)
	{
		/* Initialize NT35582 */
		/* Sleep out */
		NT35582_wr_cmd(0x1100);
		_delay_ms(200);
		
		NT35582_wr_cmd(0xC000);	NT35582_wr_dat(0x0086);
		NT35582_wr_cmd(0xC001);	NT35582_wr_dat(0x0000);
		NT35582_wr_cmd(0xC002);	NT35582_wr_dat(0x0086);
		NT35582_wr_cmd(0xC003);	NT35582_wr_dat(0x0000);
		NT35582_wr_cmd(0xC100);	NT35582_wr_dat(0x0045);
		NT35582_wr_cmd(0xC200);	NT35582_wr_dat(0x0021);
		NT35582_wr_cmd(0xC202);	NT35582_wr_dat(0x0002);
		NT35582_wr_cmd(0xB600);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xB602);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xC700);	NT35582_wr_dat(0x008F);
		
		NT35582_wr_cmd(0xE000);	NT35582_wr_dat(0x000E);
		NT35582_wr_cmd(0xE001);	NT35582_wr_dat(0x0014);
		NT35582_wr_cmd(0xE002);	NT35582_wr_dat(0x0029);
		NT35582_wr_cmd(0xE003);	NT35582_wr_dat(0x003A);
		NT35582_wr_cmd(0xE004);	NT35582_wr_dat(0x001D);
		NT35582_wr_cmd(0xE005);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xE006);	NT35582_wr_dat(0x0061);
		NT35582_wr_cmd(0xE007);	NT35582_wr_dat(0x003D);
		NT35582_wr_cmd(0xE008);	NT35582_wr_dat(0x0022);
		NT35582_wr_cmd(0xE009);	NT35582_wr_dat(0x002A);
		NT35582_wr_cmd(0xE00A);	NT35582_wr_dat(0x0087);
		NT35582_wr_cmd(0xE00B);	NT35582_wr_dat(0x0016);
		NT35582_wr_cmd(0xE00C);	NT35582_wr_dat(0x003B);
		NT35582_wr_cmd(0xE00D);	NT35582_wr_dat(0x004C);
		NT35582_wr_cmd(0xE00E);	NT35582_wr_dat(0x0078);
		NT35582_wr_cmd(0xE00F);	NT35582_wr_dat(0x0096);
		NT35582_wr_cmd(0xE010);	NT35582_wr_dat(0x004A);
		NT35582_wr_cmd(0xE011);	NT35582_wr_dat(0x004D);
		NT35582_wr_cmd(0xE100);	NT35582_wr_dat(0x000E);
		NT35582_wr_cmd(0xE101);	NT35582_wr_dat(0x0014);
		NT35582_wr_cmd(0xE102);	NT35582_wr_dat(0x0029);
		NT35582_wr_cmd(0xE103);	NT35582_wr_dat(0x003A);
		NT35582_wr_cmd(0xE104);	NT35582_wr_dat(0x001D);
		NT35582_wr_cmd(0xE105);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xE106);	NT35582_wr_dat(0x0061);
		NT35582_wr_cmd(0xE107);	NT35582_wr_dat(0x003F);
		NT35582_wr_cmd(0xE108);	NT35582_wr_dat(0x0020);
		NT35582_wr_cmd(0xE109);	NT35582_wr_dat(0x0026);
		NT35582_wr_cmd(0xE10A);	NT35582_wr_dat(0x0083);
		NT35582_wr_cmd(0xE10B);	NT35582_wr_dat(0x0016);
		NT35582_wr_cmd(0xE10C);	NT35582_wr_dat(0x003B);
		NT35582_wr_cmd(0xE10D);	NT35582_wr_dat(0x004C);
		NT35582_wr_cmd(0xE10E);	NT35582_wr_dat(0x0078);
		NT35582_wr_cmd(0xE10F);	NT35582_wr_dat(0x0096);
		NT35582_wr_cmd(0xE110);	NT35582_wr_dat(0x004A);
		NT35582_wr_cmd(0xE111);	NT35582_wr_dat(0x004D);
		NT35582_wr_cmd(0xE200);	NT35582_wr_dat(0x000E);
		NT35582_wr_cmd(0xE201);	NT35582_wr_dat(0x0014);
		NT35582_wr_cmd(0xE202);	NT35582_wr_dat(0x0029);
		NT35582_wr_cmd(0xE203);	NT35582_wr_dat(0x003A);
		NT35582_wr_cmd(0xE204);	NT35582_wr_dat(0x001D);
		NT35582_wr_cmd(0xE205);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xE206);	NT35582_wr_dat(0x0061);
		NT35582_wr_cmd(0xE207);	NT35582_wr_dat(0x003D);
		NT35582_wr_cmd(0xE208);	NT35582_wr_dat(0x0022);
		NT35582_wr_cmd(0xE209);	NT35582_wr_dat(0x002A);
		NT35582_wr_cmd(0xE20A);	NT35582_wr_dat(0x0087);
		NT35582_wr_cmd(0xE20B);	NT35582_wr_dat(0x0016);
		NT35582_wr_cmd(0xE20C);	NT35582_wr_dat(0x003B);
		NT35582_wr_cmd(0xE20D);	NT35582_wr_dat(0x004C);
		NT35582_wr_cmd(0xE20E);	NT35582_wr_dat(0x0078);
		NT35582_wr_cmd(0xE20F);	NT35582_wr_dat(0x0096);
		NT35582_wr_cmd(0xE210);	NT35582_wr_dat(0x004A);
		NT35582_wr_cmd(0xE211);	NT35582_wr_dat(0x004D);
		NT35582_wr_cmd(0xE300);	NT35582_wr_dat(0x000E);
		NT35582_wr_cmd(0xE301);	NT35582_wr_dat(0x0014);
		
		NT35582_wr_cmd(0xE302);	NT35582_wr_dat(0x0029);
		NT35582_wr_cmd(0xE303);	NT35582_wr_dat(0x003A);
		NT35582_wr_cmd(0xE304);	NT35582_wr_dat(0x001D);
		NT35582_wr_cmd(0xE305);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xE306);	NT35582_wr_dat(0x0061);
		NT35582_wr_cmd(0xE307);	NT35582_wr_dat(0x003F);
		NT35582_wr_cmd(0xE308);	NT35582_wr_dat(0x0020);
		NT35582_wr_cmd(0xE309);	NT35582_wr_dat(0x0026);
		NT35582_wr_cmd(0xE30A);	NT35582_wr_dat(0x0083);
		NT35582_wr_cmd(0xE30B);	NT35582_wr_dat(0x0016);
		NT35582_wr_cmd(0xE30C);	NT35582_wr_dat(0x003B);
		NT35582_wr_cmd(0xE30D);	NT35582_wr_dat(0x004C);
		NT35582_wr_cmd(0xE30E);	NT35582_wr_dat(0x0078);
		NT35582_wr_cmd(0xE30F);	NT35582_wr_dat(0x0096);
		NT35582_wr_cmd(0xE310);	NT35582_wr_dat(0x004A);
		NT35582_wr_cmd(0xE311);	NT35582_wr_dat(0x004D);
		NT35582_wr_cmd(0xE400);	NT35582_wr_dat(0x000E);
		NT35582_wr_cmd(0xE401);	NT35582_wr_dat(0x0014);
		NT35582_wr_cmd(0xE402);	NT35582_wr_dat(0x0029);
		NT35582_wr_cmd(0xE403);	NT35582_wr_dat(0x003A);
		NT35582_wr_cmd(0xE404);	NT35582_wr_dat(0x001D);
		NT35582_wr_cmd(0xE405);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xE406);	NT35582_wr_dat(0x0061);
		NT35582_wr_cmd(0xE407);	NT35582_wr_dat(0x003D);
		NT35582_wr_cmd(0xE408);	NT35582_wr_dat(0x0022);
		NT35582_wr_cmd(0xE409);	NT35582_wr_dat(0x002A);
		NT35582_wr_cmd(0xE40A);	NT35582_wr_dat(0x0087);
		NT35582_wr_cmd(0xE40B);	NT35582_wr_dat(0x0016);
		NT35582_wr_cmd(0xE40C);	NT35582_wr_dat(0x003B);
		NT35582_wr_cmd(0xE40D);	NT35582_wr_dat(0x004C);
		NT35582_wr_cmd(0xE40E);	NT35582_wr_dat(0x0078);
		NT35582_wr_cmd(0xE40F);	NT35582_wr_dat(0x0096);
		NT35582_wr_cmd(0xE410);	NT35582_wr_dat(0x004A);
		NT35582_wr_cmd(0xE411);	NT35582_wr_dat(0x004D);
		NT35582_wr_cmd(0xE500);	NT35582_wr_dat(0x000E);
		NT35582_wr_cmd(0xE501);	NT35582_wr_dat(0x0014);
		NT35582_wr_cmd(0xE502);	NT35582_wr_dat(0x0029);
		NT35582_wr_cmd(0xE503);	NT35582_wr_dat(0x003A);
		NT35582_wr_cmd(0xE504);	NT35582_wr_dat(0x001D);
		NT35582_wr_cmd(0xE505);	NT35582_wr_dat(0x0030);
		NT35582_wr_cmd(0xE506);	NT35582_wr_dat(0x0061);
		NT35582_wr_cmd(0xE507);	NT35582_wr_dat(0x003F);
		NT35582_wr_cmd(0xE508);	NT35582_wr_dat(0x0020);
		NT35582_wr_cmd(0xE509);	NT35582_wr_dat(0x0026);
		NT35582_wr_cmd(0xE50A);	NT35582_wr_dat(0x0083);
		
		NT35582_wr_cmd(0xE50B);	NT35582_wr_dat(0x0016);
		NT35582_wr_cmd(0xE50C);	NT35582_wr_dat(0x003B);
		NT35582_wr_cmd(0xE50D);	NT35582_wr_dat(0x004C);
		NT35582_wr_cmd(0xE50E);	NT35582_wr_dat(0x0078);
		NT35582_wr_cmd(0xE50F);	NT35582_wr_dat(0x0096);
		NT35582_wr_cmd(0xE510);	NT35582_wr_dat(0x004A);
		NT35582_wr_cmd(0xE511);	NT35582_wr_dat(0x004D);
		
		NT35582_wr_cmd(0x3A00);	NT35582_wr_dat(0x0055);		/* 55=65K   66=262K */
		
		/* Display on */
		NT35582_wr_cmd(0x2900);
	}

	else { for(;;);} /* Invalid Device Code!! */

	NT35582_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	NT35582_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		NT35582_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
