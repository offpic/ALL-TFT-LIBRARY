/********************************************************************************/
/*!
	@file			hx8363b.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P1400-E TFT module(8/16bit mode).

    @section HISTORY
		2023.08.01	V1.00	First Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef HX8363B_H
#define HX8363B_H 0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8363B unique value */
/* mst be need for HX8363B */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				854

/* Display Contol Macros */
#define HX8363B_RES_SET()	DISPLAY_RES_SET()
#define HX8363B_RES_CLR()	DISPLAY_RES_CLR()
#define HX8363B_CS_SET()	DISPLAY_CS_SET()
#define HX8363B_CS_CLR()	DISPLAY_CS_CLR()
#define HX8363B_DC_SET()	DISPLAY_DC_SET()
#define HX8363B_DC_CLR()	DISPLAY_DC_CLR()
#define HX8363B_WR_SET()	DISPLAY_WR_SET()
#define HX8363B_WR_CLR()	DISPLAY_WR_CLR()
#define HX8363B_RD_SET()	DISPLAY_RD_SET()
#define HX8363B_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8363B_WR()   	HX8363B_WR_CLR(); \
							HX8363B_WR_SET();
#else
 #define HX8363B_WR()
#endif

#define	HX8363B_DATA		DISPLAY_DATAPORT
#define HX8363B_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8363B_reset(void);
extern void HX8363B_init(void);
extern void HX8363B_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8363B_wr_cmd(uint8_t cmd);
extern void HX8363B_wr_dat(uint8_t dat);
extern void HX8363B_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8363B_clear(void);
extern uint16_t HX8363B_rd_cmd(uint8_t cmd);
extern void HX8363B_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			HX8363B_init
#define Display_rect_if 		HX8363B_rect
#define Display_wr_dat_if		HX8363B_wr_gram
#define Display_wr_cmd_if		HX8363B_wr_cmd
#define Display_wr_block_if		HX8363B_wr_block
#define Display_clear_if 		HX8363B_clear

#ifdef __cplusplus
}
#endif

#endif /* HX8363B_H */
