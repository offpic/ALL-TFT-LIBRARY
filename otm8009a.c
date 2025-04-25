/********************************************************************************/
/*!
	@file			otm8009a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -INANBO-T43C-8009-V2		(OTM8009A)		16bit mode.		@n
					 -BBM397003I4				(OTM8012A)		16bit mode.		@n
					 -NSF397WV4402				(OTM8009A)		16bit mode.

    @section HISTORY
		2014.05.01	V1.00	Stable Release
		2014.08.03	V2.00	Added OTM8012A Device.
		2014.10.15	V3.00	Fixed 8-bit access bug.
		2015.05.15	V4.00	Added Normally White Screens.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "otm8009a.h"
/* check header file version for fool proof */
#if OTM8009A_H != 0x0600
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
/* If u want normally black screen, uncomment this. */
//#define USE_LCDTYPE_NB

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void OTM8009A_reset(void)
{
	OTM8009A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	OTM8009A_RD_SET();
	OTM8009A_WR_SET();
	_delay_ms(20);								/* wait 20ms     			*/

	OTM8009A_RES_CLR();							/* RES=L, CS=L   			*/
	OTM8009A_CS_CLR();
	_delay_ms(1);								/* wait 1ms     			*/
	
	OTM8009A_RES_SET();						  	/* RES=H					*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void OTM8009A_wr_cmd(uint16_t cmd)
{
	OTM8009A_DC_CLR();							/* DC=L					*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	OTM8009A_CMD = (uint8_t)(cmd>>8);			/* upper 8bit command	*/
	OTM8009A_WR();								/* WR=L->H				*/
	OTM8009A_CMD = (uint8_t)cmd;				/* lower 8bit command	*/
#else
	OTM8009A_CMD = cmd;							/* 16bit command		*/
#endif
	OTM8009A_WR();								/* WR=L->H				*/

	OTM8009A_DC_SET();							/* DC=H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void OTM8009A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	OTM8009A_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	OTM8009A_WR();								/* WR=L->H					*/
	OTM8009A_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	OTM8009A_DATA = dat;						/* 16bit data 				*/
#endif
	OTM8009A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void OTM8009A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		OTM8009A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		OTM8009A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void OTM8009A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	/* Notice */
	/* OTM8012A MUST need LSB of xSA to 1b0,LSB of xEA to 1b1. */
	OTM8009A_wr_cmd(0x2A00);					/* Horizontal RAM Start ADDR */
	OTM8009A_wr_dat((OFS_COL + x)>>8);
	OTM8009A_wr_cmd(0x2A01);
	OTM8009A_wr_dat((OFS_COL + x));
	OTM8009A_wr_cmd(0x2A02);					/* Horizontal RAM End ADDR */
	OTM8009A_wr_dat((OFS_COL + width)>>8);
	OTM8009A_wr_cmd(0x2A03);
	OTM8009A_wr_dat((OFS_COL + width));

	OTM8009A_wr_cmd(0x2B00);					/* Vertical RAM Start ADDR */
	OTM8009A_wr_dat((OFS_RAW + y)>>8);
	OTM8009A_wr_cmd(0x2B01);
	OTM8009A_wr_dat((OFS_RAW + y));
	OTM8009A_wr_cmd(0x2B02);					/* Vertical RAM End ADDR */
	OTM8009A_wr_dat((OFS_RAW + height)>>8);
	OTM8009A_wr_cmd(0x2B03);
	OTM8009A_wr_dat((OFS_RAW + height));

	OTM8009A_wr_cmd(0x2C00);					/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void OTM8009A_clear(void)
{
	volatile uint32_t n;

	OTM8009A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		OTM8009A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t OTM8009A_rd_cmd(uint16_t cmd)
{
	uint16_t val,temp;

	OTM8009A_wr_cmd(0xFF00); OTM8009A_wr_dat(0x0080); 
	OTM8009A_wr_cmd(0xFF01); OTM8009A_wr_dat(0x0009); 
	OTM8009A_wr_cmd(0xFF02); OTM8009A_wr_dat(0x0001); 

	OTM8009A_wr_cmd(0xFF80); OTM8009A_wr_dat(0x0080); 
	OTM8009A_wr_cmd(0xFF81); OTM8009A_wr_dat(0x0009); 

	OTM8009A_wr_cmd(cmd+2); 
	ReadLCDData(temp);	/* Dummy Read */
    ReadLCDData(temp);
	

	OTM8009A_wr_cmd(cmd+3); 
	ReadLCDData(val);	/* Dummy Read */
    ReadLCDData(val);

	OTM8009A_WR_SET();

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void OTM8009A_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	OTM8009A_reset();

	/* Check Device Code */
	devicetype = OTM8009A_rd_cmd(0xD200);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8009)
	{
		/* Initialize OTM8009A */
		OTM8009A_wr_cmd(0xFF00); OTM8009A_wr_dat(0x0080);
		OTM8009A_wr_cmd(0xFF01); OTM8009A_wr_dat(0x0009);		/* enable EXTC */
		OTM8009A_wr_cmd(0xFF02); OTM8009A_wr_dat(0x0001);
		OTM8009A_wr_cmd(0xFF80); OTM8009A_wr_dat(0x0080);		/* enable Orise mode */
		OTM8009A_wr_cmd(0xFF81); OTM8009A_wr_dat(0x0009);
		OTM8009A_wr_cmd(0xFF03); OTM8009A_wr_dat(0x0001);		/* enable SPI+I2C cmd2 read */
		
		/* gamma DC */
		OTM8009A_wr_cmd(0xC0B4); OTM8009A_wr_dat(0x0050);		/* column inversion */
		OTM8009A_wr_cmd(0xC489); OTM8009A_wr_dat(0x0008);		/* reg off */
		OTM8009A_wr_cmd(0xC0A3); OTM8009A_wr_dat(0x0000);		/* pre-charge V02 */
		OTM8009A_wr_cmd(0xC582); OTM8009A_wr_dat(0x00A3);		/* REG-pump23 */
		OTM8009A_wr_cmd(0xC590); OTM8009A_wr_dat(0x0096);		/* Pump setting (3x=D6)-->(2x=96)//v02 01/11 */
		OTM8009A_wr_cmd(0xC591); OTM8009A_wr_dat(0x0087);		/* Pump setting(VGH/VGL) */
		OTM8009A_wr_cmd(0xD800); OTM8009A_wr_dat(0x0073);		/* GVDD=4.5V  73 */
		OTM8009A_wr_cmd(0xD801); OTM8009A_wr_dat(0x0071);		/* NGVDD=4.5V 71 */
		
		/* VCOMDC */
		OTM8009A_wr_cmd(0xD900); OTM8009A_wr_dat(0x006A);		/*  VCOMDC */
		_delay_ms(20);
	  
		/* Positive */
		OTM8009A_wr_cmd(0xE100); OTM8009A_wr_dat(0x0009);
		OTM8009A_wr_cmd(0xE101); OTM8009A_wr_dat(0x000A);
		OTM8009A_wr_cmd(0xE102); OTM8009A_wr_dat(0x000E);
		OTM8009A_wr_cmd(0xE103); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE104); OTM8009A_wr_dat(0x0007);
		OTM8009A_wr_cmd(0xE105); OTM8009A_wr_dat(0x0018);
		OTM8009A_wr_cmd(0xE106); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE107); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE108); OTM8009A_wr_dat(0x0001);
		OTM8009A_wr_cmd(0xE109); OTM8009A_wr_dat(0x0004);
		OTM8009A_wr_cmd(0xE10A); OTM8009A_wr_dat(0x0005);
		OTM8009A_wr_cmd(0xE10B); OTM8009A_wr_dat(0x0006);
		OTM8009A_wr_cmd(0xE10C); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE10D); OTM8009A_wr_dat(0x0022);
		OTM8009A_wr_cmd(0xE10E); OTM8009A_wr_dat(0x0020);
		OTM8009A_wr_cmd(0xE10F); OTM8009A_wr_dat(0x0005);
		
		/* Negative */
		OTM8009A_wr_cmd(0xE200); OTM8009A_wr_dat(0x0009);
		OTM8009A_wr_cmd(0xE201); OTM8009A_wr_dat(0x000A);
		OTM8009A_wr_cmd(0xE202); OTM8009A_wr_dat(0x000E);
		OTM8009A_wr_cmd(0xE203); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE204); OTM8009A_wr_dat(0x0007);
		OTM8009A_wr_cmd(0xE205); OTM8009A_wr_dat(0x0018);
		OTM8009A_wr_cmd(0xE206); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE207); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE208); OTM8009A_wr_dat(0x0001);
		OTM8009A_wr_cmd(0xE209); OTM8009A_wr_dat(0x0004);
		OTM8009A_wr_cmd(0xE20A); OTM8009A_wr_dat(0x0005);
		OTM8009A_wr_cmd(0xE20B); OTM8009A_wr_dat(0x0006);
		OTM8009A_wr_cmd(0xE20C); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xE20D); OTM8009A_wr_dat(0x0022);
		OTM8009A_wr_cmd(0xE20E); OTM8009A_wr_dat(0x0020);
		OTM8009A_wr_cmd(0xE20F); OTM8009A_wr_dat(0x0005);
		
		OTM8009A_wr_cmd(0xC181); OTM8009A_wr_dat(0x0066);    	/* Frame rate 65Hz//V02 */
		
		/* RGB I/F setting VSYNC for OTM8018 0x0e */
		OTM8009A_wr_cmd(0xC1A1); OTM8009A_wr_dat(0x0008);    	/* external Vsync,Hsync,DE */
		OTM8009A_wr_cmd(0xC0A3); OTM8009A_wr_dat(0x001B);    	/* pre-charge	V02 */
		OTM8009A_wr_cmd(0xC481); OTM8009A_wr_dat(0x0083);    	/* source bias	V02 */
		OTM8009A_wr_cmd(0xC592); OTM8009A_wr_dat(0x0001);    	/* Pump45 */
		OTM8009A_wr_cmd(0xC5B1); OTM8009A_wr_dat(0x00A9);    	/* DC voltage setting ;[0]GVDD output, default: 0xa8 */
		
		/* CE8x : vst1, vst2, vst3, vst4 */
		OTM8009A_wr_cmd(0xCE80); OTM8009A_wr_dat(0x0085);		/* ce81[7:0] : vst1_shift[7:0] */
		OTM8009A_wr_cmd(0xCE81); OTM8009A_wr_dat(0x0003);		/* ce82[7:0] : 0000,	vst1_width[3:0] */
		OTM8009A_wr_cmd(0xCE82); OTM8009A_wr_dat(0x0000);		/* ce83[7:0] : vst1_tchop[7:0] */
		OTM8009A_wr_cmd(0xCE83); OTM8009A_wr_dat(0x0084);		/* ce84[7:0] : vst2_shift[7:0] */
		OTM8009A_wr_cmd(0xCE84); OTM8009A_wr_dat(0x0003);		/* ce85[7:0] : 0000,	vst2_width[3:0] */
		OTM8009A_wr_cmd(0xCE85); OTM8009A_wr_dat(0x0000);		/* ce86[7:0] : vst2_tchop[7:0] */
		OTM8009A_wr_cmd(0xCE86); OTM8009A_wr_dat(0x0083);		/* ce87[7:0] : vst3_shift[7:0] */
		OTM8009A_wr_cmd(0xCE87); OTM8009A_wr_dat(0x0003);		/* ce88[7:0] : 0000,	vst3_width[3:0] */
		OTM8009A_wr_cmd(0xCE88); OTM8009A_wr_dat(0x0000);		/* ce89[7:0] : vst3_tchop[7:0] */
		OTM8009A_wr_cmd(0xCE89); OTM8009A_wr_dat(0x0082);		/* ce8a[7:0] : vst4_shift[7:0] */
		OTM8009A_wr_cmd(0xCE8A); OTM8009A_wr_dat(0x0003);		/* ce8b[7:0] : 0000,	vst4_width[3:0] */
		OTM8009A_wr_cmd(0xCE8B); OTM8009A_wr_dat(0x0000);		/* ce8c[7:0] : vst4_tchop[7:0] */
		
		/* CEAx : clka1, clka2 */
		OTM8009A_wr_cmd(0xCEA0); OTM8009A_wr_dat(0x0038);		/* cea1[7:0] : clka1_width[3:0], clka1_shift[11:8] */
		OTM8009A_wr_cmd(0xCEA1); OTM8009A_wr_dat(0x0002);		/* cea2[7:0] : clka1_shift[7:0] */
		OTM8009A_wr_cmd(0xCEA2); OTM8009A_wr_dat(0x0003);		/* cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]  */
		OTM8009A_wr_cmd(0xCEA3); OTM8009A_wr_dat(0x0021);		/* cea4[7:0] : clka1_switch[7:0] */
		OTM8009A_wr_cmd(0xCEA4); OTM8009A_wr_dat(0x0000);		/* cea5[7:0] : clka1_extend[7:0] */
		OTM8009A_wr_cmd(0xCEA5); OTM8009A_wr_dat(0x0000);		/* cea6[7:0] : clka1_tchop[7:0] */
		OTM8009A_wr_cmd(0xCEA6); OTM8009A_wr_dat(0x0000);		/* cea7[7:0] : clka1_tglue[7:0] */
		OTM8009A_wr_cmd(0xCEA7); OTM8009A_wr_dat(0x0038);		/* cea8[7:0] : clka2_width[3:0], clka2_shift[11:8] */
		OTM8009A_wr_cmd(0xCEA8); OTM8009A_wr_dat(0x0001);		/* cea9[7:0] : clka2_shift[7:0] */
		OTM8009A_wr_cmd(0xCEA9); OTM8009A_wr_dat(0x0003);		/* ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEAA); OTM8009A_wr_dat(0x0022);		/* ceab[7:0] : clka2_switch[7:0] */
		OTM8009A_wr_cmd(0xCEAB); OTM8009A_wr_dat(0x0000);		/* ceac[7:0] : clka2_extend */
		OTM8009A_wr_cmd(0xCEAC); OTM8009A_wr_dat(0x0000);		/* cead[7:0] : clka2_tchop */
		OTM8009A_wr_cmd(0xCEAD); OTM8009A_wr_dat(0x0000);		/* ceae[7:0] : clka2_tglue */
		
		/* CEBx : clka3, clka4 */
		OTM8009A_wr_cmd(0xCEB0); OTM8009A_wr_dat(0x0038);		/* ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8] */
		OTM8009A_wr_cmd(0xCEB1); OTM8009A_wr_dat(0x0000);		/* ceb2[7:0] : clka3_shift[7:0] */
		OTM8009A_wr_cmd(0xCEB2); OTM8009A_wr_dat(0x0003);		/* ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEB3); OTM8009A_wr_dat(0x0023);		/* ceb4[7:0] : clka3_switch[7:0] */
		OTM8009A_wr_cmd(0xCEB4); OTM8009A_wr_dat(0x0000);		/* ceb5[7:0] : clka3_extend[7:0] */
		OTM8009A_wr_cmd(0xCEB5); OTM8009A_wr_dat(0x0000);		/* ceb6[7:0] : clka3_tchop[7:0] */
		OTM8009A_wr_cmd(0xCEB6); OTM8009A_wr_dat(0x0000);		/* ceb7[7:0] : clka3_tglue[7:0] */
		OTM8009A_wr_cmd(0xCEB7); OTM8009A_wr_dat(0x0030);		/* ceb8[7:0] : clka4_width[3:0], clka2_shift[11:8] */
		OTM8009A_wr_cmd(0xCEB8); OTM8009A_wr_dat(0x0000);		/* ceb9[7:0] : clka4_shift[7:0] */
		OTM8009A_wr_cmd(0xCEB9); OTM8009A_wr_dat(0x0003);		/* ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEBA); OTM8009A_wr_dat(0x0024);		/* cebb[7:0] : clka4_switch[7:0] */
		OTM8009A_wr_cmd(0xCEBB); OTM8009A_wr_dat(0x0000);		/* cebc[7:0] : clka4_extend */
		OTM8009A_wr_cmd(0xCEBC); OTM8009A_wr_dat(0x0000);		/* cebd[7:0] : clka4_tchop */
		OTM8009A_wr_cmd(0xCEBD); OTM8009A_wr_dat(0x0000);		/* cebe[7:0] : clka4_tglue */
		
		/* CECx : clkb1, clkb2 */
		OTM8009A_wr_cmd(0xCEC0); OTM8009A_wr_dat(0x0030);		/* cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8] */
		OTM8009A_wr_cmd(0xCEC1); OTM8009A_wr_dat(0x0001);		/* cec2[7:0] : clkb1_shift[7:0] */                 
		OTM8009A_wr_cmd(0xCEC2); OTM8009A_wr_dat(0x0003);		/* cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEC3); OTM8009A_wr_dat(0x0025);		/* cec4[7:0] : clkb1_switch[7:0] */
		OTM8009A_wr_cmd(0xCEC4); OTM8009A_wr_dat(0x0000);		/* cec5[7:0] : clkb1_extend[7:0] */
		OTM8009A_wr_cmd(0xCEC5); OTM8009A_wr_dat(0x0000);		/* cec6[7:0] : clkb1_tchop[7:0] */
		OTM8009A_wr_cmd(0xCEC6); OTM8009A_wr_dat(0x0000);		/* cec7[7:0] : clkb1_tglue[7:0] */
		OTM8009A_wr_cmd(0xCEC7); OTM8009A_wr_dat(0x0030);		/* cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8] */
		OTM8009A_wr_cmd(0xCEC8); OTM8009A_wr_dat(0x0002);		/* cec9[7:0] : clkb2_shift[7:0] */
		OTM8009A_wr_cmd(0xCEC9); OTM8009A_wr_dat(0x0003);		/* ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCECA); OTM8009A_wr_dat(0x0026);		/* cecb[7:0] : clkb2_switch[7:0] */
		OTM8009A_wr_cmd(0xCECB); OTM8009A_wr_dat(0x0000);		/* cecc[7:0] : clkb2_extend */
		OTM8009A_wr_cmd(0xCECC); OTM8009A_wr_dat(0x0000);		/* cecd[7:0] : clkb2_tchop */  
		OTM8009A_wr_cmd(0xCECD); OTM8009A_wr_dat(0x0000);		/* cece[7:0] : clkb2_tglue  */
		
		/* CEDx : clkb3, clkb4 */
		OTM8009A_wr_cmd(0xCED0); OTM8009A_wr_dat(0x0030);		/* ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8] */
		OTM8009A_wr_cmd(0xCED1); OTM8009A_wr_dat(0x0003);		/* ced2[7:0] : clkb3_shift[7:0] */
		OTM8009A_wr_cmd(0xCED2); OTM8009A_wr_dat(0x0003);		/* ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCED3); OTM8009A_wr_dat(0x0027);		/* ced4[7:0] : clkb3_switch[7:0] */
		OTM8009A_wr_cmd(0xCED4); OTM8009A_wr_dat(0x0000);		/* ced5[7:0] : clkb3_extend[7:0] */
		OTM8009A_wr_cmd(0xCED5); OTM8009A_wr_dat(0x0000);		/* ced6[7:0] : clkb3_tchop[7:0] */
		OTM8009A_wr_cmd(0xCED6); OTM8009A_wr_dat(0x0000);		/* ced7[7:0] : clkb3_tglue[7:0] */
		OTM8009A_wr_cmd(0xCED7); OTM8009A_wr_dat(0x0030);		/* ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8] */
		OTM8009A_wr_cmd(0xCED8); OTM8009A_wr_dat(0x0004);		/* ced9[7:0] : clkb4_shift[7:0] */
		OTM8009A_wr_cmd(0xCED9); OTM8009A_wr_dat(0x0003);		/* ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEDA); OTM8009A_wr_dat(0x0028);		/* cedb[7:0] : clkb4_switch[7:0] */
		OTM8009A_wr_cmd(0xCEDB); OTM8009A_wr_dat(0x0000);		/* cedc[7:0] : clkb4_extend */
		OTM8009A_wr_cmd(0xCEDC); OTM8009A_wr_dat(0x0000);		/* cedd[7:0] : clkb4_tchop */
		OTM8009A_wr_cmd(0xCEDD); OTM8009A_wr_dat(0x0000);		/* cede[7:0] : clkb4_tglue */
		
		/* CFCx : */        
		OTM8009A_wr_cmd(0xCFC0); OTM8009A_wr_dat(0x0000);		/* cfc1[7:0] : eclk_normal_width[7:0] */
		OTM8009A_wr_cmd(0xCFC1); OTM8009A_wr_dat(0x0000);		/* cfc2[7:0] : eclk_partial_width[7:0] */
		OTM8009A_wr_cmd(0xCFC2); OTM8009A_wr_dat(0x0000);		/* cfc3[7:0] : all_normal_tchop[7:0] */
		OTM8009A_wr_cmd(0xCFC3); OTM8009A_wr_dat(0x0000);		/* cfc4[7:0] : all_partial_tchop[7:0] */
		OTM8009A_wr_cmd(0xCFC4); OTM8009A_wr_dat(0x0000);		/* cfc5[7:0] : eclk1_follow[3:0], eclk2_follow[3:0] */
		OTM8009A_wr_cmd(0xCFC5); OTM8009A_wr_dat(0x0000);		/* cfc6[7:0] : eclk3_follow[3:0], eclk4_follow[3:0] */
		OTM8009A_wr_cmd(0xCFC6); OTM8009A_wr_dat(0x0000);		/* cfc7[7:0] : 00, vstmask, vendmask, 00, dir1, dir2 (0=VGL, 1=VGH) */ 
		OTM8009A_wr_cmd(0xCFC7); OTM8009A_wr_dat(0x0000);		/* cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4 */
		OTM8009A_wr_cmd(0xCFC8); OTM8009A_wr_dat(0x0000);		/* cfc9[7:0] : duty_block[3:0], DGPM[3:0] */
		OTM8009A_wr_cmd(0xCFC9); OTM8009A_wr_dat(0x0000);		/* cfca[7:0] : reg_goa_gnd_period[7:0] */
		
		/* CFDx : */
		OTM8009A_wr_cmd(0xCFD0); OTM8009A_wr_dat(0x0000);		/* cfd1[7:0] : 0000000, reg_goa_frame_odd_high */
		
		/* PARAMETER 1 */
		/* ------------------------------------------------------------------------------ */
		/* 		initial setting 3 < Panel setting >											*/
		/* ------------------------------------------------------------------------------ */
		/* cbcx */
		OTM8009A_wr_cmd(0xCBC0); OTM8009A_wr_dat(0x0000);		/* cbc1[7:0] : enmode H-byte of sig1  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC1); OTM8009A_wr_dat(0x0000);		/* cbc2[7:0] : enmode H-byte of sig2  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC2); OTM8009A_wr_dat(0x0000);		/* cbc3[7:0] : enmode H-byte of sig3  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC3); OTM8009A_wr_dat(0x0000);		/* cbc4[7:0] : enmode H-byte of sig4  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC4); OTM8009A_wr_dat(0x0004);		/* cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC5); OTM8009A_wr_dat(0x0004);		/* cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC6); OTM8009A_wr_dat(0x0004);		/* cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC7); OTM8009A_wr_dat(0x0004);		/* cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC8); OTM8009A_wr_dat(0x0004);		/* cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBC9); OTM8009A_wr_dat(0x0004);		/* cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBCA); OTM8009A_wr_dat(0x0000);		/* cbcb[7:0] : enmode H-byte of sig11 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBCB); OTM8009A_wr_dat(0x0000);		/* cbcc[7:0] : enmode H-byte of sig12 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBCC); OTM8009A_wr_dat(0x0000);		/* cbcd[7:0] : enmode H-byte of sig13 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBCD); OTM8009A_wr_dat(0x0000);		/* cbce[7:0] : enmode H-byte of sig14 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBCE); OTM8009A_wr_dat(0x0000);		/* cbcf[7:0] : enmode H-byte of sig15 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		
		/* cbdx */    
		OTM8009A_wr_cmd(0xCBD0); OTM8009A_wr_dat(0x0000);		/* cbd1[7:0] : enmode H-byte of sig16 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD1); OTM8009A_wr_dat(0x0000);		/* cbd2[7:0] : enmode H-byte of sig17 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD2); OTM8009A_wr_dat(0x0000);		/* cbd3[7:0] : enmode H-byte of sig18 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD3); OTM8009A_wr_dat(0x0000);		/* cbd4[7:0] : enmode H-byte of sig19 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD4); OTM8009A_wr_dat(0x0000);		/* cbd5[7:0] : enmode H-byte of sig20 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD5); OTM8009A_wr_dat(0x0000);		/* cbd6[7:0] : enmode H-byte of sig21 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD6); OTM8009A_wr_dat(0x0000);		/* cbd7[7:0] : enmode H-byte of sig22 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD7); OTM8009A_wr_dat(0x0000);		/* cbd8[7:0] : enmode H-byte of sig23 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD8); OTM8009A_wr_dat(0x0000);		/* cbd9[7:0] : enmode H-byte of sig24 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBD9); OTM8009A_wr_dat(0x0004);		/* cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBDA); OTM8009A_wr_dat(0x0004);		/* cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBDB); OTM8009A_wr_dat(0x0004);		/* cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBDC); OTM8009A_wr_dat(0x0004);		/* cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBDD); OTM8009A_wr_dat(0x0004);		/* cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBDE); OTM8009A_wr_dat(0x0004);		/* cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		
		/* cbex */
		OTM8009A_wr_cmd(0xCBE0); OTM8009A_wr_dat(0x0000);		/* cbe1[7:0] : enmode H-byte of sig31 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE1); OTM8009A_wr_dat(0x0000);		/* cbe2[7:0] : enmode H-byte of sig32 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE2); OTM8009A_wr_dat(0x0000);		/* cbe3[7:0] : enmode H-byte of sig33 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE3); OTM8009A_wr_dat(0x0000);		/* cbe4[7:0] : enmode H-byte of sig34 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE4); OTM8009A_wr_dat(0x0000);		/* cbe5[7:0] : enmode H-byte of sig35 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE5); OTM8009A_wr_dat(0x0000);		/* cbe6[7:0] : enmode H-byte of sig36 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE6); OTM8009A_wr_dat(0x0000);		/* cbe7[7:0] : enmode H-byte of sig37 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE7); OTM8009A_wr_dat(0x0000);		/* cbe8[7:0] : enmode H-byte of sig38 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE8); OTM8009A_wr_dat(0x0000);		/* cbe9[7:0] : enmode H-byte of sig39 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		OTM8009A_wr_cmd(0xCBE9); OTM8009A_wr_dat(0x0000);		/* cbea[7:0] : enmode H-byte of sig40 (pwrof_0, pwrof_1, norm, pwron_4 ) */
		
		/* cc8x */                 
		OTM8009A_wr_cmd(0xCC80); OTM8009A_wr_dat(0x0000);		/* cc81[7:0] : reg setting for signal01 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC81); OTM8009A_wr_dat(0x0000);		/* cc82[7:0] : reg setting for signal02 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC82); OTM8009A_wr_dat(0x0000);		/* cc83[7:0] : reg setting for signal03 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC83); OTM8009A_wr_dat(0x0000);		/* cc84[7:0] : reg setting for signal04 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC84); OTM8009A_wr_dat(0x000C);		/* cc85[7:0] : reg setting for signal05 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC85); OTM8009A_wr_dat(0x000A);		/* cc86[7:0] : reg setting for signal06 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC86); OTM8009A_wr_dat(0x0010);		/* cc87[7:0] : reg setting for signal07 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC87); OTM8009A_wr_dat(0x000E);		/* cc88[7:0] : reg setting for signal08 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC88); OTM8009A_wr_dat(0x0003);		/* cc89[7:0] : reg setting for signal09 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC89); OTM8009A_wr_dat(0x0004);		/* cc8a[7:0] : reg setting for signal10 selection with u2d mode */
		
		/* cc9x */    
		OTM8009A_wr_cmd(0xCC90); OTM8009A_wr_dat(0x0000);		/* cc91[7:0] : reg setting for signal11 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC91); OTM8009A_wr_dat(0x0000);		/* cc92[7:0] : reg setting for signal12 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC92); OTM8009A_wr_dat(0x0000);		/* cc93[7:0] : reg setting for signal13 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC93); OTM8009A_wr_dat(0x0000);		/* cc94[7:0] : reg setting for signal14 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC94); OTM8009A_wr_dat(0x0000);		/* cc95[7:0] : reg setting for signal15 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC95); OTM8009A_wr_dat(0x0000);		/* cc96[7:0] : reg setting for signal16 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC96); OTM8009A_wr_dat(0x0000);		/* cc97[7:0] : reg setting for signal17 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC97); OTM8009A_wr_dat(0x0000);		/* cc98[7:0] : reg setting for signal18 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC98); OTM8009A_wr_dat(0x0000);		/* cc99[7:0] : reg setting for signal19 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC99); OTM8009A_wr_dat(0x0000);		/* cc9a[7:0] : reg setting for signal20 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC9A); OTM8009A_wr_dat(0x0000);		/* cc9b[7:0] : reg setting for signal21 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC9B); OTM8009A_wr_dat(0x0000);		/* cc9c[7:0] : reg setting for signal22 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC9C); OTM8009A_wr_dat(0x0000);		/* cc9d[7:0] : reg setting for signal23 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC9D); OTM8009A_wr_dat(0x0000);		/* cc9e[7:0] : reg setting for signal24 selection with u2d mode */
		OTM8009A_wr_cmd(0xCC9E); OTM8009A_wr_dat(0x000B);		/* cc9f[7:0] : reg setting for signal25 selection with u2d mode */
		
		/* ccax */    
		OTM8009A_wr_cmd(0xCCA0); OTM8009A_wr_dat(0x0009);		/* cca1[7:0] : reg setting for signal26 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA1); OTM8009A_wr_dat(0x000F);		/* cca2[7:0] : reg setting for signal27 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA2); OTM8009A_wr_dat(0x000D);		/* cca3[7:0] : reg setting for signal28 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA3); OTM8009A_wr_dat(0x0001);		/* cca4[7:0] : reg setting for signal29 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA4); OTM8009A_wr_dat(0x0002);		/* cca5[7:0] : reg setting for signal20 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA5); OTM8009A_wr_dat(0x0000);		/* cca6[7:0] : reg setting for signal31 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA6); OTM8009A_wr_dat(0x0000);		/* cca7[7:0] : reg setting for signal32 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA7); OTM8009A_wr_dat(0x0000);		/* cca8[7:0] : reg setting for signal33 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA8); OTM8009A_wr_dat(0x0000);		/* cca9[7:0] : reg setting for signal34 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCA9); OTM8009A_wr_dat(0x0000);		/* ccaa[7:0] : reg setting for signal35 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCAA); OTM8009A_wr_dat(0x0000);		/* ccab[7:0] : reg setting for signal36 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCAB); OTM8009A_wr_dat(0x0000);		/* ccac[7:0] : reg setting for signal37 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCAC); OTM8009A_wr_dat(0x0000);		/* ccad[7:0] : reg setting for signal38 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCAD); OTM8009A_wr_dat(0x0000);		/* ccae[7:0] : reg setting for signal39 selection with u2d mode */
		OTM8009A_wr_cmd(0xCCAE); OTM8009A_wr_dat(0x0000);		/* ccaf[7:0] : reg setting for signal40 selection with u2d mode */
		
		/* ccbx */    
		OTM8009A_wr_cmd(0xCCB0); OTM8009A_wr_dat(0x0000);		/* ccb1[7:0] : reg setting for signal01 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB1); OTM8009A_wr_dat(0x0000);		/* ccb2[7:0] : reg setting for signal02 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB2); OTM8009A_wr_dat(0x0000);		/* ccb3[7:0] : reg setting for signal03 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB3); OTM8009A_wr_dat(0x0000);		/* ccb4[7:0] : reg setting for signal04 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB4); OTM8009A_wr_dat(0x000D);		/* ccb5[7:0] : reg setting for signal05 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB5); OTM8009A_wr_dat(0x000F);		/* ccb6[7:0] : reg setting for signal06 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB6); OTM8009A_wr_dat(0x0009);		/* ccb7[7:0] : reg setting for signal07 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB7); OTM8009A_wr_dat(0x000B);		/* ccb8[7:0] : reg setting for signal08 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB8); OTM8009A_wr_dat(0x0002);		/* ccb9[7:0] : reg setting for signal09 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCB9); OTM8009A_wr_dat(0x0001);		/* ccba[7:0] : reg setting for signal10 selection with d2u mode */
		
		/* cccx */    
		OTM8009A_wr_cmd(0xCCC0); OTM8009A_wr_dat(0x0000);		/* ccc1[7:0] : reg setting for signal11 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC1); OTM8009A_wr_dat(0x0000);		/* ccc2[7:0] : reg setting for signal12 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC2); OTM8009A_wr_dat(0x0000);		/* ccc3[7:0] : reg setting for signal13 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC3); OTM8009A_wr_dat(0x0000);		/* ccc4[7:0] : reg setting for signal14 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC4); OTM8009A_wr_dat(0x0000);		/* ccc5[7:0] : reg setting for signal15 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC5); OTM8009A_wr_dat(0x0000);		/* ccc6[7:0] : reg setting for signal16 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC6); OTM8009A_wr_dat(0x0000);		/* ccc7[7:0] : reg setting for signal17 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC7); OTM8009A_wr_dat(0x0000);		/* ccc8[7:0] : reg setting for signal18 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC8); OTM8009A_wr_dat(0x0000);		/* ccc9[7:0] : reg setting for signal19 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCC9); OTM8009A_wr_dat(0x0000);		/* ccca[7:0] : reg setting for signal20 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCCA); OTM8009A_wr_dat(0x0000);		/* cccb[7:0] : reg setting for signal21 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCCB); OTM8009A_wr_dat(0x0000);		/* cccc[7:0] : reg setting for signal22 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCCC); OTM8009A_wr_dat(0x0000);		/* cccd[7:0] : reg setting for signal23 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCCD); OTM8009A_wr_dat(0x0000);		/* ccce[7:0] : reg setting for signal24 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCCE); OTM8009A_wr_dat(0x000E);		/* cccf[7:0] : reg setting for signal25 selection with d2u mode */
		
		/* ccdx */    
		OTM8009A_wr_cmd(0xCCD0); OTM8009A_wr_dat(0x0010);		/* ccd1[7:0] : reg setting for signal26 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD1); OTM8009A_wr_dat(0x000A);		/* ccd2[7:0] : reg setting for signal27 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD2); OTM8009A_wr_dat(0x000C);		/* ccd3[7:0] : reg setting for signal28 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD3); OTM8009A_wr_dat(0x0004);		/* ccd4[7:0] : reg setting for signal29 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD4); OTM8009A_wr_dat(0x0003);		/* ccd5[7:0] : reg setting for signal30 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD5); OTM8009A_wr_dat(0x0000);		/* ccd6[7:0] : reg setting for signal31 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD6); OTM8009A_wr_dat(0x0000);		/* ccd7[7:0] : reg setting for signal32 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD7); OTM8009A_wr_dat(0x0000);		/* ccd8[7:0] : reg setting for signal33 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD8); OTM8009A_wr_dat(0x0000);		/* ccd9[7:0] : reg setting for signal34 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCD9); OTM8009A_wr_dat(0x0000);		/* ccda[7:0] : reg setting for signal35 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCDA); OTM8009A_wr_dat(0x0000);		/* ccdb[7:0] : reg setting for signal36 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCDB); OTM8009A_wr_dat(0x0000);		/* ccdc[7:0] : reg setting for signal37 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCDC); OTM8009A_wr_dat(0x0000);		/* ccdd[7:0] : reg setting for signal38 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCDD); OTM8009A_wr_dat(0x0000);		/* ccde[7:0] : reg setting for signal39 selection with d2u mode */
		OTM8009A_wr_cmd(0xCCDE); OTM8009A_wr_dat(0x0000);		/* ccdf[7:0] : reg setting for signal40 selection with d2u mode */
		
		OTM8009A_wr_cmd(0x3A00); OTM8009A_wr_dat(0x0055);    	/* MCU 16bits D[17:0] */
		OTM8009A_wr_cmd(0x3600); OTM8009A_wr_dat(0x0000);    	/* BGR=0 */
		
