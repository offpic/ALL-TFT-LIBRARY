/********************************************************************************/
/*!
	@file			nt35510.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TK040F1510 TFT module(8/16bit mode).

    @section HISTORY
		2013.05.02	V1.00	Stable Release
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01  V4.00	Revised initialize routine.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef NT35510_H
#define NT35510_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* NT35510 unique value */
/* mst be need for NT35510 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				800

/* Display Contol Macros */
#define NT35510_RES_SET()	DISPLAY_RES_SET()
#define NT35510_RES_CLR()	DISPLAY_RES_CLR()
#define NT35510_CS_SET()	DISPLAY_CS_SET()
#define NT35510_CS_CLR()	DISPLAY_CS_CLR()
#define NT35510_DC_SET()	DISPLAY_DC_SET()
#define NT35510_DC_CLR()	DISPLAY_DC_CLR()
#define NT35510_WR_SET()	DISPLAY_WR_SET()
#define NT35510_WR_CLR()	DISPLAY_WR_CLR()
#define NT35510_RD_SET()	DISPLAY_RD_SET()
#define NT35510_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define NT35510_WR()   	NT35510_WR_CLR(); \
							NT35510_WR_SET();
#else
 #define NT35510_WR()
#endif

#define	NT35510_DATA		DISPLAY_DATAPORT
#define NT35510_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void NT35510_reset(void);
extern void NT35510_init(void);
extern void NT35510_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void NT35510_wr_cmd(uint16_t cmd);
extern void NT35510_wr_dat(uint16_t dat);
extern void NT35510_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void NT35510_clear(void);
extern uint16_t NT35510_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			NT35510_init
#define Display_rect_if 		NT35510_rect
#define Display_wr_dat_if		NT35510_wr_dat
#define Display_wr_cmd_if		NT35510_wr_cmd
#define Display_wr_block_if		NT35510_wr_block
#define Display_clear_if 		NT35510_clear

#ifdef __cplusplus
}
#endif

#endif /* NT35510_H */
