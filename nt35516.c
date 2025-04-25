/********************************************************************************/
/*!
	@file			nt35516.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive IPS1P1399 TFT module(8/16bit mode).

    @section HISTORY
		2023.08.01	V1.00	Stable Release

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "nt35516.h"
/* check header file version for fool proof */
#if NT35516_H != 0x0100
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
inline void NT35516_reset(void)
{
	NT35516_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	NT35516_RD_SET();
	NT35516_WR_SET();
	_delay_ms(100);								/* wait 100ms to powerup    */

	NT35516_RES_CLR();							/* RES=L, CS=L   			*/
	NT35516_CS_CLR();
	_delay_ms(20);								/* wait 20ms     			*/
	
	NT35516_RES_SET();						  	/* RES=H					*/
	_delay_ms(20);				    			/* wait 20ms    			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void NT35516_wr_cmd(uint16_t cmd)
{
	NT35516_DC_CLR();							/* DC=L					*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	NT35516_CMD = (uint8_t)(cmd>>8);			/* upper 8bit command	*/
	NT35516_WR();								/* WR=L->H				*/
	NT35516_CMD = (uint8_t)cmd;					/* lower 8bit command	*/
#else
	NT35516_CMD = cmd;							/* 16bit command		*/
#endif	
    NT35516_WR();								/* WR=L->H				*/

	NT35516_DC_SET();							/* DC=H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void NT35516_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	NT35516_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	NT35516_WR();								/* WR=L->H					*/
	NT35516_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	NT35516_DATA = dat;							/* 16bit data 				*/
#endif
	NT35516_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void NT35516_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		NT35516_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		NT35516_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void NT35516_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	NT35516_wr_cmd(0x2A00);				/* Horizontal RAM Start ADDR */
	NT35516_wr_dat((OFS_COL + x)>>8);
	NT35516_wr_cmd(0x2A01);
	NT35516_wr_dat(OFS_COL + x);
	NT35516_wr_cmd(0x2A02);				/* Horizontal RAM End ADDR */
	NT35516_wr_dat((OFS_COL + width)>>8);
	NT35516_wr_cmd(0x2A03);
	NT35516_wr_dat(OFS_COL + width);

	NT35516_wr_cmd(0x2B00);				/* Vertical RAM Start ADDR */
	NT35516_wr_dat((OFS_RAW + y)>>8);
	NT35516_wr_cmd(0x2B01);
	NT35516_wr_dat(OFS_RAW + y);
	NT35516_wr_cmd(0x2B02);				/* Vertical RAM End ADDR */
	NT35516_wr_dat((OFS_RAW + height)>>8);
	NT35516_wr_cmd(0x2B03);
	NT35516_wr_dat(OFS_RAW + height);

	NT35516_wr_cmd(0x2C00);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void NT35516_clear(void)
{
	volatile uint32_t n;

	NT35516_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		NT35516_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t NT35516_rd_cmd(uint16_t cmd)
{
	uint16_t val,temp;
	
	/* PAGE1 */
	NT35516_wr_cmd(0xF000);	NT35516_wr_dat(0x0055);
	NT35516_wr_cmd(0xF001);	NT35516_wr_dat(0x00AA);	
	NT35516_wr_cmd(0xF002);	NT35516_wr_dat(0x0052);	
	NT35516_wr_cmd(0xF003);	NT35516_wr_dat(0x0008);	
	NT35516_wr_cmd(0xF004);	NT35516_wr_dat(0x0001);	

	NT35516_wr_cmd(cmd);
	NT35516_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif
    ReadLCDData(temp);
	temp <<= 8;

	NT35516_wr_cmd(cmd | 0x0001);
	NT35516_WR_SET();

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
void NT35516_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	NT35516_reset();

	devicetype = NT35516_rd_cmd(0xC500);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x5516)
	{
		/* Initialize NT35516 */
		NT35516_wr_cmd(0xFF00); NT35516_wr_dat(0x00AA);
		NT35516_wr_cmd(0xFF01); NT35516_wr_dat(0x0055);
		NT35516_wr_cmd(0xFF02); NT35516_wr_dat(0x0025);
		NT35516_wr_cmd(0xFF03); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xFF04); NT35516_wr_dat(0x0001);
		
		NT35516_wr_cmd(0xF200); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF201); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF202); NT35516_wr_dat(0x004A);
		NT35516_wr_cmd(0xF203); NT35516_wr_dat(0x000A);
		NT35516_wr_cmd(0xF204); NT35516_wr_dat(0x00A8);
		NT35516_wr_cmd(0xF205); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF206); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF207); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF208); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF20A); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF20B); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF20C); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF20D); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF20E); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF20F); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF210); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF211); NT35516_wr_dat(0x000B);
		NT35516_wr_cmd(0xF212); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF213); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF214); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF215); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF216); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF217); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF218); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF219); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF21A); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF21B); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF21C); NT35516_wr_dat(0x0040);
		NT35516_wr_cmd(0xF21D); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xF21E); NT35516_wr_dat(0x0051);
		NT35516_wr_cmd(0xF21F); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF220); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xF221); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xF222); NT35516_wr_dat(0x0001);
		
		NT35516_wr_cmd(0xF300); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xF301); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xF302); NT35516_wr_dat(0x0007);
		NT35516_wr_cmd(0xF303); NT35516_wr_dat(0x0045);
		NT35516_wr_cmd(0xF304); NT35516_wr_dat(0x0088);
		NT35516_wr_cmd(0xF305); NT35516_wr_dat(0x00D1);
		NT35516_wr_cmd(0xF306); NT35516_wr_dat(0x000D);
		
		NT35516_wr_cmd(0xF000); NT35516_wr_dat(0x0055);
		NT35516_wr_cmd(0xF001); NT35516_wr_dat(0x00AA);
		NT35516_wr_cmd(0xF002); NT35516_wr_dat(0x0052);
		NT35516_wr_cmd(0xF003); NT35516_wr_dat(0x0008);
		NT35516_wr_cmd(0xF004); NT35516_wr_dat(0x0000);
		
		NT35516_wr_cmd(0xB000); NT35516_wr_dat(0x0003); // CMRC  VSDL  HSDL  DEDL  PCKP  DEP  HSP  VSP 
		
		NT35516_wr_cmd(0xB100); NT35516_wr_dat(0x00CC);
		NT35516_wr_cmd(0xB101); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xB102); NT35516_wr_dat(0x0000);
		
		NT35516_wr_cmd(0xB800); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xB801); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xB802); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xB803); NT35516_wr_dat(0x0002);
		
		NT35516_wr_cmd(0xBC00); NT35516_wr_dat(0x0000); // NLA2  NLA1  NLA0
		NT35516_wr_cmd(0xBC01); NT35516_wr_dat(0x0000); // NLB2  NLB1  NLB0
		NT35516_wr_cmd(0xBC02); NT35516_wr_dat(0x0000); // NLC2  NLC1  NLC0
		
		NT35516_wr_cmd(0xC900); NT35516_wr_dat(0x0063);
		NT35516_wr_cmd(0xC901); NT35516_wr_dat(0x0006);
		NT35516_wr_cmd(0xC902); NT35516_wr_dat(0x000D);
		NT35516_wr_cmd(0xC903); NT35516_wr_dat(0x001A);
		NT35516_wr_cmd(0xC904); NT35516_wr_dat(0x0017);
		NT35516_wr_cmd(0xC905); NT35516_wr_dat(0x0000);
		
		NT35516_wr_cmd(0xF000); NT35516_wr_dat(0x0055);
		NT35516_wr_cmd(0xF001); NT35516_wr_dat(0x00AA);
		NT35516_wr_cmd(0xF002); NT35516_wr_dat(0x0052);
		NT35516_wr_cmd(0xF003); NT35516_wr_dat(0x0008);
		NT35516_wr_cmd(0xF004); NT35516_wr_dat(0x0001);
		
		NT35516_wr_cmd(0xB000); NT35516_wr_dat(0x0005);
		NT35516_wr_cmd(0xB001); NT35516_wr_dat(0x0005);
		NT35516_wr_cmd(0xB002); NT35516_wr_dat(0x0005);
		
		NT35516_wr_cmd(0xB100); NT35516_wr_dat(0x0005);
		NT35516_wr_cmd(0xB101); NT35516_wr_dat(0x0005);
		NT35516_wr_cmd(0xB102); NT35516_wr_dat(0x0005);
		
		NT35516_wr_cmd(0xB200); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xB201); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xB202); NT35516_wr_dat(0x0001);
		
		NT35516_wr_cmd(0xB300); NT35516_wr_dat(0x000E);
		NT35516_wr_cmd(0xB301); NT35516_wr_dat(0x000E);
		NT35516_wr_cmd(0xB302); NT35516_wr_dat(0x000E);
		
		NT35516_wr_cmd(0xB400); NT35516_wr_dat(0x000A);
		NT35516_wr_cmd(0xB401); NT35516_wr_dat(0x000A);
		NT35516_wr_cmd(0xB402); NT35516_wr_dat(0x000A);
		
		NT35516_wr_cmd(0xB600); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xB601); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xB602); NT35516_wr_dat(0x0044);
		
		NT35516_wr_cmd(0xB700); NT35516_wr_dat(0x0034);
		NT35516_wr_cmd(0xB701); NT35516_wr_dat(0x0034);
		NT35516_wr_cmd(0xB702); NT35516_wr_dat(0x0034);
		
		NT35516_wr_cmd(0xB800); NT35516_wr_dat(0x0020); // Set VCL boosting times/frequency
		NT35516_wr_cmd(0xB801); NT35516_wr_dat(0x0020);
		NT35516_wr_cmd(0xB802); NT35516_wr_dat(0x0020);
		
		NT35516_wr_cmd(0xB900); NT35516_wr_dat(0x0026); // Set VGH boosting times/frequency
		NT35516_wr_cmd(0xB901); NT35516_wr_dat(0x0026);
		NT35516_wr_cmd(0xB902); NT35516_wr_dat(0x0026);
		
		NT35516_wr_cmd(0xBA00); NT35516_wr_dat(0x0024); // Set VGLX boosting times/frequency
		NT35516_wr_cmd(0xBA01); NT35516_wr_dat(0x0024);
		NT35516_wr_cmd(0xBA02); NT35516_wr_dat(0x0024);
		
		NT35516_wr_cmd(0xBC00); NT35516_wr_dat(0x0000); // Set VGMP/VGSP voltages 
		NT35516_wr_cmd(0xBC01); NT35516_wr_dat(0x00C8);
		NT35516_wr_cmd(0xBC02); NT35516_wr_dat(0x0000);
		
		NT35516_wr_cmd(0xBD00); NT35516_wr_dat(0x0000); // Set VGMN/VGSN voltages
		NT35516_wr_cmd(0xBD01); NT35516_wr_dat(0x00C8);
		NT35516_wr_cmd(0xBD02); NT35516_wr_dat(0x0000);
		
		NT35516_wr_cmd(0xBE00); NT35516_wr_dat(0x0071); // Setting DC VCOM offset 
		
		NT35516_wr_cmd(0xC000); NT35516_wr_dat(0x0004);
		NT35516_wr_cmd(0xC001); NT35516_wr_dat(0x0000);
		
		NT35516_wr_cmd(0xCA00); NT35516_wr_dat(0x0000); // Gate signal voltage control 
		
		NT35516_wr_cmd(0xD000); NT35516_wr_dat(0x000A);
		NT35516_wr_cmd(0xD001); NT35516_wr_dat(0x0010);
		NT35516_wr_cmd(0xD002); NT35516_wr_dat(0x000D);
		NT35516_wr_cmd(0xD003); NT35516_wr_dat(0x000F);
		
		NT35516_wr_cmd(0xD100); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD101); NT35516_wr_dat(0x0070);
		NT35516_wr_cmd(0xD102); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD103); NT35516_wr_dat(0x00CE);
		NT35516_wr_cmd(0xD104); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD105); NT35516_wr_dat(0x00F7);
		NT35516_wr_cmd(0xD106); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD107); NT35516_wr_dat(0x0010);
		NT35516_wr_cmd(0xD108); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD109); NT35516_wr_dat(0x0021);
		NT35516_wr_cmd(0xD10A); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD10B); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xD10C); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD10D); NT35516_wr_dat(0x0062);
		NT35516_wr_cmd(0xD10E); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD10F); NT35516_wr_dat(0x008D);
		
		NT35516_wr_cmd(0xD200); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD201); NT35516_wr_dat(0x00AF);
		NT35516_wr_cmd(0xD202); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD203); NT35516_wr_dat(0x00E4);
		NT35516_wr_cmd(0xD204); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD205); NT35516_wr_dat(0x000C);
		NT35516_wr_cmd(0xD206); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD207); NT35516_wr_dat(0x004D);
		NT35516_wr_cmd(0xD208); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD209); NT35516_wr_dat(0x0082);
		NT35516_wr_cmd(0xD20A); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD20B); NT35516_wr_dat(0x0084);
		NT35516_wr_cmd(0xD20C); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD20D); NT35516_wr_dat(0x00B8);
		NT35516_wr_cmd(0xD20E); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD20F); NT35516_wr_dat(0x00F0);
		
		NT35516_wr_cmd(0xD300); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD301); NT35516_wr_dat(0x0014);
		NT35516_wr_cmd(0xD302); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD303); NT35516_wr_dat(0x0042);
		NT35516_wr_cmd(0xD304); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD305); NT35516_wr_dat(0x005E);
		NT35516_wr_cmd(0xD306); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD307); NT35516_wr_dat(0x0080);
		NT35516_wr_cmd(0xD308); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD309); NT35516_wr_dat(0x0097);
		NT35516_wr_cmd(0xD30A); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD30B); NT35516_wr_dat(0x00B0);
		NT35516_wr_cmd(0xD30C); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD30D); NT35516_wr_dat(0x00C0);
		NT35516_wr_cmd(0xD30E); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD30F); NT35516_wr_dat(0x00DF);
		
		NT35516_wr_cmd(0xD400); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD401); NT35516_wr_dat(0x00FD);
		NT35516_wr_cmd(0xD402); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD403); NT35516_wr_dat(0x00FF);
		
		NT35516_wr_cmd(0xD500); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD501); NT35516_wr_dat(0x0070);
		NT35516_wr_cmd(0xD502); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD503); NT35516_wr_dat(0x00CE);
		NT35516_wr_cmd(0xD504); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD505); NT35516_wr_dat(0x00F7);
		NT35516_wr_cmd(0xD506); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD507); NT35516_wr_dat(0x0010);
		NT35516_wr_cmd(0xD508); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD509); NT35516_wr_dat(0x0021);
		NT35516_wr_cmd(0xD50A); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD50B); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xD50C); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD50D); NT35516_wr_dat(0x0062);
		NT35516_wr_cmd(0xD50E); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD50F); NT35516_wr_dat(0x008D);
		
		NT35516_wr_cmd(0xD600); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD601); NT35516_wr_dat(0x00AF);
		NT35516_wr_cmd(0xD602); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD603); NT35516_wr_dat(0x00E4);
		NT35516_wr_cmd(0xD604); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD605); NT35516_wr_dat(0x000C);
		NT35516_wr_cmd(0xD606); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD607); NT35516_wr_dat(0x004D);
		NT35516_wr_cmd(0xD608); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD609); NT35516_wr_dat(0x0082);
		NT35516_wr_cmd(0xD60A); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD60B); NT35516_wr_dat(0x0084);
		NT35516_wr_cmd(0xD60C); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD60D); NT35516_wr_dat(0x00B8);
		NT35516_wr_cmd(0xD60E); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xD60F); NT35516_wr_dat(0x00F0);
		
		NT35516_wr_cmd(0xD700); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD701); NT35516_wr_dat(0x0014);
		NT35516_wr_cmd(0xD702); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD703); NT35516_wr_dat(0x0042);
		NT35516_wr_cmd(0xD704); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD705); NT35516_wr_dat(0x005E);
		NT35516_wr_cmd(0xD706); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD707); NT35516_wr_dat(0x0080);
		NT35516_wr_cmd(0xD708); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD709); NT35516_wr_dat(0x0097);
		NT35516_wr_cmd(0xD70A); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD70B); NT35516_wr_dat(0x00B0);
		NT35516_wr_cmd(0xD70C); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD70D); NT35516_wr_dat(0x00C0);
		NT35516_wr_cmd(0xD70E); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD70F); NT35516_wr_dat(0x00DF);
		
		NT35516_wr_cmd(0xD800); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD801); NT35516_wr_dat(0x00FD);
		NT35516_wr_cmd(0xD802); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xD803); NT35516_wr_dat(0x00FF);
		
		NT35516_wr_cmd(0xD900); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD901); NT35516_wr_dat(0x0070);
		NT35516_wr_cmd(0xD902); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD903); NT35516_wr_dat(0x00CE);
		NT35516_wr_cmd(0xD904); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xD905); NT35516_wr_dat(0x00F7);
		NT35516_wr_cmd(0xD906); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD907); NT35516_wr_dat(0x0010);
		NT35516_wr_cmd(0xD908); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD909); NT35516_wr_dat(0x0021);
		NT35516_wr_cmd(0xD90A); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD90B); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xD90C); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD90D); NT35516_wr_dat(0x0062);
		NT35516_wr_cmd(0xD90E); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xD90F); NT35516_wr_dat(0x008D);
		
		NT35516_wr_cmd(0xDD00); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xDD01); NT35516_wr_dat(0x00AF);
		NT35516_wr_cmd(0xDD02); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xDD03); NT35516_wr_dat(0x00E4);
		NT35516_wr_cmd(0xDD04); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xDD05); NT35516_wr_dat(0x000C);
		NT35516_wr_cmd(0xDD06); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xDD07); NT35516_wr_dat(0x004D);
		NT35516_wr_cmd(0xDD08); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xDD09); NT35516_wr_dat(0x0082);
		NT35516_wr_cmd(0xDD0A); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xDD0B); NT35516_wr_dat(0x0084);
		NT35516_wr_cmd(0xDD0C); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xDD0D); NT35516_wr_dat(0x00B8);
		NT35516_wr_cmd(0xDD0E); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xDD0F); NT35516_wr_dat(0x00F0);
		
		NT35516_wr_cmd(0xDE00); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE01); NT35516_wr_dat(0x0014);
		NT35516_wr_cmd(0xDE02); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE03); NT35516_wr_dat(0x0042);
		NT35516_wr_cmd(0xDE04); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE05); NT35516_wr_dat(0x005E);
		NT35516_wr_cmd(0xDE06); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE07); NT35516_wr_dat(0x0080);
		NT35516_wr_cmd(0xDE08); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE09); NT35516_wr_dat(0x0097);
		NT35516_wr_cmd(0xDE0A); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE0B); NT35516_wr_dat(0x00B0);
		NT35516_wr_cmd(0xDE0C); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE0D); NT35516_wr_dat(0x00C0);
		NT35516_wr_cmd(0xDE0E); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDE0F); NT35516_wr_dat(0x00DF);
		
		NT35516_wr_cmd(0xDF00); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDF01); NT35516_wr_dat(0x00FD);
		NT35516_wr_cmd(0xDF02); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xDF03); NT35516_wr_dat(0x00FF);
		
		NT35516_wr_cmd(0xE000); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE001); NT35516_wr_dat(0x0070);
		NT35516_wr_cmd(0xE002); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE003); NT35516_wr_dat(0x00CE);
		NT35516_wr_cmd(0xE004); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE005); NT35516_wr_dat(0x00F7);
		NT35516_wr_cmd(0xE006); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE007); NT35516_wr_dat(0x0010);
		NT35516_wr_cmd(0xE008); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE009); NT35516_wr_dat(0x0021);
		NT35516_wr_cmd(0xE00A); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE00B); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xE00C); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE00D); NT35516_wr_dat(0x0062);
		NT35516_wr_cmd(0xE00E); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE00F); NT35516_wr_dat(0x008D);
		
		NT35516_wr_cmd(0xE100); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE101); NT35516_wr_dat(0x00AF);
		NT35516_wr_cmd(0xE102); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE103); NT35516_wr_dat(0x00E4);
		NT35516_wr_cmd(0xE104); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE105); NT35516_wr_dat(0x000C);
		NT35516_wr_cmd(0xE106); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE107); NT35516_wr_dat(0x004D);
		NT35516_wr_cmd(0xE108); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE109); NT35516_wr_dat(0x0082);
		NT35516_wr_cmd(0xE10A); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE10B); NT35516_wr_dat(0x0084);
		NT35516_wr_cmd(0xE10C); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE10D); NT35516_wr_dat(0x00B8);
		NT35516_wr_cmd(0xE10E); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE10F); NT35516_wr_dat(0x00F0);
		
		NT35516_wr_cmd(0xE200); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE201); NT35516_wr_dat(0x0014);
		NT35516_wr_cmd(0xE202); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE203); NT35516_wr_dat(0x0042);
		NT35516_wr_cmd(0xE204); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE205); NT35516_wr_dat(0x005E);
		NT35516_wr_cmd(0xE206); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE207); NT35516_wr_dat(0x0080);
		NT35516_wr_cmd(0xE208); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE209); NT35516_wr_dat(0x0097);
		NT35516_wr_cmd(0xE20A); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE20B); NT35516_wr_dat(0x00B0);
		NT35516_wr_cmd(0xE20C); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE20D); NT35516_wr_dat(0x00C0);
		NT35516_wr_cmd(0xE20E); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE20F); NT35516_wr_dat(0x00DF);
		
		NT35516_wr_cmd(0xE300); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE301); NT35516_wr_dat(0x00FD);
		NT35516_wr_cmd(0xE302); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE303); NT35516_wr_dat(0x00FF);
		
		NT35516_wr_cmd(0xE400); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE401); NT35516_wr_dat(0x0070);
		NT35516_wr_cmd(0xE402); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE403); NT35516_wr_dat(0x00CE);
		NT35516_wr_cmd(0xE404); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE405); NT35516_wr_dat(0x00F7);
		NT35516_wr_cmd(0xE406); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE407); NT35516_wr_dat(0x0010);
		NT35516_wr_cmd(0xE408); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE409); NT35516_wr_dat(0x0021);
		NT35516_wr_cmd(0xE40A); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE40B); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xE40C); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE40D); NT35516_wr_dat(0x0062);
		NT35516_wr_cmd(0xE40E); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE40F); NT35516_wr_dat(0x008D);
		
		NT35516_wr_cmd(0xE500); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE501); NT35516_wr_dat(0x00AF);
		NT35516_wr_cmd(0xE502); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE503); NT35516_wr_dat(0x00E4);
		NT35516_wr_cmd(0xE504); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE505); NT35516_wr_dat(0x000C);
		NT35516_wr_cmd(0xE506); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE507); NT35516_wr_dat(0x004D);
		NT35516_wr_cmd(0xE508); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE509); NT35516_wr_dat(0x0082);
		NT35516_wr_cmd(0xE50A); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE50B); NT35516_wr_dat(0x0084);
		NT35516_wr_cmd(0xE50C); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE50D); NT35516_wr_dat(0x00B8);
		NT35516_wr_cmd(0xE50E); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE50F); NT35516_wr_dat(0x00F0);
		
		NT35516_wr_cmd(0xE600); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE601); NT35516_wr_dat(0x0014);
		NT35516_wr_cmd(0xE602); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE603); NT35516_wr_dat(0x0042);
		NT35516_wr_cmd(0xE604); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE605); NT35516_wr_dat(0x005E);
		NT35516_wr_cmd(0xE606); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE607); NT35516_wr_dat(0x0080);
		NT35516_wr_cmd(0xE608); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE609); NT35516_wr_dat(0x0097);
		NT35516_wr_cmd(0xE60A); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE60B); NT35516_wr_dat(0x00B0);
		NT35516_wr_cmd(0xE60C); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE60D); NT35516_wr_dat(0x00C0);
		NT35516_wr_cmd(0xE60E); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE60F); NT35516_wr_dat(0x00DF);
		
		NT35516_wr_cmd(0xE700); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE701); NT35516_wr_dat(0x00FD);
		NT35516_wr_cmd(0xE702); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xE703); NT35516_wr_dat(0x00FF);
		
		NT35516_wr_cmd(0xE800); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE801); NT35516_wr_dat(0x0070);
		NT35516_wr_cmd(0xE802); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE803); NT35516_wr_dat(0x00CE);
		NT35516_wr_cmd(0xE804); NT35516_wr_dat(0x0000);
		NT35516_wr_cmd(0xE805); NT35516_wr_dat(0x00F7);
		NT35516_wr_cmd(0xE806); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE807); NT35516_wr_dat(0x0010);
		NT35516_wr_cmd(0xE808); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE809); NT35516_wr_dat(0x0021);
		NT35516_wr_cmd(0xE80A); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE80B); NT35516_wr_dat(0x0044);
		NT35516_wr_cmd(0xE80C); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE80D); NT35516_wr_dat(0x0062);
		NT35516_wr_cmd(0xE80E); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE80F); NT35516_wr_dat(0x008D);
		
		NT35516_wr_cmd(0xE900); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE901); NT35516_wr_dat(0x00AF);
		NT35516_wr_cmd(0xE902); NT35516_wr_dat(0x0001);
		NT35516_wr_cmd(0xE903); NT35516_wr_dat(0x00E4);
		NT35516_wr_cmd(0xE904); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE905); NT35516_wr_dat(0x000C);
		NT35516_wr_cmd(0xE906); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE907); NT35516_wr_dat(0x004D);
		NT35516_wr_cmd(0xE908); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE909); NT35516_wr_dat(0x0082);
		NT35516_wr_cmd(0xE90A); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE90B); NT35516_wr_dat(0x0084);
		NT35516_wr_cmd(0xE90C); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE90D); NT35516_wr_dat(0x00B8);
		NT35516_wr_cmd(0xE90E); NT35516_wr_dat(0x0002);
		NT35516_wr_cmd(0xE90F); NT35516_wr_dat(0x00F0);
		
		NT35516_wr_cmd(0xEA00); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA01); NT35516_wr_dat(0x0014);
		NT35516_wr_cmd(0xEA02); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA03); NT35516_wr_dat(0x0042);
		NT35516_wr_cmd(0xEA04); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA05); NT35516_wr_dat(0x005E);
		NT35516_wr_cmd(0xEA06); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA07); NT35516_wr_dat(0x0080);
		NT35516_wr_cmd(0xEA08); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA09); NT35516_wr_dat(0x0097);
		NT35516_wr_cmd(0xEA0A); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA0B); NT35516_wr_dat(0x00B0);
		NT35516_wr_cmd(0xEA0C); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA0D); NT35516_wr_dat(0x00C0);
		NT35516_wr_cmd(0xEA0E); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEA0F); NT35516_wr_dat(0x00DF);
		
		NT35516_wr_cmd(0xEB00); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEB01); NT35516_wr_dat(0x00FD);
		NT35516_wr_cmd(0xEB02); NT35516_wr_dat(0x0003);
		NT35516_wr_cmd(0xEB03); NT35516_wr_dat(0x00FF);
		
		//NT35516_wr_cmd(0x3600);NT35516_wr_dat(0x00C0);		/* MADCTL */
		NT35516_wr_cmd(0x3600); NT35516_wr_dat(0x0003);		/* MADCTL */
		NT35516_wr_cmd(0x3A00); NT35516_wr_dat(0x0055);		/* 0x0055=65K 0x0066=262K */
		
		/* Sleep out */
		NT35516_wr_cmd(0x1100);
		_delay_ms(150);	/* At least 120mSec */
		
		/* Display on */
		NT35516_wr_cmd(0x2900);
	}

	else { for(;;);} /* Invalid Device Code!! */

	NT35516_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	NT35516_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		NT35516_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
