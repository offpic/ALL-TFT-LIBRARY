/********************************************************************************/
/*!
	@file			seps525.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG-6028GDEAF01 OLED module(8bit,spi mode).

    @section HISTORY
		2011.04.01	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef SEPS525_H
#define SEPS525_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SEPS525 Unique Value		*/
/* MUST be need for SEPS525	*/
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				160
#define MAX_Y				128

/* Display Contol Macros */
#define SEPS525_RES_SET()	DISPLAY_RES_SET()
#define SEPS525_RES_CLR()	DISPLAY_RES_CLR()
#define SEPS525_CS_SET()	DISPLAY_CS_SET()
#define SEPS525_CS_CLR()	DISPLAY_CS_CLR()
#define SEPS525_DC_SET()	DISPLAY_DC_SET()
#define SEPS525_DC_CLR()	DISPLAY_DC_CLR()
#define SEPS525_WR_SET()	DISPLAY_WR_SET()
#define SEPS525_WR_CLR()	DISPLAY_WR_CLR()
#define SEPS525_RD_SET()	DISPLAY_RD_SET()
#define SEPS525_RD_CLR()	DISPLAY_RD_CLR()
#define SEPS525_SDATA_SET()	DISPLAY_SDI_SET()
#define SEPS525_SDATA_CLR()	DISPLAY_SDI_CLR()
#define SEPS525_SCLK_SET()	DISPLAY_SCK_SET()
#define SEPS525_SCLK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SEPS525_WR()   	SEPS525_WR_CLR(); \
							SEPS525_WR_SET();
#else
 #define SEPS525_WR()
#endif

#define	SEPS525_DATA		DISPLAY_DATAPORT
#define SEPS525_CMD			DISPLAY_CMDPORT



/* Display Control Functions Prototype */
extern void SEPS525_reset(void);
extern void SEPS525_init(void);
extern void SEPS525_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SEPS525_wr_cmd(uint8_t cmd);
extern void SEPS525_wr_dat(uint8_t dat);
extern void SEPS525_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SEPS525_clear(void);
extern void SEPS525_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SEPS525_init
#define Display_rect_if 		SEPS525_rect
#define Display_wr_dat_if		SEPS525_wr_gram
#define Display_wr_cmd_if		SEPS525_wr_cmd
#define Display_wr_block_if		SEPS525_wr_block
#define Display_clear_if 		SEPS525_clear

#ifdef __cplusplus
}
#endif

#endif /* SEPS525_H */
