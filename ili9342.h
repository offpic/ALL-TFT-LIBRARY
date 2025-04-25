/********************************************************************************/
/*!
	@file			ili9342.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -XYL62291B-2B		(ILI9342)	8/16bit mode.				@n
					 -YB020C01-40		(ILI9342C)	3-wire/9-bit serial only!

    @section HISTORY
		2013.01.02	V1.00	Stable Release
		2016.11.04	V2.00	Added 3-Wire/9bit Serial Support.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ILI9342_H
#define ILI9342_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* Module Driver Configure */
#ifdef USE_ILI9342_TFT
 /* U MUST select one from those modules */
 #define USE_XYL62291B_2B
#else
 /* U MUST select one from those modules */
 #define USE_YB020C01_40
#endif

/* ILI9342 Unique Value		*/
#if defined(USE_YB020C01_40)
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				320
#define MAX_Y				240
#define USEMODULE			USE_YB020C01_40
#define ILI9342SPI_3WIREMODE

#elif  defined(USE_XYL62291B_2B)
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				320
#define MAX_Y				240

#endif


/* Display Contol Macros */
#ifndef USE_ILI9342_TFT

#ifdef ILI9342SPI_3WIREMODE
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
#endif

#else /* 4WIRE-8BitMODE */
 #if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
  #error "4-WireMode Supports 8bit-SPI Handling ONLY !!"
 #endif
 #define DNC_CMD()	
 #define DNC_DAT()	
#endif


/* Display Contol Macros */
#define ILI9342_RES_SET()		DISPLAY_RES_SET()
#define ILI9342_RES_CLR()		DISPLAY_RES_CLR()
#define ILI9342_CS_SET()		DISPLAY_CS_SET()
#define ILI9342_CS_CLR()		DISPLAY_CS_CLR()
#ifndef USE_ILI9342_TFT
 #if 	defined(ILI9342SPI_3WIREMODE)
  #define ILI9342_DC_SET()
  #define ILI9342_DC_CLR()
 #elif 	defined(ILI9342SPI_4WIREMODE)
  #define ILI9342_DC_SET()		DISPLAY_DC_SET()
  #define ILI9342_DC_CLR()		DISPLAY_DC_CLR()
 #else
  #error "U MUST Select ILI9342 SPI Mode!!"
 #endif
#else
 #define ILI9342_DC_SET()		DISPLAY_DC_SET()
 #define ILI9342_DC_CLR()		DISPLAY_DC_CLR()
#endif
#define ILI9342_WR_SET()		DISPLAY_WR_SET()
#define ILI9342_WR_CLR()		DISPLAY_WR_CLR()
#define ILI9342_RD_SET()		DISPLAY_RD_SET()
#define ILI9342_RD_CLR()		DISPLAY_RD_CLR()
#define ILI9342_SDATA_SET()		DISPLAY_SDI_SET()
#define ILI9342_SDATA_CLR()		DISPLAY_SDI_CLR()
#define ILI9342_SCK_SET()		DISPLAY_SCK_SET()
#define ILI9342_SCK_CLR()		DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9342_WR()   	ILI9342_WR_CLR(); \
							ILI9342_WR_SET();
#else
 #define ILI9342_WR()
#endif

#define	ILI9342_DATA		DISPLAY_DATAPORT
#define ILI9342_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI9342_reset(void);
extern void ILI9342_init(void);
extern void ILI9342_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9342_wr_cmd(uint8_t cmd);
extern void ILI9342_wr_dat(uint8_t dat);
extern void ILI9342_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9342_clear(void);
extern uint16_t ILI9342_rd_cmd(uint8_t cmd);
extern void ILI9342_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			ILI9342_init
#define Display_rect_if 		ILI9342_rect
#define Display_wr_dat_if		ILI9342_wr_gram
#define Display_wr_cmd_if		ILI9342_wr_cmd
#define Display_wr_block_if		ILI9342_wr_block
#define Display_clear_if 		ILI9342_clear

#ifdef __cplusplus
}
#endif

#endif /* ILI9342_H */
