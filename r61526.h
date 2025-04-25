/********************************************************************************/
/*!
	@file			r61526.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KD024B-3-TP 				(R61526)	8/16bit mode.		@n
					 -Nokia6300-LCD				(MC2PA8201)	8bit mode only.

    @section HISTORY
		2012.04.20	V1.00	First Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef R61526_H
#define R61526_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* R61526 unique value */
/* mst be need for R61526 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define R61526_RES_SET()	DISPLAY_RES_SET()
#define R61526_RES_CLR()	DISPLAY_RES_CLR()
#define R61526_CS_SET()		DISPLAY_CS_SET()
#define R61526_CS_CLR()		DISPLAY_CS_CLR()
#define R61526_DC_SET()		DISPLAY_DC_SET()
#define R61526_DC_CLR()		DISPLAY_DC_CLR()
#define R61526_WR_SET()		DISPLAY_WR_SET()
#define R61526_WR_CLR()		DISPLAY_WR_CLR()
#define R61526_RD_SET()		DISPLAY_RD_SET()
#define R61526_RD_CLR()		DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define R61526_WR()   		R61526_WR_CLR(); \
							R61526_WR_SET();
#else
 #define R61526_WR()
#endif

#define	R61526_DATA			DISPLAY_DATAPORT
#define R61526_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void R61526_reset(void);
extern void R61526_init(void);
extern void R61526_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void R61526_wr_cmd(uint8_t cmd);
extern void R61526_wr_dat(uint8_t dat);
extern void R61526_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void R61526_clear(void);
extern uint16_t R61526_rd_cmd(uint8_t cmd);
extern void R61526_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			R61526_init
#define Display_rect_if 		R61526_rect
#define Display_wr_dat_if		R61526_wr_gram
#define Display_wr_cmd_if		R61526_wr_cmd
#define Display_wr_block_if		R61526_wr_block
#define Display_clear_if 		R61526_clear

#ifdef __cplusplus
}
#endif

#endif /* R61526_H */
