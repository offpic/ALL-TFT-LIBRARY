/********************************************************************************/
/*!
	@file			ili9806h.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TK040FH001		(ILI9806H)	8/16bit mode.

    @section HISTORY
		2015.10.20	V1.00	First Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ILI9806H_H
#define ILI9806H_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ILI9806H unique value */
/* mst be need for ILI9806H */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				800

/* Display Contol Macros */
#define ILI9806H_RES_SET()	DISPLAY_RES_SET()
#define ILI9806H_RES_CLR()	DISPLAY_RES_CLR()
#define ILI9806H_CS_SET()	DISPLAY_CS_SET()
#define ILI9806H_CS_CLR()	DISPLAY_CS_CLR()
#define ILI9806H_DC_SET()	DISPLAY_DC_SET()
#define ILI9806H_DC_CLR()	DISPLAY_DC_CLR()
#define ILI9806H_WR_SET()	DISPLAY_WR_SET()
#define ILI9806H_WR_CLR()	DISPLAY_WR_CLR()
#define ILI9806H_RD_SET()	DISPLAY_RD_SET()
#define ILI9806H_RD_CLR()	DISPLAY_RD_CLR()
#define ILI9806H_SCK_SET()	DISPLAY_SCK_SET()
#define ILI9806H_SCK_CLR()	DISPLAY_SCK_CLR()
#define ILI9806H_SDI_SET()	DISPLAY_SDI_SET()
#define ILI9806H_SDI_CLR()	DISPLAY_SDI_CLR()
#define ILI9806H_SDO_SET()	DISPLAY_SDO_SET()
#define ILI9806H_SDO_CLR()	DISPLAY_SDO_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9806H_WR()   	ILI9806H_WR_CLR(); \
							ILI9806H_WR_SET();
#else
 #define ILI9806H_WR()
#endif

#define	ILI9806H_DATA		DISPLAY_DATAPORT
#define ILI9806H_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI9806H_reset(void);
extern void ILI9806H_init(void);
extern void ILI9806H_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9806H_wr_cmd(uint8_t cmd);
extern void ILI9806H_wr_dat(uint8_t dat);
extern void ILI9806H_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9806H_clear(void);
extern uint16_t ILI9806H_rd_cmd(uint8_t cmd);
extern void ILI9806H_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			ILI9806H_init
#define Display_rect_if 		ILI9806H_rect
#define Display_wr_dat_if		ILI9806H_wr_gram
#define Display_wr_cmd_if		ILI9806H_wr_cmd
#define Display_wr_block_if		ILI9806H_wr_block
#define Display_clear_if 		ILI9806H_clear

#ifdef __cplusplus
}
#endif

#endif /* ILI9806H_H */
