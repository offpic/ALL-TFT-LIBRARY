/********************************************************************************/
/*!
	@file			bd663474.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive MZTX06A TFT module(spi mode only).

    @section HISTORY
		2013.08.20	V1.00	Stable Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef BD663474_H
#define BD663474_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* BD663474 unique value */
/* mst be need for BD663474 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define BD663474_RES_SET()	DISPLAY_RES_SET()
#define BD663474_RES_CLR()	DISPLAY_RES_CLR()
#define BD663474_CS_SET()	DISPLAY_CS_SET()
#define BD663474_CS_CLR()	DISPLAY_CS_CLR()
#define BD663474_DC_SET()	DISPLAY_DC_SET()
#define BD663474_DC_CLR()	DISPLAY_DC_CLR()
#define BD663474_WR_SET()	DISPLAY_WR_SET()
#define BD663474_WR_CLR()	DISPLAY_WR_CLR()
#define BD663474_RD_SET()	DISPLAY_RD_SET()
#define BD663474_RD_CLR()	DISPLAY_RD_CLR()
#define BD663474_SCK_SET()	DISPLAY_SCK_SET()
#define BD663474_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define BD663474_WR()   	BD663474_WR_CLR(); \
							BD663474_WR_SET();
#else
 #define BD663474_WR()
#endif

#define	BD663474_DATA		DISPLAY_DATAPORT
#define BD663474_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void BD663474_reset(void);
extern void BD663474_init(void);
extern void BD663474_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void BD663474_wr_cmd(uint16_t cmd);
extern void BD663474_wr_dat(uint16_t dat);
extern void BD663474_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void BD663474_clear(void);
extern uint16_t BD663474_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			BD663474_init
#define Display_rect_if 		BD663474_rect
#define Display_wr_dat_if		BD663474_wr_dat
#define Display_wr_cmd_if		BD663474_wr_cmd
#define Display_wr_block_if		BD663474_wr_block
#define Display_clear_if 		BD663474_clear

#ifdef __cplusplus
}
#endif

#endif /* BD663474_H */
