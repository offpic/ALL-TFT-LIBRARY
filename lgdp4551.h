/********************************************************************************/
/*!
	@file			lgdp4551.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive WS-TFT30L05T-3B TFT module(8bit mode).

    @section HISTORY
		2014.08.03	V1.00	Stable Release
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef LGDP4551_H
#define LGDP4551_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* LGDP4551 unique value */
/* mst be need for LGDP4551 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				400

/* Display Contol Macros */
#define LGDP4551_RES_SET()	DISPLAY_RES_SET()
#define LGDP4551_RES_CLR()	DISPLAY_RES_CLR()
#define LGDP4551_CS_SET()	DISPLAY_CS_SET()
#define LGDP4551_CS_CLR()	DISPLAY_CS_CLR()
#define LGDP4551_DC_SET()	DISPLAY_DC_SET()
#define LGDP4551_DC_CLR()	DISPLAY_DC_CLR()
#define LGDP4551_WR_SET()	DISPLAY_WR_SET()
#define LGDP4551_WR_CLR()	DISPLAY_WR_CLR()
#define LGDP4551_RD_SET()	DISPLAY_RD_SET()
#define LGDP4551_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define LGDP4551_WR()   	LGDP4551_WR_CLR(); \
							LGDP4551_WR_SET();
#else
 #define LGDP4551_WR()
#endif

#define	LGDP4551_DATA		DISPLAY_DATAPORT
#define LGDP4551_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void LGDP4551_reset(void);
extern void LGDP4551_init(void);
extern void LGDP4551_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void LGDP4551_wr_cmd(uint8_t cmd);
extern void LGDP4551_wr_dat(uint16_t dat);
extern void LGDP4551_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void LGDP4551_clear(void);
extern uint16_t LGDP4551_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			LGDP4551_init
#define Display_rect_if 		LGDP4551_rect
#define Display_wr_dat_if		LGDP4551_wr_dat
#define Display_wr_cmd_if		LGDP4551_wr_cmd
#define Display_wr_block_if		LGDP4551_wr_block
#define Display_clear_if 		LGDP4551_clear

#ifdef __cplusplus
}
#endif

#endif /* LGDP4551_H */
