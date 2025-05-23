/********************************************************************************/
/*!
	@file			s6d04d1.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TFT1P3520-E				(S6D04D1)	16bit mode.

    @section HISTORY
		2014.08.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef S6D04D1_H
#define S6D04D1_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S6D04D1 unique value */
/* mst be need for S6D04D1 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				400

/* Display Contol Macros */
#define S6D04D1_RES_SET()	DISPLAY_RES_SET()
#define S6D04D1_RES_CLR()	DISPLAY_RES_CLR()
#define S6D04D1_CS_SET()	DISPLAY_CS_SET()
#define S6D04D1_CS_CLR()	DISPLAY_CS_CLR()
#define S6D04D1_DC_SET()	DISPLAY_DC_SET()
#define S6D04D1_DC_CLR()	DISPLAY_DC_CLR()
#define S6D04D1_WR_SET()	DISPLAY_WR_SET()
#define S6D04D1_WR_CLR()	DISPLAY_WR_CLR()
#define S6D04D1_RD_SET()	DISPLAY_RD_SET()
#define S6D04D1_RD_CLR()	DISPLAY_RD_CLR()
#define S6D04D1_SCK_SET()	DISPLAY_SCK_SET()
#define S6D04D1_SCK_CLR()	DISPLAY_SCK_CLR()
#define S6D04D1_SDI_SET()	DISPLAY_SDI_SET()
#define S6D04D1_SDI_CLR()	DISPLAY_SDI_CLR()
#define S6D04D1_SDO_SET()	DISPLAY_SDO_SET()
#define S6D04D1_SDO_CLR()	DISPLAY_SDO_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S6D04D1_WR()   	S6D04D1_WR_CLR();S6D04D1_WR_CLR(); \
							S6D04D1_WR_SET();
#else
 #define S6D04D1_WR()
#endif

#define	S6D04D1_DATA		DISPLAY_DATAPORT
#define S6D04D1_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S6D04D1_reset(void);
extern void S6D04D1_init(void);
extern void S6D04D1_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S6D04D1_wr_cmd(uint8_t cmd);
extern void S6D04D1_wr_dat(uint8_t dat);
extern void S6D04D1_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S6D04D1_clear(void);
extern uint16_t S6D04D1_rd_cmd(uint8_t cmd);
extern void S6D04D1_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			S6D04D1_init
#define Display_rect_if 		S6D04D1_rect
#define Display_wr_dat_if		S6D04D1_wr_gram
#define Display_wr_cmd_if		S6D04D1_wr_cmd
#define Display_wr_block_if		S6D04D1_wr_block
#define Display_clear_if 		S6D04D1_clear

#ifdef __cplusplus
}
#endif

#endif /* S6D04D1_H */
