/********************************************************************************/
/*!
	@file			ili934x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        14.00
    @date           2023.09.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				 	 	 @n
					Available TFT-LCM are listed below.							 	 	 @n
					 -SDT028ATFT				(ILI9341)	8/16bit & 4-Wire,8bitSerial. @n
					 -SDT022ATFT				(ILI9340)	8/16bit & 4-Wire,8bitSerial. @n
					 -NHD-2.4-240320SF-CTXI#-T1 (ILI9340)	8/16bit mode.				 @n
					 -DJN 15-12378-18251		(ILI9338B)	8/16bit mode.				 @n
					 -STM32F429I-Discovery		(ILI9341)	4-Wire,8bitSerial-1 only.	 @n
					 -FGD240P3735				(NT35702)	8/16bit mode.				 @n
					 -DJN 15-12406-10481		(NT39116B)	8/16bit mode.

    @section HISTORY
		2011.11.10	V1.00	First Release.
		2011.12.23	V2.00	Data Write Bug Fix.
		2012.01.15	V3.00	Data Write Bug Fix in i8080-bus access.
						    Added NHD-2.4-240320SF-CTXI#-T1 support.
		2012.08.01	V4.00	Improved Register Read Function in Serial Interface.
		2012.11.30	V5.00	Added DJN 15-12378-18251(ILI9338B) support.
		2013.03.20  V6.00	Fixed ILI934x DeviceID Read Commands.
		2013.11.30  V7.00	Adopted STM32F429I-Discovery.
		2014.05.29  V8.00	Fixed ILI9341 Initialization.
		2014.10.15	V9.00	Fixed 8-bit access bug.
		2014.11.01 V10.00	Added FGD240P3735(NT35702) support.
		2016.11.04 V11.00	Fixed DeviceID Read Command on spi mode.
		2023.05.01 V12.00	Removed unused delay function.
		2023.08.01 V13.00	Revised release.
		2023.09.01 V14.00	Added DJN 15-12406-10481(NT39116B) support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ILI934X_H
#define ILI934X_H 0x1400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

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
#define Display_rect_if 		ILI934x_rect
#define Display_wr_dat_if		ILI934x_wr_gram
#define Display_wr_cmd_if		ILI934x_wr_cmd
#define Display_wr_block_if		ILI934x_wr_block
#define Display_clear_if 		ILI934x_clear

#ifdef __cplusplus
}
#endif

#endif /* ILI934X_H */
