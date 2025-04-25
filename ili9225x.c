/********************************************************************************/
/*!
	@file			ili9225x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TXDT200A-15V13		(ILI9225)	  	8bit mode. 			@n
					 -KXM220HS-V05			(ILI9225G)		8/16bit mode.		@n
					 -RX020C-1				(S6D0164X1)		8bit mode.			@n
					 -BTL221722-276LP		(ILI9225B)		8bit Serial only.	@n
					 -FPC20061A-V0			(RM68130)		8bit Serial only.	@n
					 -H20TM114A-V0			(GC9201/ST7775R)8bit Serial only.

    @section HISTORY
		2012.08.15	V1.00	Revised from ili9225.c
		2015.05.15	V2.00	Added FPC20061A-V0 support.
		2016.11.03	V3.00	Added H20TM114A-V0(GC9201/ST7775R) support.
		2023.05.01	V4.00	Removed unused delay function.
		2023.08.01	V5.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9225x.h"
/* check header file version for fool proof */
#if ILI9225X_H != 0x0500
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#ifdef  USE_ILI9225x_SPI_TFT
#warning If U do use RM68130/GC9201/ST7775 device,please commentize below.
//#undef TFT_SDA_READ


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
inline void ILI9225x_reset(void)
{
#ifdef  USE_ILI9225x_TFT
	ILI9225x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9225x_RD_SET();
	ILI9225x_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9225x_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9225x_CS_CLR();

#elif  USE_ILI9225x_SPI_TFT
	ILI9225x_RES_SET();							/* RES=H, CS=H				*/
	ILI9225x_CS_SET();
	ILI9225x_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9225x_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(30);								/* wait 30ms     			*/
	ILI9225x_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ILI9225x_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9225x_wr_cmd(uint8_t cmd)
{
	ILI9225x_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9225x_CMD = 0;
	ILI9225x_WR();
#endif

	ILI9225x_CMD = cmd;							/* cmd(8bit)				*/
	ILI9225x_WR();								/* WR=L->H					*/

	ILI9225x_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI9225x_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9225x_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	ILI9225x_WR();								/* WR=L->H					*/
	ILI9225x_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	ILI9225x_DATA = dat;						/* 16bit data 				*/
#endif
	ILI9225x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9225x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9225x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9225x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9225x_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	ILI9225x_wr_cmd(cmd);
	ILI9225x_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif

    ReadLCDData(val);

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	val &= 0x00FF;
	val |= temp<<8;
#endif

	return val;
}


#elif USE_ILI9225x_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9225x_wr_cmd(uint8_t cmd)
{
	ILI9225x_DC_CLR();							/* DC=L			 */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ILI9225x_DC_SET();							/* DC=H			 */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ILI9225x_wr_dat(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9225x_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */

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
inline uint16_t ILI9225x_rd_cmd(uint8_t cmd)
{
 #warning "RM68130/GC9201 USES ONLY SDA(Input&Output Multiplexed) Line!"
 #warning "RM68130/GC9201 4-WireMode  CANNOT Use SDO!"
	uint16_t val;
	uint8_t temp;

	ILI9225x_wr_cmd(0x66);						/* Register Read Mode */
	ILI9225x_wr_dat(0x01);

	DISPLAY_ASSART_CS();						/* CS=L		    */
	ILI9225x_DC_CLR();							/* DC=L			*/

	SendSPI(cmd);

	Display_ChangeSDA_If(TFT_SDA_READ);
	ILI9225x_DC_SET();							/* DC=H			*/
	temp = RecvSPI();							/* Dummy Read 	*/
	temp = RecvSPI();							/* Dummy Read 	*/
	temp = RecvSPI();							/* Upper Read 	*/
	val  = RecvSPI();							/* Lower Read	*/
	Display_ChangeSDA_If(TFT_SDA_WRITE);

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;

	DISPLAY_NEGATE_CS();						/* CS=H		    */

	ILI9225x_wr_cmd(0x66);						/* Register Write Mode */
	ILI9225x_wr_dat(0x00);

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI9225x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9225x_wr_cmd(0x37);				/* Horizontal RAM Start ADDR */
	ILI9225x_wr_dat(OFS_COL + x);
	ILI9225x_wr_cmd(0x36);				/* Horizontal RAM End ADDR */
	ILI9225x_wr_dat(OFS_COL + width);
	ILI9225x_wr_cmd(0x39);				/* Vertical RAM Start ADDR */
	ILI9225x_wr_dat(OFS_RAW + y);
	ILI9225x_wr_cmd(0x38);				/* Vertical End ADDR */
	ILI9225x_wr_dat(OFS_RAW + height);

	ILI9225x_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	ILI9225x_wr_dat(OFS_RAW + y);
	ILI9225x_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	ILI9225x_wr_dat(OFS_COL + x);

	ILI9225x_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9225x_clear(void)
{
	volatile uint32_t n;

	ILI9225x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9225x_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9225x_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	ILI9225x_reset();

	/* Check Device Code */
	devicetype = ILI9225x_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x9225)
	{
		/* Initialize ILI9225 & ILI9225B & ILI9225C */
		ILI9225x_wr_cmd(0x01);		/* set SS and NL bit */
		ILI9225x_wr_dat(0x011C);
		ILI9225x_wr_cmd(0x02);		/* set 1 line inversion */
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x03);		/* Set GRAM write direction and BGR=1 */ 
		ILI9225x_wr_dat((1<<12)|(0<<9)|(1<<8)|(1<<5)|(1<<4)|(0<<3));
		/* ILI9225x_wr_dat(0x1030); */ /* original */
		ILI9225x_wr_cmd(0x08);		/* set BP and FP */
		ILI9225x_wr_dat(0x0808);
		ILI9225x_wr_cmd(0x0B);		/* frame cycle */
		ILI9225x_wr_dat(0x1100);
		ILI9225x_wr_cmd(0x0C);		/* RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit */
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x0F);		/* Set frame rate----0801 */
		ILI9225x_wr_dat(0x1401);
		ILI9225x_wr_cmd(0x15);		/* set system interface */
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x20);	 	/* Set GRAM Address */
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x21);	 	/* Set GRAM Address */
		ILI9225x_wr_dat(0x0000);
		
		/* Power Up sequence */
		_delay_ms(50);
		ILI9225x_wr_cmd(0x10);		/* Set SAP,DSTB,STB----0A00 */
		ILI9225x_wr_dat(0x0800);
		ILI9225x_wr_cmd(0x11);		/* Set APON,PON,AON,VCI1EN,VC----1038 */
		ILI9225x_wr_dat(0x1F3F);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x12);	 	/* Internal reference voltage= Vci;----1121 */
		ILI9225x_wr_dat(0x0121);
		ILI9225x_wr_cmd(0x13);	 	/* Set GVDD----0066 */
		ILI9225x_wr_dat(0x006F);
		ILI9225x_wr_cmd(0x14);	 	/* Set VCOMH/VCOML voltage----5F60 */
		ILI9225x_wr_dat(0x4349);
		
		/* Set GRAM area */
		ILI9225x_wr_cmd(0x30);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x31);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x32);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x33);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x34);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x35);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x36);
		ILI9225x_wr_dat(0x00AF);
		ILI9225x_wr_cmd(0x37);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x38);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x39);
		ILI9225x_wr_dat(0x0000);
		
		/* Adjust the Gamma Curve */
		ILI9225x_wr_cmd(0x50);
		ILI9225x_wr_dat(0x0001);  /* 0400 */
		ILI9225x_wr_cmd(0x51);
		ILI9225x_wr_dat(0x200B);  /* 060B */
		ILI9225x_wr_cmd(0x52);
		ILI9225x_wr_dat(0x0000);  /* 0C0A */
		ILI9225x_wr_cmd(0x53);
		ILI9225x_wr_dat(0x0404);  /* 0105 */
		ILI9225x_wr_cmd(0x54);
		ILI9225x_wr_dat(0x0C0C);  /* 0A0C */
		ILI9225x_wr_cmd(0x55);
		ILI9225x_wr_dat(0x000C);  /* 0B06 */
		ILI9225x_wr_cmd(0x56);
		ILI9225x_wr_dat(0x0101);  /* 0004 */
		ILI9225x_wr_cmd(0x57);
		ILI9225x_wr_dat(0x0400);  /* 0501 */
		ILI9225x_wr_cmd(0x58);
		ILI9225x_wr_dat(0x1108);  /* 0E00 */
		ILI9225x_wr_cmd(0x59);
		ILI9225x_wr_dat(0x050C);  /* 000E */
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x07);						   /* Display On */ 
		ILI9225x_wr_dat((1<<12)|(1<<4)|(1<<2)|(1<<1)|(1<<0));
	}

	else if(devicetype == 0x9226)
	{
		/* Initialize ILI9225G */
		/* Start Initial Sequence */
		ILI9225x_wr_cmd(0x01);
		ILI9225x_wr_dat(0x011C);		/* set SS and NL bit */
		ILI9225x_wr_cmd(0x02);
		ILI9225x_wr_dat(0x0100);		/* set 1 line inversion */
		ILI9225x_wr_cmd(0x03);
		ILI9225x_wr_dat(0x1030);		/* set GRAM write direction and BGR=1. */
		ILI9225x_wr_cmd(0x08);
		ILI9225x_wr_dat(0x0808);		/* set BP and FP */
		ILI9225x_wr_cmd(0x0C);
		ILI9225x_wr_dat(0x0000);		/* RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111forRGB16Bit */
		ILI9225x_wr_cmd(0x0F);
		ILI9225x_wr_dat(0x0801);		/* Set frame rate */
		ILI9225x_wr_cmd(0x20);
		ILI9225x_wr_dat(0x0000);		/* Set GRAM Address */
		ILI9225x_wr_cmd(0x21);
		ILI9225x_wr_dat(0x0000);		/* Set GRAM Address */
		
		/* Power On sequence */
		_delay_ms(50);		 			/* Delay 50ms */
		ILI9225x_wr_cmd(0x10);
		ILI9225x_wr_dat(0x0A00);		/* Set SAP,DSTB,STB */
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x103B);		/* Set APON,PON,AON,VCI1EN,VC */
		_delay_ms(50);		 			/* Delay 50ms */
		ILI9225x_wr_cmd(0x12);
		ILI9225x_wr_dat(0x3121);		/* Internal reference voltage= Vci; */
		ILI9225x_wr_cmd(0x13);
		ILI9225x_wr_dat(0x0066);		/* Set GVDD */
		ILI9225x_wr_cmd(0x14);
		ILI9225x_wr_dat(0x4050);		/* Set VCOMH/VCOML voltage */
		
		/* Set GRAM area */
		ILI9225x_wr_cmd(0x30);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x31);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x32);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x33);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x34);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x35);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x36);
		ILI9225x_wr_dat(0x00AF);
		ILI9225x_wr_cmd(0x37);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x38);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x39);
		ILI9225x_wr_dat(0x0000);
		
		/* Adjust the Gamma Curve */
		ILI9225x_wr_cmd(0x50);
		ILI9225x_wr_dat(0x0400);
		ILI9225x_wr_cmd(0x51);
		ILI9225x_wr_dat(0x080B);
		ILI9225x_wr_cmd(0x52);
		ILI9225x_wr_dat(0x0E0C);
		ILI9225x_wr_cmd(0x53);
		ILI9225x_wr_dat(0x0103);
		ILI9225x_wr_cmd(0x54);
		ILI9225x_wr_dat(0x0C0E);
		ILI9225x_wr_cmd(0x55);
		ILI9225x_wr_dat(0x0B08);
		ILI9225x_wr_cmd(0x56);
		ILI9225x_wr_dat(0x0004);
		ILI9225x_wr_cmd(0x57);
		ILI9225x_wr_dat(0x0301);
		ILI9225x_wr_cmd(0x58);
		ILI9225x_wr_dat(0x0800);
		ILI9225x_wr_cmd(0x59);
		ILI9225x_wr_dat(0x0008);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x1017);
	}

	else if((devicetype == 0x0164) || (devicetype == 0x2201))
	{
		/* Initialize OTM2201 & S6D0164 */
		ILI9225x_wr_cmd(0xF0);
		ILI9225x_wr_dat(0x2201);
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0xF7);
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x15);
		ILI9225x_wr_dat(0x0010);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0012);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x001A);
		ILI9225x_wr_cmd(0x01);
		ILI9225x_wr_dat(0x011C);
		ILI9225x_wr_cmd(0x12);
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x03);
		ILI9225x_wr_dat(0x1030);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x08);
		ILI9225x_wr_dat(0x0808);
		ILI9225x_wr_cmd(0x0B);
		ILI9225x_wr_dat(0x1100);
		ILI9225x_wr_cmd(0x0C);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x0F);
		ILI9225x_wr_dat(0x0501);
		ILI9225x_wr_cmd(0x15);
		ILI9225x_wr_dat(0x0020);
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0018);
		ILI9225x_wr_cmd(0x12);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x13);
		ILI9225x_wr_dat(0x0063);
		ILI9225x_wr_cmd(0x14);
		ILI9225x_wr_dat(0x556A);
		ILI9225x_wr_cmd(0x10);
		ILI9225x_wr_dat(0x0800);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0118);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0318);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0718);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0F18);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0F38);
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x36);           
		ILI9225x_wr_dat(0x00AF);
		ILI9225x_wr_cmd(0x37);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x38);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x39);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x50);
		ILI9225x_wr_dat(0x0001);
		ILI9225x_wr_cmd(0x51);
		ILI9225x_wr_dat(0x0208);
		ILI9225x_wr_cmd(0x52);
		ILI9225x_wr_dat(0x0805);
		ILI9225x_wr_cmd(0x53);
		ILI9225x_wr_dat(0x0404);
		ILI9225x_wr_cmd(0x54);
		ILI9225x_wr_dat(0x0C0C);
		ILI9225x_wr_cmd(0x55);
		ILI9225x_wr_dat(0x000C);
		ILI9225x_wr_cmd(0x56);
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x57);
		ILI9225x_wr_dat(0x0400);
		ILI9225x_wr_cmd(0x58);
		ILI9225x_wr_dat(0x1108);
		ILI9225x_wr_cmd(0x59);
		ILI9225x_wr_dat(0x050C);
		ILI9225x_wr_cmd(0x0F);
		ILI9225x_wr_dat(0x0F01);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0012);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0017);
	}

	else if(devicetype == 0x6813)
	{
		/* Initialize RM68130 */
		ILI9225x_wr_cmd(0x28);		 /* Software Reset */
		ILI9225x_wr_dat(0x00CE);
		ILI9225x_wr_cmd(0x01);		 /* driver output control */
		ILI9225x_wr_dat(0x011C);
		ILI9225x_wr_cmd(0x03);		 /* Entry mode */
		ILI9225x_wr_dat(0x1030);
		ILI9225x_wr_cmd(0x07);		 /* Display control */
		ILI9225x_wr_dat(0x0007);
		ILI9225x_wr_cmd(0x11);		 /* Power control */
		ILI9225x_wr_dat(0x1000);
		ILI9225x_wr_cmd(0x02);		 /* LCD driving wave control 0 : Column Inversion */
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0xB0);		 /* Power control(0c12) */
		ILI9225x_wr_dat(0x1412);
		ILI9225x_wr_cmd(0x0B);		 /* Frame Rate Control 4-bit */
		ILI9225x_wr_dat(0x0000);
		
		/************* Start Gamma Setting **********/
		ILI9225x_wr_cmd(0xE8);		 /* Gamma Command 1 : Gamma Enable */
		ILI9225x_wr_dat(0x0110);
		ILI9225x_wr_cmd(0xB1);		 /* +- Gamma Voltage Setting */
		ILI9225x_wr_dat(0x0F0F);
		ILI9225x_wr_cmd(0x50);		 /* Below : Gamma Setting */
		ILI9225x_wr_dat(0x0003);
		ILI9225x_wr_cmd(0x51);
		ILI9225x_wr_dat(0x0807);
		ILI9225x_wr_cmd(0x52);
		ILI9225x_wr_dat(0x0C08);
		ILI9225x_wr_cmd(0x53);
		ILI9225x_wr_dat(0x0503);
		ILI9225x_wr_cmd(0x54);
		ILI9225x_wr_dat(0x0003);
		ILI9225x_wr_cmd(0x55);
		ILI9225x_wr_dat(0x0807);
		ILI9225x_wr_cmd(0x56);
		ILI9225x_wr_dat(0x0003);
		ILI9225x_wr_cmd(0x57);
		ILI9225x_wr_dat(0x0503);
		ILI9225x_wr_cmd(0x58);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x59);
		ILI9225x_wr_dat(0x0000);
		
		ILI9225x_wr_cmd(0xE8);
		ILI9225x_wr_dat(0x0102);
		ILI9225x_wr_cmd(0xFB);
		ILI9225x_wr_dat(0x002A);
		ILI9225x_wr_cmd(0xE8);
		ILI9225x_wr_dat(0x0101);
		ILI9225x_wr_cmd(0xF1);
		ILI9225x_wr_dat(0x0040);
	}

	else if((devicetype == 0x9201) || (devicetype == 0x7775))
	{
		/* Initialize GC9201/ST7775R */
		ILI9225x_wr_cmd(0x01);
		ILI9225x_wr_dat(0x011C);
		ILI9225x_wr_cmd(0x02);
		ILI9225x_wr_dat(0x0010);
		ILI9225x_wr_cmd(0x03);
		ILI9225x_wr_dat(0x1030);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x1017);
		ILI9225x_wr_cmd(0x08);
		ILI9225x_wr_dat(0x0808);
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x103B);
		
		/***** Start Initial Sequence *****/
		ILI9225x_wr_cmd(0x13);
		ILI9225x_wr_dat(0x0003);
		ILI9225x_wr_cmd(0x14);
		ILI9225x_wr_dat(0x637C);
		ILI9225x_wr_cmd(0xfe);
		ILI9225x_wr_dat(0x0075);
		ILI9225x_wr_cmd(0xff);
		ILI9225x_wr_dat(0x00a5);
		ILI9225x_wr_cmd(0xF0);
		ILI9225x_wr_dat(0x0741);
		ILI9225x_wr_cmd(0xF1);
		ILI9225x_wr_dat(0x0010);
		ILI9225x_wr_cmd(0xF5);
		ILI9225x_wr_dat(0x0010);
		ILI9225x_wr_cmd(0xF8);
		ILI9225x_wr_dat(0x0021);
		ILI9225x_wr_cmd(0xFC);
		ILI9225x_wr_dat(0x0311);
		ILI9225x_wr_cmd(0x0C);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x0F);
		ILI9225x_wr_dat(0x0b01);
		ILI9225x_wr_cmd(0x20);
		ILI9225x_wr_dat(0x0100);
		
		/***** Power Onsequence *****/
		ILI9225x_wr_cmd(0x10);
		ILI9225x_wr_dat(0x0A00);
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x103B);
		ILI9225x_wr_cmd(0x12);
		ILI9225x_wr_dat(0x6121);
		ILI9225x_wr_cmd(0x30);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x31);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x32);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x33);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x34);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x35);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x36);
		ILI9225x_wr_dat(0x00AF);
		ILI9225x_wr_cmd(0x37);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x38);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x39);
		ILI9225x_wr_dat(0x0000);
		
		/***** Display ON *****/
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x1017);
		ILI9225x_wr_cmd(0x22);
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9225x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9225x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9225x_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
