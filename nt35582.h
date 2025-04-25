/********************************************************************************/
/*!
	@file			nt35582.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -LT043ANYB02				(NT35582)	8/16bit mode.

    @section HISTORY
		2015.05.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef NT35582_H
#define NT35582_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* NT35582 unique value */
/* mst be need for NT35582 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				800

/* Display Contol Macros */
#define NT35582_RES_SET()	DISPLAY_RES_SET()
#define NT35582_RES_CLR()	DISPLAY_RES_CLR()
#define NT35582_CS_SET()	DISPLAY_CS_SET()
#define NT35582_CS_CLR()	DISPLAY_CS_CLR()
#define NT35582_DC_SET()	DISPLAY_DC_SET()
#define NT35582_DC_CLR()	DISPLAY_DC_CLR()
#define NT35582_WR_SET()	DISPLAY_WR_SET()
#define NT35582_WR_CLR()	DISPLAY_WR_CLR()
#define NT35582_RD_SET()	DISPLAY_RD_SET()
#define NT35582_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define NT35582_WR()   	NT35582_WR_CLR(); \
							NT35582_WR_SET();
#else
 #define NT35582_WR()
#endif

#define	NT35582_DATA		DISPLAY_DATAPORT
#define NT35582_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void NT35582_reset(void);
extern void NT35582_init(void);
extern void NT35582_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void NT35582_wr_cmd(uint16_t cmd);
extern void NT35582_wr_dat(uint16_t dat);
extern void NT35582_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void NT35582_clear(void);
extern uint16_t NT35582_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			NT35582_init
#define Display_rect_if 		NT35582_rect
#define Display_wr_dat_if		NT35582_wr_dat
#define Display_wr_cmd_if		NT35582_wr_cmd
#define Display_wr_block_if		NT35582_wr_block
#define Display_clear_if 		NT35582_clear

#ifdef __cplusplus
}
#endif

#endif /* NT35582_H */
