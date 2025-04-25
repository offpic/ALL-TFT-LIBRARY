/********************************************************************************/
/*!
	@file			tl1763.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -FPC-SH9705				(TL1763)	16bit mode only.

    @section HISTORY
		2014.08.01	V1.00	Stable Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef TL1763_H
#define TL1763_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* TL1763 unique value */
/* mst be need for TL1763 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define TL1763_RES_SET()	DISPLAY_RES_SET()
#define TL1763_RES_CLR()	DISPLAY_RES_CLR()
#define TL1763_CS_SET()		DISPLAY_CS_SET()
#define TL1763_CS_CLR()		DISPLAY_CS_CLR()
#define TL1763_DC_SET()		DISPLAY_DC_SET()
#define TL1763_DC_CLR()		DISPLAY_DC_CLR()
#define TL1763_WR_SET()		DISPLAY_WR_SET()
#define TL1763_WR_CLR()		DISPLAY_WR_CLR()
#define TL1763_RD_SET()		DISPLAY_RD_SET()
#define TL1763_RD_CLR()		DISPLAY_RD_CLR()
#define TL1763_SCK_SET()	DISPLAY_SCK_SET()
#define TL1763_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define TL1763_WR()   		TL1763_WR_CLR(); \
							TL1763_WR_SET();
#else
 #define TL1763_WR()
#endif

#define	TL1763_DATA			DISPLAY_DATAPORT
#define TL1763_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void TL1763_reset(void);
extern void TL1763_init(void);
extern void TL1763_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void TL1763_wr_cmd(uint8_t cmd);
extern void TL1763_wr_dat(uint16_t dat);
extern void TL1763_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void TL1763_clear(void);
extern uint16_t TL1763_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			TL1763_init
#define Display_rect_if 		TL1763_rect
#define Display_wr_dat_if		TL1763_wr_dat
#define Display_wr_cmd_if		TL1763_wr_cmd
#define Display_wr_block_if		TL1763_wr_block
#define Display_clear_if 		TL1763_clear

#ifdef __cplusplus
}
#endif

#endif /* TL1763_H */
