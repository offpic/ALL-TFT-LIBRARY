/********************************************************************************/
/*!
	@file			otm8009a_dsi.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2024.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -STM32F769I-Discovery		(OTM8009A)	DSI-Interface		@n
					 -STM32H747I-Discovery		(OTM8009A)	DSI-Interface

    @section HISTORY
		2019.10.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.
		2024.08.01	V4.00	Fixed unused parameter.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef OTM8009A_DSI_H
#define OTM8009A_DSI_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* check header file version for fool proof */
#if !(defined(USE_STM32F769I_DISCOVERY) || defined(USE_STM32H747I_DISCO))
 #error "This function works on STM32F769I & STM32H747I Discovery ONLY!!!"
#endif

/* OTM8009A unique value */
/* mst be need for OTM8009A */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				800
#define MAX_Y				480

/* Display Contol Macros */


/* Display Control Functions Prototype */
extern void OTM8009A_reset(void);
extern void OTM8009A_init(void);
extern void OTM8009A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void OTM8009A_wr_cmd(uint8_t cmd);
extern void OTM8009A_wr_dat(uint8_t dat);
extern void OTM8009A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void OTM8009A_clear(void);
extern uint16_t OTM8009A_rd_cmd(uint8_t cmd);
extern void OTM8009A_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			OTM8009A_init
extern void Display_rect_if(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void Display_wr_dat_if(uint16_t gram);
#define Display_wr_cmd_if		OTM8009A_wr_cmd
extern void Display_wr_block_if(uint8_t* blockdata,unsigned int datacount);
extern void Display_clear_if(void);

#ifdef __cplusplus
}
#endif

#endif /* OTM8009A_DSI_H */
