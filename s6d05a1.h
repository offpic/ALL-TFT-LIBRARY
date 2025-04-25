/********************************************************************************/
/*!
	@file			s6d05a1.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TFT1P3925-E				(S6D05A1)	16bit mode.

    @section HISTORY
		2013.12.30	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef S6D05A1_H
#define S6D05A1_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S6D05A1 unique value */
/* mst be need for S6D05A1 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				320
#define MAX_Y				480

/* For TFT1P2797-E (with TouchPanel model S6D05A1) module Force */
/*#define USE_TFT1P2797_E*/
/* For TFT1P7134-E (with TouchPanel model R61581) module Force */
/*#define USE_TFT1P7134_E*/
#if !defined(USE_S6D05A1_SPI_TFT)
 #if	defined(USE_TOUCH_CTRL)
	#define USE_TFT1P2797_E				/* For TFT1P2797 (with TouchPanel model) module */
 #else 									/* For S95517 ( NO TouchPanel & Normally Black)module */
 #endif
#endif

/* Serect MIPI-DBI TypeC Handlings */
#define S6D05A1SPI_4WIREMODE
/*#define S6D05A1SPI_3WIREMODE*/

/* Don't Touch This!! */
#if defined(USE_S6D05A1_SPI_TFT)
#ifdef S6D05A1SPI_3WIREMODE
#if defined(USE_HARDWARE_SPI)
 #if defined(SUPPORT_HARDWARE_9BIT_SPI)
  #define DNC_CMD()	
  #define DNC_DAT()	
 #else
  #error "9bit-SPI Does not Support on Hardware 8bit-SPI Handling !!"
 #endif
#elif defined(USE_SOFTWARE_SPI)
 #define DNC_CMD()			DISPLAY_SDI_CLR();	\
							CLK_OUT();
 #define DNC_DAT()			DISPLAY_SDI_SET();	\
							CLK_OUT();
#else
  #error "NOT Defined SPI Handling !"
#endif

#else /* 4WIRE-8BitMODE */
 #if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
  #error "4-WireMode Supports 8bit-SPI Handling ONLY !!"
 #endif
 #define DNC_CMD()	
 #define DNC_DAT()	
#endif
#endif

/* Display Contol Macros */
#define S6D05A1_RES_SET()	DISPLAY_RES_SET()
#define S6D05A1_RES_CLR()	DISPLAY_RES_CLR()
#define S6D05A1_CS_SET()	DISPLAY_CS_SET()
#define S6D05A1_CS_CLR()	DISPLAY_CS_CLR()
#define S6D05A1_DC_SET()	DISPLAY_DC_SET()
#define S6D05A1_DC_CLR()	DISPLAY_DC_CLR()
#define S6D05A1_WR_SET()	DISPLAY_WR_SET()
#define S6D05A1_WR_CLR()	DISPLAY_WR_CLR()
#define S6D05A1_RD_SET()	DISPLAY_RD_SET()
#define S6D05A1_RD_CLR()	DISPLAY_RD_CLR()
#define S6D05A1_SCK_SET()	DISPLAY_SCK_SET()
#define S6D05A1_SCK_CLR()	DISPLAY_SCK_CLR()
#define S6D05A1_SDI_SET()	DISPLAY_SDI_SET()
#define S6D05A1_SDI_CLR()	DISPLAY_SDI_CLR()
#define S6D05A1_SDO_SET()	DISPLAY_SDO_SET()
#define S6D05A1_SDO_CLR()	DISPLAY_SDO_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S6D05A1_WR()   	S6D05A1_WR_CLR();S6D05A1_WR_CLR(); \
							S6D05A1_WR_SET();
#else
 #define S6D05A1_WR()
#endif

#define	S6D05A1_DATA		DISPLAY_DATAPORT
#define S6D05A1_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S6D05A1_reset(void);
extern void S6D05A1_init(void);
extern void S6D05A1_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S6D05A1_wr_cmd(uint8_t cmd);
extern void S6D05A1_wr_dat(uint8_t dat);
extern void S6D05A1_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S6D05A1_clear(void);
extern uint16_t S6D05A1_rd_cmd(uint8_t cmd);
extern void S6D05A1_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			S6D05A1_init
#define Display_rect_if 		S6D05A1_rect
#define Display_wr_dat_if		S6D05A1_wr_gram
#define Display_wr_cmd_if		S6D05A1_wr_cmd
#define Display_wr_block_if		S6D05A1_wr_block
#define Display_clear_if 		S6D05A1_clear

#ifdef __cplusplus
}
#endif

#endif /* S6D05A1_H */
