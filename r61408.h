/********************************************************************************/
/*!
	@file			r61408.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TK040F1532				(R61408)	8/16bit mode.		

    @section HISTORY
		2023.06.01	V1.00	Start Here.
		2023.08.01	V2.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef R61408_H
#define R61408_H 0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* R61408 unique value */
/* mst be need for R61408 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				800

/* Display Contol Macros */
#define R61408_RES_SET()	DISPLAY_RES_SET()
#define R61408_RES_CLR()	DISPLAY_RES_CLR()
#define R61408_CS_SET()		DISPLAY_CS_SET()
#define R61408_CS_CLR()		DISPLAY_CS_CLR()
#define R61408_DC_SET()		DISPLAY_DC_SET()
#define R61408_DC_CLR()		DISPLAY_DC_CLR()
#define R61408_WR_SET()		DISPLAY_WR_SET()
#define R61408_WR_CLR()		DISPLAY_WR_CLR()
#define R61408_RD_SET()		DISPLAY_RD_SET()
#define R61408_RD_CLR()		DISPLAY_RD_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define R61408_WR()   		R61408_WR_CLR(); \
							R61408_WR_SET();
#else
 #define R61408_WR()
#endif

#define	R61408_DATA			DISPLAY_DATAPORT
#define R61408_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void R61408_reset(void);
extern void R61408_init(void);
extern void R61408_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void R61408_wr_cmd(uint8_t cmd);
extern void R61408_wr_dat(uint8_t dat);
extern void R61408_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void R61408_clear(void);
extern uint16_t R61408_rd_cmd(uint8_t cmd);
extern void R61408_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			R61408_init
#define Display_rect_if 		R61408_rect
#define Display_wr_dat_if		R61408_wr_gram
#define Display_wr_cmd_if		R61408_wr_cmd
#define Display_wr_block_if		R61408_wr_block
#define Display_clear_if 		R61408_clear

#ifdef __cplusplus
}
#endif

#endif /* R61408_H */
