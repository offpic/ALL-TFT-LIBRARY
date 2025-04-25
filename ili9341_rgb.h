/********************************************************************************/
/*!
	@file			ili9341_rgb.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				 	 	 @n
					Available TFT-LCM are listed below.							 	 	 @n
					 -STM32F429I-Discovery		(ILI9341)	4-Wire,8bitSerial-1 and
															RGB-Interface

    @section HISTORY
		2014.03.11	V1.00	First Release.
		2014.05.29  V2.00	Fixed ILI9341 Initialization.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ILI9341_RGB_H
#define ILI9341_RGB_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* check header file version for fool proof */
#if !defined(USE_32F429IDISCOVERY)
#error "This function works on STM32F429I-Discovery ONLY!!!"
#endif

/* ILI934x unique value */
/* mst be need for ILI934x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define ILI934x_RES_SET()	DISPLAY_RES_SET()
#define ILI934x_RES_CLR()	DISPLAY_RES_CLR()
#define ILI934x_CS_SET()	DISPLAY_CS_SET()
#define ILI934x_CS_CLR()	DISPLAY_CS_CLR()
#define ILI934x_DC_SET()	DISPLAY_DC_SET()
#define ILI934x_DC_CLR()	DISPLAY_DC_CLR()
#define ILI934x_WR_SET()	DISPLAY_WR_SET()
#define ILI934x_WR_CLR()	DISPLAY_WR_CLR()
#define ILI934x_RD_SET()	DISPLAY_RD_SET()
#define ILI934x_RD_CLR()	DISPLAY_RD_CLR()
#define ILI934x_SCK_SET()	DISPLAY_SCK_SET()
#define ILI934x_SCK_CLR()	DISPLAY_SCK_CLR()
#define ILI934x_SDI_SET()	DISPLAY_SDI_SET()
#define ILI934x_SDI_CLR()	DISPLAY_SDI_CLR()
#define ILI934x_SDO_SET()	DISPLAY_SDO_SET()
#define ILI934x_SDO_CLR()	DISPLAY_SDO_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI934x_WR()   	ILI934x_WR_CLR(); \
							ILI934x_WR_SET();
#else
 #define ILI934x_WR()
#endif

#define	ILI934x_DATA		DISPLAY_DATAPORT
#define ILI934x_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI934x_reset(void);
extern void ILI934x_init(void);
extern void ILI934x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI934x_wr_cmd(uint8_t cmd);
extern void ILI934x_wr_dat(uint8_t dat);
extern void ILI934x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI934x_clear(void);
extern uint16_t ILI934x_rd_cmd(uint8_t cmd);
extern void ILI934x_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			ILI934x_init
extern void Display_rect_if(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void Display_wr_dat_if(uint16_t gram);
#define Display_wr_cmd_if		ILI934x_wr_cmd
extern void Display_wr_block_if(uint8_t* blockdata,unsigned int datacount);
extern void Display_clear_if(void);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_RGB_H */