#ifdef USE_LCDTYPE_NB
		#warning "USE Normally Black Screen!!!!"
		OTM8009A_wr_cmd(0x2100);								/* display invertion on */
#endif
		OTM8009A_wr_cmd(0x1100);	/* Sleep OFF */
		_delay_ms(150);	
		OTM8009A_wr_cmd(0x2900);	/* Diaplay ON */
		_delay_ms(200);
	}

	else if(devicetype == 0x8012)
	{
		/* Initialize OTM8012A */
		#warning "S**kly,OTM8012A has HALF-RAM Structure ! So You CANNOT use several GFX sub Functions....! Too F**K!"
		OTM8009A_wr_cmd(0xFF00); OTM8009A_wr_dat(0x0080);
		OTM8009A_wr_cmd(0xFF01); OTM8009A_wr_dat(0x0012);		/* enable EXTC */
		OTM8009A_wr_cmd(0xFF02); OTM8009A_wr_dat(0x0001);
		OTM8009A_wr_cmd(0xFF80); OTM8009A_wr_dat(0x0080);		/* enable Orise mode */
		OTM8009A_wr_cmd(0xFF81); OTM8009A_wr_dat(0x0012);
		OTM8009A_wr_cmd(0xFF03); OTM8009A_wr_dat(0x0001);		/* enable SPI+I2C cmd2 read */

		OTM8009A_wr_cmd(0xC090); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xC091); OTM8009A_wr_dat(0x004C);
		OTM8009A_wr_cmd(0xC092); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xC093); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xC094); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xC095); OTM8009A_wr_dat(0x000F);;

		OTM8009A_wr_cmd(0xC180); OTM8009A_wr_dat(0x0045);
		OTM8009A_wr_cmd(0xC181); OTM8009A_wr_dat(0x0055);

		OTM8009A_wr_cmd(0xC480); OTM8009A_wr_dat(0x0030);
		OTM8009A_wr_cmd(0xC481); OTM8009A_wr_dat(0x0084);
	
		OTM8009A_wr_cmd(0xC580); OTM8009A_wr_dat(0x0050);
		OTM8009A_wr_cmd(0xC581); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xC582); OTM8009A_wr_dat(0x00F1);
		OTM8009A_wr_cmd(0xC583); OTM8009A_wr_dat(0x0000);

		OTM8009A_wr_cmd(0xC590); OTM8009A_wr_dat(0x0003);
		OTM8009A_wr_cmd(0xC591); OTM8009A_wr_dat(0x0087);
		OTM8009A_wr_cmd(0xC594); OTM8009A_wr_dat(0x0044);
		OTM8009A_wr_cmd(0xC595); OTM8009A_wr_dat(0x0044);
		OTM8009A_wr_cmd(0xC596); OTM8009A_wr_dat(0x0046);

		OTM8009A_wr_cmd(0xCBC0); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBC1); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBC2); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBC3); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBC4); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBC5); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBC6); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBC7); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBC8); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBC9); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBCA); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBCB); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBCC); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBCD); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBCE); OTM8009A_wr_dat(0x0000);

		OTM8009A_wr_cmd(0xCBD0); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD1); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD2); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD3); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD4); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD5); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD6); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD7); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD8); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBD9); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBDA); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBDB); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBDC); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBDD); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBDE); OTM8009A_wr_dat(0x0055);

		OTM8009A_wr_cmd(0xCBE0); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBE1); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBE2); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0xCBE3); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBE4); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCBE5); OTM8009A_wr_dat(0x0000);

		OTM8009A_wr_cmd(0xCC80); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC81); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC82); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC83); OTM8009A_wr_dat(0x0003);
		OTM8009A_wr_cmd(0xCC84); OTM8009A_wr_dat(0x0001);
		OTM8009A_wr_cmd(0xCC85); OTM8009A_wr_dat(0x0009);
		OTM8009A_wr_cmd(0xCC86); OTM8009A_wr_dat(0x000B);
		OTM8009A_wr_cmd(0xCC87); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xCC88); OTM8009A_wr_dat(0x000F);
		OTM8009A_wr_cmd(0xCC89); OTM8009A_wr_dat(0x0005);

		OTM8009A_wr_cmd(0xCC90); OTM8009A_wr_dat(0x0007);
		OTM8009A_wr_cmd(0xCC91); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC92); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC93); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC94); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC95); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC96); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC97); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC98); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC99); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC9A); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC9B); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC9C); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCC9D); OTM8009A_wr_dat(0x0008);
		OTM8009A_wr_cmd(0xCC9E); OTM8009A_wr_dat(0x0006);

		OTM8009A_wr_cmd(0xCCA0); OTM8009A_wr_dat(0x0008);
		OTM8009A_wr_cmd(0xCCA1); OTM8009A_wr_dat(0x0006);
		OTM8009A_wr_cmd(0xCCA2); OTM8009A_wr_dat(0x0010);
		OTM8009A_wr_cmd(0xCCA3); OTM8009A_wr_dat(0x000E);
		OTM8009A_wr_cmd(0xCCA4); OTM8009A_wr_dat(0x000C);
		OTM8009A_wr_cmd(0xCCA5); OTM8009A_wr_dat(0x000A);
		OTM8009A_wr_cmd(0xCCA6); OTM8009A_wr_dat(0x0002);
		OTM8009A_wr_cmd(0xCCA7); OTM8009A_wr_dat(0x0004);
		OTM8009A_wr_cmd(0xCCA8); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCA9); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCAA); OTM8009A_wr_dat(0x0000);

		OTM8009A_wr_cmd(0xCCB0); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCB1); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCB2); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCB3); OTM8009A_wr_dat(0x0006);
		OTM8009A_wr_cmd(0xCCB4); OTM8009A_wr_dat(0x0008);
		OTM8009A_wr_cmd(0xCCB5); OTM8009A_wr_dat(0x000C);
		OTM8009A_wr_cmd(0xCCB6); OTM8009A_wr_dat(0x000A);
		OTM8009A_wr_cmd(0xCCB7); OTM8009A_wr_dat(0x0010);
		OTM8009A_wr_cmd(0xCCB8); OTM8009A_wr_dat(0x000E);
		OTM8009A_wr_cmd(0xCCB9); OTM8009A_wr_dat(0x0004);

		OTM8009A_wr_cmd(0xCCC0); OTM8009A_wr_dat(0x0002);
		OTM8009A_wr_cmd(0xCCC1); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC2); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC3); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC4); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC5); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC6); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC7); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC8); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCC9); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCCA); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCCB); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCCC); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCCD); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCCE); OTM8009A_wr_dat(0x0000);

		OTM8009A_wr_cmd(0xCCD0); OTM8009A_wr_dat(0x0001);
		OTM8009A_wr_cmd(0xCCD1); OTM8009A_wr_dat(0x0003);
		OTM8009A_wr_cmd(0xCCD2); OTM8009A_wr_dat(0x000D);
		OTM8009A_wr_cmd(0xCCD3); OTM8009A_wr_dat(0x000F);
		OTM8009A_wr_cmd(0xCCD4); OTM8009A_wr_dat(0x0009);
		OTM8009A_wr_cmd(0xCCD5); OTM8009A_wr_dat(0x000B);
		OTM8009A_wr_cmd(0xCCD6); OTM8009A_wr_dat(0x0007);
		OTM8009A_wr_cmd(0xCCD7); OTM8009A_wr_dat(0x0005);
		OTM8009A_wr_cmd(0xCCD8); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCD9); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0xCCDA); OTM8009A_wr_dat(0x0000);

		/* CE8x : vst1, vst2, vst3, vst4 */
		OTM8009A_wr_cmd(0xCE80); OTM8009A_wr_dat(0x0085);		/* ce81[7:0] : vst1_shift[7:0] */
		OTM8009A_wr_cmd(0xCE81); OTM8009A_wr_dat(0x0003);		/* ce82[7:0] : 0000,	vst1_width[3:0] */
		OTM8009A_wr_cmd(0xCE82); OTM8009A_wr_dat(0x0000);		/* ce83[7:0] : vst1_tchop[7:0] */
		OTM8009A_wr_cmd(0xCE83); OTM8009A_wr_dat(0x0084);		/* ce84[7:0] : vst2_shift[7:0] */
		OTM8009A_wr_cmd(0xCE84); OTM8009A_wr_dat(0x0003);		/* ce85[7:0] : 0000,	vst2_width[3:0] */
		OTM8009A_wr_cmd(0xCE85); OTM8009A_wr_dat(0x0000);		/* ce86[7:0] : vst2_tchop[7:0] */
		OTM8009A_wr_cmd(0xCE86); OTM8009A_wr_dat(0x0083);		/* ce87[7:0] : vst3_shift[7:0] */
		OTM8009A_wr_cmd(0xCE87); OTM8009A_wr_dat(0x0003);		/* ce88[7:0] : 0000,	vst3_width[3:0] */
		OTM8009A_wr_cmd(0xCE88); OTM8009A_wr_dat(0x0000);		/* ce89[7:0] : vst3_tchop[7:0] */
		OTM8009A_wr_cmd(0xCE89); OTM8009A_wr_dat(0x0082);		/* ce8a[7:0] : vst4_shift[7:0] */
		OTM8009A_wr_cmd(0xCE8A); OTM8009A_wr_dat(0x0003);		/* ce8b[7:0] : 0000,	vst4_width[3:0] */
		OTM8009A_wr_cmd(0xCE8B); OTM8009A_wr_dat(0x0000);		/* ce8c[7:0] : vst4_tchop[7:0] */

		/* CEAx : clka1, clka2 */
		OTM8009A_wr_cmd(0xCEA0); OTM8009A_wr_dat(0x0038);		/* cea1[7:0] : clka1_width[3:0], clka1_shift[11:8] */
		OTM8009A_wr_cmd(0xCEA1); OTM8009A_wr_dat(0x0002);		/* cea2[7:0] : clka1_shift[7:0] */
		OTM8009A_wr_cmd(0xCEA2); OTM8009A_wr_dat(0x0003);		/* cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]  */
		OTM8009A_wr_cmd(0xCEA3); OTM8009A_wr_dat(0x0021);		/* cea4[7:0] : clka1_switch[7:0] */
		OTM8009A_wr_cmd(0xCEA4); OTM8009A_wr_dat(0x0000);		/* cea5[7:0] : clka1_extend[7:0] */
		OTM8009A_wr_cmd(0xCEA5); OTM8009A_wr_dat(0x0000);		/* cea6[7:0] : clka1_tchop[7:0] */
		OTM8009A_wr_cmd(0xCEA6); OTM8009A_wr_dat(0x0000);		/* cea7[7:0] : clka1_tglue[7:0] */
		OTM8009A_wr_cmd(0xCEA7); OTM8009A_wr_dat(0x0038);		/* cea8[7:0] : clka2_width[3:0], clka2_shift[11:8] */
		OTM8009A_wr_cmd(0xCEA8); OTM8009A_wr_dat(0x0001);		/* cea9[7:0] : clka2_shift[7:0] */
		OTM8009A_wr_cmd(0xCEA9); OTM8009A_wr_dat(0x0003);		/* ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEAA); OTM8009A_wr_dat(0x0022);		/* ceab[7:0] : clka2_switch[7:0] */
		OTM8009A_wr_cmd(0xCEAB); OTM8009A_wr_dat(0x0000);		/* ceac[7:0] : clka2_extend */
		OTM8009A_wr_cmd(0xCEAC); OTM8009A_wr_dat(0x0000);		/* cead[7:0] : clka2_tchop */
		OTM8009A_wr_cmd(0xCEAD); OTM8009A_wr_dat(0x0000);		/* ceae[7:0] : clka2_tglue */

		/* CEBx : clka3, clka4 */
		OTM8009A_wr_cmd(0xCEB0); OTM8009A_wr_dat(0x0038);		/* ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8] */
		OTM8009A_wr_cmd(0xCEB1); OTM8009A_wr_dat(0x0000);		/* ceb2[7:0] : clka3_shift[7:0] */
		OTM8009A_wr_cmd(0xCEB2); OTM8009A_wr_dat(0x0003);		/* ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEB3); OTM8009A_wr_dat(0x0023);		/* ceb4[7:0] : clka3_switch[7:0] */
		OTM8009A_wr_cmd(0xCEB4); OTM8009A_wr_dat(0x0000);		/* ceb5[7:0] : clka3_extend[7:0] */
		OTM8009A_wr_cmd(0xCEB5); OTM8009A_wr_dat(0x0000);		/* ceb6[7:0] : clka3_tchop[7:0] */
		OTM8009A_wr_cmd(0xCEB6); OTM8009A_wr_dat(0x0000);		/* ceb7[7:0] : clka3_tglue[7:0] */
		OTM8009A_wr_cmd(0xCEB7); OTM8009A_wr_dat(0x0030);		/* ceb8[7:0] : clka4_width[3:0], clka2_shift[11:8] */
		OTM8009A_wr_cmd(0xCEB8); OTM8009A_wr_dat(0x0000);		/* ceb9[7:0] : clka4_shift[7:0] */
		OTM8009A_wr_cmd(0xCEB9); OTM8009A_wr_dat(0x0003);		/* ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEBA); OTM8009A_wr_dat(0x0024);		/* cebb[7:0] : clka4_switch[7:0] */
		OTM8009A_wr_cmd(0xCEBB); OTM8009A_wr_dat(0x0000);		/* cebc[7:0] : clka4_extend */
		OTM8009A_wr_cmd(0xCEBC); OTM8009A_wr_dat(0x0000);		/* cebd[7:0] : clka4_tchop */
		OTM8009A_wr_cmd(0xCEBD); OTM8009A_wr_dat(0x0000);		/* cebe[7:0] : clka4_tglue */

		/* CECx : clkb1, clkb2 */
		OTM8009A_wr_cmd(0xCEC0); OTM8009A_wr_dat(0x0030);		/* cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8] */
		OTM8009A_wr_cmd(0xCEC1); OTM8009A_wr_dat(0x0001);		/* cec2[7:0] : clkb1_shift[7:0] */
		OTM8009A_wr_cmd(0xCEC2); OTM8009A_wr_dat(0x0003);		/* cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEC3); OTM8009A_wr_dat(0x0025);		/* cec4[7:0] : clkb1_switch[7:0] */
		OTM8009A_wr_cmd(0xCEC4); OTM8009A_wr_dat(0x0000);		/* cec5[7:0] : clkb1_extend[7:0] */
		OTM8009A_wr_cmd(0xCEC5); OTM8009A_wr_dat(0x0000);		/* cec6[7:0] : clkb1_tchop[7:0] */
		OTM8009A_wr_cmd(0xCEC6); OTM8009A_wr_dat(0x0000);		/* cec7[7:0] : clkb1_tglue[7:0] */
		OTM8009A_wr_cmd(0xCEC7); OTM8009A_wr_dat(0x0030);		/* cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8] */
		OTM8009A_wr_cmd(0xCEC8); OTM8009A_wr_dat(0x0002);		/* cec9[7:0] : clkb2_shift[7:0] */
		OTM8009A_wr_cmd(0xCEC9); OTM8009A_wr_dat(0x0003);		/* ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCECA); OTM8009A_wr_dat(0x0026);		/* cecb[7:0] : clkb2_switch[7:0] */
		OTM8009A_wr_cmd(0xCECB); OTM8009A_wr_dat(0x0000);		/* cecc[7:0] : clkb2_extend */
		OTM8009A_wr_cmd(0xCECC); OTM8009A_wr_dat(0x0000);		/* cecd[7:0] : clkb2_tchop */
		OTM8009A_wr_cmd(0xCECD); OTM8009A_wr_dat(0x0000);		/* cece[7:0] : clkb2_tglue  */

		/* CEDx : clkb3, clkb4 */
		OTM8009A_wr_cmd(0xCED0); OTM8009A_wr_dat(0x0030);		/* ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8] */
		OTM8009A_wr_cmd(0xCED1); OTM8009A_wr_dat(0x0003);		/* ced2[7:0] : clkb3_shift[7:0] */
		OTM8009A_wr_cmd(0xCED2); OTM8009A_wr_dat(0x0003);		/* ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCED3); OTM8009A_wr_dat(0x0027);		/* ced4[7:0] : clkb3_switch[7:0] */
		OTM8009A_wr_cmd(0xCED4); OTM8009A_wr_dat(0x0000);		/* ced5[7:0] : clkb3_extend[7:0] */
		OTM8009A_wr_cmd(0xCED5); OTM8009A_wr_dat(0x0000);		/* ced6[7:0] : clkb3_tchop[7:0] */
		OTM8009A_wr_cmd(0xCED6); OTM8009A_wr_dat(0x0000);		/* ced7[7:0] : clkb3_tglue[7:0] */
		OTM8009A_wr_cmd(0xCED7); OTM8009A_wr_dat(0x0030);		/* ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8] */
		OTM8009A_wr_cmd(0xCED8); OTM8009A_wr_dat(0x0004);		/* ced9[7:0] : clkb4_shift[7:0] */
		OTM8009A_wr_cmd(0xCED9); OTM8009A_wr_dat(0x0003);		/* ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8] */
		OTM8009A_wr_cmd(0xCEDA); OTM8009A_wr_dat(0x0028);		/* cedb[7:0] : clkb4_switch[7:0] */
		OTM8009A_wr_cmd(0xCEDB); OTM8009A_wr_dat(0x0000);		/* cedc[7:0] : clkb4_extend */
		OTM8009A_wr_cmd(0xCEDC); OTM8009A_wr_dat(0x0000);		/* cedd[7:0] : clkb4_tchop */
		OTM8009A_wr_cmd(0xCEDD); OTM8009A_wr_dat(0x0000);		/* cede[7:0] : clkb4_tglue */

		/* CFCx : */        
		OTM8009A_wr_cmd(0xCFC0); OTM8009A_wr_dat(0x0000);		/* cfc1[7:0] : eclk_normal_width[7:0] */
		OTM8009A_wr_cmd(0xCFC1); OTM8009A_wr_dat(0x0000);		/* cfc2[7:0] : eclk_partial_width[7:0] */
		OTM8009A_wr_cmd(0xCFC2); OTM8009A_wr_dat(0x0000);		/* cfc3[7:0] : all_normal_tchop[7:0] */
		OTM8009A_wr_cmd(0xCFC3); OTM8009A_wr_dat(0x0000);		/* cfc4[7:0] : all_partial_tchop[7:0] */             
		OTM8009A_wr_cmd(0xCFC4); OTM8009A_wr_dat(0x0000);		/* cfc5[7:0] : eclk1_follow[3:0], eclk2_follow[3:0] */
		OTM8009A_wr_cmd(0xCFC5); OTM8009A_wr_dat(0x0000);		/* cfc6[7:0] : eclk3_follow[3:0], eclk4_follow[3:0] */
		OTM8009A_wr_cmd(0xCFC6); OTM8009A_wr_dat(0x0000);		/* cfc7[7:0] : 00, vstmask, vendmask, 00, dir1, dir2 (0=VGL, 1=VGH) */ 
		OTM8009A_wr_cmd(0xCFC7); OTM8009A_wr_dat(0x0000);		/* cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4 */
		OTM8009A_wr_cmd(0xCFC8); OTM8009A_wr_dat(0x0000);		/* cfc9[7:0] : duty_block[3:0], DGPM[3:0] */
		OTM8009A_wr_cmd(0xCFC9); OTM8009A_wr_dat(0x0000);		/* cfca[7:0] : reg_goa_gnd_period[7:0] */

		OTM8009A_wr_cmd(0xD800); OTM8009A_wr_dat(0x0097);
		OTM8009A_wr_cmd(0xD801); OTM8009A_wr_dat(0x0097);
		
		OTM8009A_wr_cmd(0xD900); OTM8009A_wr_dat(0x0039);
		OTM8009A_wr_cmd(0xD900); OTM8009A_wr_dat(0x0039);

		OTM8009A_wr_cmd(0xE100); OTM8009A_wr_dat(0x0004);
		OTM8009A_wr_cmd(0xE101); OTM8009A_wr_dat(0x000F);
		OTM8009A_wr_cmd(0xE102); OTM8009A_wr_dat(0x0017);
		OTM8009A_wr_cmd(0xE103); OTM8009A_wr_dat(0x002D);
		OTM8009A_wr_cmd(0xE104); OTM8009A_wr_dat(0x0043);
		OTM8009A_wr_cmd(0xE105); OTM8009A_wr_dat(0x005B);
		OTM8009A_wr_cmd(0xE106); OTM8009A_wr_dat(0x0064);
		OTM8009A_wr_cmd(0xE107); OTM8009A_wr_dat(0x0095);
		OTM8009A_wr_cmd(0xE108); OTM8009A_wr_dat(0x0083);
		OTM8009A_wr_cmd(0xE109); OTM8009A_wr_dat(0x0099);

		OTM8009A_wr_cmd(0xE10A); OTM8009A_wr_dat(0x006E);
		OTM8009A_wr_cmd(0xE10B); OTM8009A_wr_dat(0x005D);
		OTM8009A_wr_cmd(0xE10C); OTM8009A_wr_dat(0x0076);
		OTM8009A_wr_cmd(0xE10D); OTM8009A_wr_dat(0x0061);
		OTM8009A_wr_cmd(0xE10E); OTM8009A_wr_dat(0x0067);
		OTM8009A_wr_cmd(0xE10F); OTM8009A_wr_dat(0x0060);
		OTM8009A_wr_cmd(0xE110); OTM8009A_wr_dat(0x0059);
		OTM8009A_wr_cmd(0xE111); OTM8009A_wr_dat(0x0050);
		OTM8009A_wr_cmd(0xE112); OTM8009A_wr_dat(0x0045);
		OTM8009A_wr_cmd(0xE113); OTM8009A_wr_dat(0x0020);

		OTM8009A_wr_cmd(0xE200); OTM8009A_wr_dat(0x0004);
		OTM8009A_wr_cmd(0xE201); OTM8009A_wr_dat(0x000F);
		OTM8009A_wr_cmd(0xE202); OTM8009A_wr_dat(0x0017);
		OTM8009A_wr_cmd(0xE203); OTM8009A_wr_dat(0x002D);
		OTM8009A_wr_cmd(0xE204); OTM8009A_wr_dat(0x0043);
		OTM8009A_wr_cmd(0xE205); OTM8009A_wr_dat(0x005B);
		OTM8009A_wr_cmd(0xE206); OTM8009A_wr_dat(0x0064);
		OTM8009A_wr_cmd(0xE207); OTM8009A_wr_dat(0x0095);
		OTM8009A_wr_cmd(0xE208); OTM8009A_wr_dat(0x0083);
		OTM8009A_wr_cmd(0xE209); OTM8009A_wr_dat(0x0099);

		OTM8009A_wr_cmd(0xE20A); OTM8009A_wr_dat(0x006E);
		OTM8009A_wr_cmd(0xE20B); OTM8009A_wr_dat(0x005E);
		OTM8009A_wr_cmd(0xE20C); OTM8009A_wr_dat(0x0076);
		OTM8009A_wr_cmd(0xE20D); OTM8009A_wr_dat(0x0062);
		OTM8009A_wr_cmd(0xE20E); OTM8009A_wr_dat(0x0068);
		OTM8009A_wr_cmd(0xE20F); OTM8009A_wr_dat(0x0061);
		OTM8009A_wr_cmd(0xE210); OTM8009A_wr_dat(0x0059);
		OTM8009A_wr_cmd(0xE211); OTM8009A_wr_dat(0x0050);
		OTM8009A_wr_cmd(0xE212); OTM8009A_wr_dat(0x0045);
		OTM8009A_wr_cmd(0xE213); OTM8009A_wr_dat(0x0020);

		OTM8009A_wr_cmd(0x1C00); OTM8009A_wr_dat(0x0001);
		OTM8009A_wr_cmd(0x5900); OTM8009A_wr_dat(0x0001);

		OTM8009A_wr_cmd(0x3A00); OTM8009A_wr_dat(0x0055);
		OTM8009A_wr_cmd(0x3500); OTM8009A_wr_dat(0x0000);
		OTM8009A_wr_cmd(0x3600); OTM8009A_wr_dat(0x0000);		
		OTM8009A_wr_cmd(0x3800);

		OTM8009A_wr_cmd(0x1100);
		_delay_ms(150);

		OTM8009A_wr_cmd(0xFF00); OTM8009A_wr_dat(0x00FF);
		OTM8009A_wr_cmd(0xFF01); OTM8009A_wr_dat(0x00FF);
		OTM8009A_wr_cmd(0xFF02); OTM8009A_wr_dat(0x00FF);

		OTM8009A_wr_cmd(0x2900);
		_delay_ms(20);
	}

	else { for(;;);} /* Invalid Device Code!! */

	OTM8009A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	OTM8009A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		OTM8009A_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
