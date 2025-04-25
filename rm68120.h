/********************************************************************************/
/*!
	@file			rm68120.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KGM1066A0			(RM68120)	8/16bit mode.				@n
					 -FPC-CPTWV4058-JXY	(RM68180)	8bit mode only.

    @section HISTORY
		2016.06.01	V1.00	Stable Release.
		2016.11.04	V2.00	Revised read idcode routine.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef RM68120_H
#define RM68120_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* RM68120 unique value */
/* mst be need for RM68120 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				800

/* Display Contol Macros */
#define RM68120_RES_SET()	DISPLAY_RES_SET()
#define RM68120_RES_CLR()	DISPLAY_RES_CLR()
#define RM68120_CS_SET()	DISPLAY_CS_SET()
#define RM68120_CS_CLR()	DISPLAY_CS_CLR()
#define RM68120_DC_SET()	DISPLAY_DC_SET()
#define RM68120_DC_CLR()	DISPLAY_DC_CLR()
#define RM68120_WR_SET()	DISPLAY_WR_SET()
#define RM68120_WR_CLR()	DISPLAY_WR_CLR()
#define RM68120_RD_SET()	DISPLAY_RD_SET()
#define RM68120_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define RM68120_WR()   	RM68120_WR_CLR(); \
							RM68120_WR_SET();
#else
 #define RM68120_WR()
#endif

#define	RM68120_DATA		DISPLAY_DATAPORT
#define RM68120_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void RM68120_reset(void);
extern void RM68120_init(void);
extern void RM68120_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void RM68120_wr_cmd(uint16_t cmd);
extern void RM68120_wr_dat(uint16_t dat);
extern void RM68120_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void RM68120_clear(void);
extern uint16_t RM68120_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			RM68120_init
#define Display_rect_if 		RM68120_rect
#define Display_wr_dat_if		RM68120_wr_dat
#define Display_wr_cmd_if		RM68120_wr_cmd
#define Display_wr_block_if		RM68120_wr_block
#define Display_clear_if 		RM68120_clear

#ifdef __cplusplus
}
#endif

#endif /* RM68120_H */
