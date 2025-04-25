/********************************************************************************/
/*!
	@file			st7789v2.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2024.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -ATM0130B3				(ST7789V2)	(8bit4wire serial only)

    @section HISTORY
		2023.07.01	V1.00	Stable release.
		2023.08.01	V2.00	Revised release.
		2023.09.01	V3.00	Fixed DDRAM address set.
		2024.08.01	V4.00	Fixed unused parameter fix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ST7789V2_H
#define ST7789V2_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ST7789V2 unique value */
/* Select TFT-LCD module model */
#define USE_ATM0130B3

#if defined(USE_ATM0130B3)
 #define OFS_COL				0
 #define OFS_RAW				0
 #define MAX_X					240
 #define MAX_Y					240
#else
 #error "Select Your ST7789V2 module type!"
#endif

/* Display Contol Macros */
#define ST7789V2_RES_SET()		DISPLAY_RES_SET()
#define ST7789V2_RES_CLR()		DISPLAY_RES_CLR()
#define ST7789V2_CS_SET()		DISPLAY_CS_SET()
#define ST7789V2_CS_CLR()		DISPLAY_CS_CLR()
#define ST7789V2_DC_SET()		DISPLAY_DC_SET()
#define ST7789V2_DC_CLR()		DISPLAY_DC_CLR()
#define ST7789V2_WR_SET()		DISPLAY_WR_SET()
#define ST7789V2_WR_CLR()		DISPLAY_WR_CLR()
#define ST7789V2_RD_SET()		DISPLAY_RD_SET()
#define ST7789V2_RD_CLR()		DISPLAY_RD_CLR()
#define ST7789V2_SDATA_SET()	DISPLAY_SDI_SET()
#define ST7789V2_SDATA_CLR()	DISPLAY_SDI_CLR()
#define ST7789V2_SCLK_SET()		DISPLAY_SCK_SET()
#define ST7789V2_SCLK_CLR()		DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ST7789V2_WR()			ST7789V2_WR_CLR(); \
								ST7789V2_WR_SET();
#else
 #define ST7789V2_WR()
#endif

#define	ST7789V2_DATA			DISPLAY_DATAPORT
#define ST7789V2_CMD			DISPLAY_CMDPORT


/* ST7789V2 Instruction */
#define NO_OP		(0x00)
#define SWRESET		(0x01)
#define RDDID		(0x04)
#define RDDST		(0x09)
#define RDDPM		(0x0A)
#define RDDMADCTL	(0x0B)
#define RDDCOLMOD	(0x0C)
#define RDDIM		(0x0D)
#define RDDSM		(0x0E)
#define SLPIN		(0x10)
#define SLPOUT		(0x11)
#define PTLON		(0x12)
#define NORON		(0x13)
#define INVOFF		(0x20)
#define INVON		(0x21)
#define GAMSET		(0x26)
#define DISPOFF		(0x28)
#define DISPON		(0x29)
#define CASET		(0x2A)
#define RASET		(0x2B)
#define RAMWR		(0x2C)
#define RAMRD		(0x2E)
#define PTLAR		(0x30)
#define TEOFF		(0x34)
#define TEON		(0x35)
#define MADCTL		(0x36)
#define IDMOFF		(0x38)
#define IDMON		(0x39)
#define COLMOD		(0x3A)
#define RDID1		(0xDA)
#define RDID2		(0xDB)
#define RDID3		(0xDC)

/* ST7789V2 Panel Function Command List */
#define RAMCTRL		(0xB0)
#define FRMCTR1		(0xB3)
#define GATECTRL	(0xB7)
#define VCOMS		(0xB7)
#define VDVVRHEN	(0xC2)
#define VRHS		(0xC3)
#define VDVSET		(0xC4)
#define FRCTR2		(0xC5)
#define PWCTRL1		(0xD0)
#define PVGAMCTRL	(0xE0)
#define NVGAMCTRL	(0xE1)


/* If U want to true device id,uncomment this */
#define ST7789V2_SPI_4WIRE_READID_IGNORE

/* Display Control Functions Prototype */
extern void ST7789V2_reset(void);
extern void ST7789V2_init(void);
extern void ST7789V2_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ST7789V2_wr_cmd(uint8_t cmd);
extern void ST7789V2_wr_dat(uint8_t dat);
extern void ST7789V2_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ST7789V2_clear(void);
extern uint8_t ST7789V2_rd_cmd(uint8_t cmd);
extern void ST7789V2_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			ST7789V2_init
#define Display_rect_if 		ST7789V2_rect
#define Display_wr_dat_if		ST7789V2_wr_gram
#define Display_wr_cmd_if		ST7789V2_wr_cmd
#define Display_wr_block_if		ST7789V2_wr_block
#define Display_clear_if 		ST7789V2_clear

#ifdef __cplusplus
}
#endif

#endif /* ST7789V2_H */
