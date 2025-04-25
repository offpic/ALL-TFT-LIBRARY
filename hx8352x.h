/********************************************************************************/
/*!
	@file			hx8352x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -QD-ET3207ABT				(HX8352A)	16bit mode only.	@n
					 -S95461C					(HX8352B)	16bit mode only.	@n
					 -QDTFT2600LB				(HX8352C)	8/16bit mode.

    @section HISTORY
		2012.08.27	V1.00	Revised From hx8352a.c
		2013.07.10	V2.00	Added HX8352C Devices Ready.
							Added 3/4wire Serial Handlings.
		2013.08.10	V3.00	Added HX8352C Devices.
		2023.05.01	V4.00	Removed unused delay function.
		2023.08.01	V5.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef HX8352X_H
#define HX8352X_H 0x0500

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8352x unique value */
/* mst be need for HX8352x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				400
/* MUST be need SPI access */
/*#define HX8352xSPI_4WIREMODE*/
#define HX8352xSPI_3WIREMODE

/* HX8352B SPI 8Bit 3-Wire mode Settings */
#define HX8352x_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(HX8352x_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(HX8352x_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(HX8352x_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(HX8352x_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define HX8352x_RES_SET()	DISPLAY_RES_SET()
#define HX8352x_RES_CLR()	DISPLAY_RES_CLR()
#define HX8352x_CS_SET()	DISPLAY_CS_SET()
#define HX8352x_CS_CLR()	DISPLAY_CS_CLR()
#if 	defined(HX8352xSPI_3WIREMODE)
 #define HX8352x_DC_SET()
 #define HX8352x_DC_CLR()
#elif 	defined(HX8352xSPI_4WIREMODE)
 #define HX8352x_DC_SET()	DISPLAY_DC_SET()
 #define HX8352x_DC_CLR()	DISPLAY_DC_CLR()
#else
 #error "U MUST Select HX8352x SPI Mode!!"
#endif
#define HX8352x_WR_SET()	DISPLAY_WR_SET()
#define HX8352x_WR_CLR()	DISPLAY_WR_CLR()
#define HX8352x_RD_SET()	DISPLAY_RD_SET()
#define HX8352x_RD_CLR()	DISPLAY_RD_CLR()
#define HX8352x_SCK_SET()	DISPLAY_SCK_SET()
#define HX8352x_SCK_CLR()	DISPLAY_SCK_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8352x_WR()   	HX8352x_WR_CLR(); \
							HX8352x_WR_SET();
#else
 #define HX8352x_WR()
#endif

#define	HX8352x_DATA		DISPLAY_DATAPORT
#define HX8352x_CMD			DISPLAY_CMDPORT


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8352x_WR()   	HX8352x_WR_CLR(); \
							HX8352x_WR_SET();
#else
 #define HX8352x_WR()
#endif

#define	HX8352x_DATA		DISPLAY_DATAPORT
#define HX8352x_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8352x_reset(void);
extern void HX8352x_init(void);
extern void (*HX8352x_rect)(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8352x_wr_cmd(uint8_t cmd);
extern void HX8352x_wr_dat(uint8_t dat);
extern void HX8352x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8352x_clear(void);
extern uint8_t HX8352x_rd_cmd(uint8_t cmd);
extern void HX8352x_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8352x_init
#define Display_rect_if 		HX8352x_rect
#define Display_wr_dat_if		HX8352x_wr_gram
#define Display_wr_cmd_if		HX8352x_wr_cmd
#define Display_wr_block_if		HX8352x_wr_block
#define Display_clear_if 		HX8352x_clear

#ifdef __cplusplus
}
#endif

#endif /* HX8352X_H */
