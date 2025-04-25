/********************************************************************************/
/*!
	@file			rk043fn48h_rgb.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				 	 	 @n
					Available TFT-LCM are listed below.							 	 	 @n
					 -STM32F746G-Discovery		(RK043FN48H)	RGB-Interface

    @section HISTORY
		2015.08.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef RK043FN48H_RGB_H
#define RK043FN48H_RGB_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* check header file version for fool proof */
#if !defined(USE_STM32746G_DISCOVERY)
#error "This function works on STM32F746G-Discovery ONLY!!!"
#endif

/* RK043FN48H unique value */
/* mst be need for RK043FN48H */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				272

/* Display Contol Macros */


/* Display Control Functions Prototype */
extern void RK043FN48H_reset(void);
extern void RK043FN48H_init(void);
extern void RK043FN48H_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void RK043FN48H_wr_cmd(uint8_t cmd);
extern void RK043FN48H_wr_dat(uint8_t dat);
extern void RK043FN48H_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void RK043FN48H_clear(void);
extern uint16_t RK043FN48H_rd_cmd(uint8_t cmd);
extern void RK043FN48H_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			RK043FN48H_init
extern void Display_rect_if(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void Display_wr_dat_if(uint16_t gram);
#define Display_wr_cmd_if		RK043FN48H_wr_cmd
extern void Display_wr_block_if(uint8_t* blockdata,unsigned int datacount);
extern void Display_clear_if(void);

#ifdef __cplusplus
}
#endif

#endif /* RK043FN48H_RGB_H */
