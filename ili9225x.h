/********************************************************************************/
/*!
	@file			ili9225x.h
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
#ifndef ILI9225X_H
#define ILI9225X_H 0x0500

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ILI9225x unique value */
/* mst be need for ILI9225x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				176
#define MAX_Y				220

/* Display Contol Macros */
#define ILI9225x_RES_SET()	DISPLAY_RES_SET()
#define ILI9225x_RES_CLR()	DISPLAY_RES_CLR()
#define ILI9225x_CS_SET()	DISPLAY_CS_SET()
#define ILI9225x_CS_CLR()	DISPLAY_CS_CLR()
#define ILI9225x_DC_SET()	DISPLAY_DC_SET()
#define ILI9225x_DC_CLR()	DISPLAY_DC_CLR()
#define ILI9225x_WR_SET()	DISPLAY_WR_SET()
#define ILI9225x_WR_CLR()	DISPLAY_WR_CLR()
#define ILI9225x_RD_SET()	DISPLAY_RD_SET()
#define ILI9225x_RD_CLR()	DISPLAY_RD_CLR()
#define ILI9225x_SCK_SET()	DISPLAY_SCK_SET()
#define ILI9225x_SCK_CLR()	DISPLAY_SCK_CLR()
#define ILI9225x_SDI_SET()	DISPLAY_SDI_SET()
#define ILI9225x_SDI_CLR()	DISPLAY_SDI_CLR()
#define ILI9225x_SDO_SET()	DISPLAY_SDO_SET()
#define ILI9225x_SDO_CLR()	DISPLAY_SDO_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9225x_WR()   	ILI9225x_WR_CLR(); \
							ILI9225x_WR_SET();
#else
 #define ILI9225x_WR()
#endif

#define	ILI9225x_DATA		DISPLAY_DATAPORT
#define ILI9225x_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI9225x_reset(void);
extern void ILI9225x_init(void);
extern void ILI9225x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9225x_wr_cmd(uint8_t cmd);
extern void ILI9225x_wr_dat(uint16_t dat);
extern void ILI9225x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9225x_clear(void);
extern uint16_t ILI9225x_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			ILI9225x_init
#define Display_rect_if 		ILI9225x_rect
#define Display_wr_dat_if		ILI9225x_wr_dat
#define Display_wr_cmd_if		ILI9225x_wr_cmd
#define Display_wr_block_if		ILI9225x_wr_block
#define Display_clear_if 		ILI9225x_clear

#ifdef __cplusplus
}
#endif

#endif /* ILI9225X_H */
