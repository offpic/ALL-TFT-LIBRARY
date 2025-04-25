/********************************************************************************/
/*!
	@file			hx8369a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KD043FM-1			(HX8369A)	8/16bit mode.

    @section HISTORY
		2014.05.01	V1.00	First Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef HX8369A_H
#define HX8369A_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8369A unique value */
/* mst be need for HX8369A */
#define OFS_COL				0
#define OFS_RAW				7
#define MAX_X				480
#define MAX_Y				800

/* Display Contol Macros */
#define HX8369A_RES_SET()	DISPLAY_RES_SET()
#define HX8369A_RES_CLR()	DISPLAY_RES_CLR()
#define HX8369A_CS_SET()	DISPLAY_CS_SET()
#define HX8369A_CS_CLR()	DISPLAY_CS_CLR()
#define HX8369A_DC_SET()	DISPLAY_DC_SET()
#define HX8369A_DC_CLR()	DISPLAY_DC_CLR()
#define HX8369A_WR_SET()	DISPLAY_WR_SET()
#define HX8369A_WR_CLR()	DISPLAY_WR_CLR()
#define HX8369A_RD_SET()	DISPLAY_RD_SET()
#define HX8369A_RD_CLR()	DISPLAY_RD_CLR()
#define HX8369A_SCK_SET()	DISPLAY_SCK_SET()
#define HX8369A_SCK_CLR()	DISPLAY_SCK_CLR()
#define HX8369A_SDI_SET()	DISPLAY_SDI_SET()
#define HX8369A_SDI_CLR()	DISPLAY_SDI_CLR()
#define HX8369A_SDO_SET()	DISPLAY_SDO_SET()
#define HX8369A_SDO_CLR()	DISPLAY_SDO_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8369A_WR()   	HX8369A_WR_CLR(); \
							HX8369A_WR_SET();
#else
 #define HX8369A_WR()
#endif

#define	HX8369A_DATA		DISPLAY_DATAPORT
#define HX8369A_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8369A_reset(void);
extern void HX8369A_init(void);
extern void HX8369A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8369A_wr_cmd(uint8_t cmd);
extern void HX8369A_wr_dat(uint8_t dat);
extern void HX8369A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8369A_clear(void);
extern uint16_t HX8369A_rd_cmd(uint8_t cmd);
extern void HX8369A_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			HX8369A_init
#define Display_rect_if 		HX8369A_rect
#define Display_wr_dat_if		HX8369A_wr_gram
#define Display_wr_cmd_if		HX8369A_wr_cmd
#define Display_wr_block_if		HX8369A_wr_block
#define Display_clear_if 		HX8369A_clear

#ifdef __cplusplus
}
#endif

#endif /* HX8369A_H */
