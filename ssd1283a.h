/********************************************************************************/
/*!
	@file			ssd1283a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive H016IT01 TFT module(4-wire,8-bit serial only)

    @section HISTORY
		2010.03.01	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Changed Some Description (16bit Write as GRAM write).
		2011.03.10	V4.00	C++ Ready.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef SSD1283A_H
#define SSD1283A_H 0x0600

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SSD1283A Unique Value	 */
/* MUST be need for SSD1283A */
#define OFS_RAW				0
#define OFS_COL				2
#define MAX_X				130
#define MAX_Y				130

/* Display Contol Macros */
#define SSD1283A_RES_SET()		DISPLAY_RES_SET()
#define SSD1283A_RES_CLR()		DISPLAY_RES_CLR()
#define SSD1283A_CS_SET()		DISPLAY_CS_SET()
#define SSD1283A_CS_CLR()		DISPLAY_CS_CLR()
#define SSD1283A_DC_SET()		DISPLAY_DC_SET()
#define SSD1283A_DC_CLR()		DISPLAY_DC_CLR()
#define SSD1283A_SDATA_SET()	DISPLAY_SDI_SET()
#define SSD1283A_SDATA_CLR()	DISPLAY_SDI_CLR()
#define SSD1283A_SCLK_SET()		DISPLAY_SCK_SET()
#define SSD1283A_SCLK_CLR()		DISPLAY_SCK_CLR()


/* Display Control Functions Prototype */
extern void SSD1283A_reset(void);
extern void SSD1283A_init(void);
extern void SSD1283A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SSD1283A_wr_cmd(uint8_t cmd);
extern void SSD1283A_wr_dat(uint16_t dat);
extern void SSD1283A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SSD1283A_clear(void);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SSD1283A_init
#define Display_rect_if 		SSD1283A_rect
#define Display_wr_dat_if	    SSD1283A_wr_dat
#define Display_wr_cmd_if		SSD1283A_wr_cmd
#define Display_wr_block_if		SSD1283A_wr_block
#define Display_clear_if 		SSD1283A_clear

#ifdef __cplusplus
}
#endif

#endif /* SSD1283A_H */
