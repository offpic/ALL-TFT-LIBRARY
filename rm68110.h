/********************************************************************************/
/*!
	@file			rm68110.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -DT18QQ003-B1R2				(4-wire serial)				@n
					 -BBM018027C1-F8				(8bit mode)

    @section HISTORY
		2014.10.21	V1.00	Stable Release.
		2015.11.03  V2.00	Added BBM018027C1-F8 Module Support.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef RM68110_H
#define RM68110_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* RM68110 Unique Value		*/
/* MUST be need for RM68110	*/
#define OFS_RAW				0
#define OFS_COL				0
#define MAX_X				128
#define MAX_Y				160

/* Display Contol Macros */
#define RM68110_RES_SET()	DISPLAY_RES_SET()
#define RM68110_RES_CLR()	DISPLAY_RES_CLR()
#define RM68110_CS_SET()	DISPLAY_CS_SET()
#define RM68110_CS_CLR()	DISPLAY_CS_CLR()
#define RM68110_DC_SET()	DISPLAY_DC_SET()
#define RM68110_DC_CLR()	DISPLAY_DC_CLR()
#define RM68110_WR_SET()	DISPLAY_WR_SET()
#define RM68110_WR_CLR()	DISPLAY_WR_CLR()
#define RM68110_RD_SET()	DISPLAY_RD_SET()
#define RM68110_RD_CLR()	DISPLAY_RD_CLR()
#define RM68110_SDATA_SET()	DISPLAY_SDI_SET()
#define RM68110_SDATA_CLR()	DISPLAY_SDI_CLR()
#define RM68110_SCLK_SET()	DISPLAY_SCK_SET()
#define RM68110_SCLK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define RM68110_WR()   	RM68110_WR_CLR(); \
							RM68110_WR_SET();
#else
 #define RM68110_WR()
#endif

#define	RM68110_DATA		DISPLAY_DATAPORT
#define RM68110_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void RM68110_reset(void);
extern void RM68110_init(void);
extern void RM68110_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void RM68110_wr_cmd(uint8_t cmd);
extern void RM68110_wr_dat(uint8_t dat);
extern void RM68110_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void RM68110_clear(void);
extern void RM68110_wr_gram(uint16_t gram);
extern uint16_t RM68110_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			RM68110_init
#define Display_rect_if 		RM68110_rect
#define Display_wr_dat_if		RM68110_wr_gram
#define Display_wr_cmd_if		RM68110_wr_cmd
#define Display_wr_block_if		RM68110_wr_block
#define Display_clear_if 		RM68110_clear

#ifdef __cplusplus
}
#endif

#endif /* RM68110_H */
