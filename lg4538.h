/********************************************************************************/
/*!
	@file			lg4538.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				 	 @n
					It can drive SF-TC240G-9366A-N	TFT module(8/16bit & spi mode).

    @section HISTORY
		2011.11.10	V1.00	First Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef LG4538_H
#define LG4538_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* LG4538 unique value */
/* mst be need for LG4538 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320
/* MUST be need SPI access */
/*#define LG4538SPI_4WIREMODE*/
#define LG4538SPI_3WIREMODE

/* LG4538 SPI 8Bit 3-Wire mode Settings */
#define LG4538_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(LG4538_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(LG4538_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(LG4538_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(LG4538_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define LG4538_RES_SET()	DISPLAY_RES_SET()
#define LG4538_RES_CLR()	DISPLAY_RES_CLR()
#define LG4538_CS_SET()		DISPLAY_CS_SET()
#define LG4538_CS_CLR()		DISPLAY_CS_CLR()
#if 	defined(LG4538SPI_3WIREMODE)
 #define LG4538_DC_SET()
 #define LG4538_DC_CLR()
#elif 	defined(LG4538SPI_4WIREMODE)
 #define LG4538_DC_SET()	DISPLAY_DC_SET()
 #define LG4538_DC_CLR()	DISPLAY_DC_CLR()
#else
 #error "U MUST Select LG4538 SPI Mode!!"
#endif
#define LG4538_WR_SET()		DISPLAY_WR_SET()
#define LG4538_WR_CLR()		DISPLAY_WR_CLR()
#define LG4538_RD_SET()		DISPLAY_RD_SET()
#define LG4538_RD_CLR()		DISPLAY_RD_CLR()
#define LG4538_SCK_SET()	DISPLAY_SCK_SET()
#define LG4538_SCK_CLR()	DISPLAY_SCK_CLR()
#define LG4538_SDI_SET()	DISPLAY_SDI_SET()
#define LG4538_SDI_CLR()	DISPLAY_SDI_CLR()
#define LG4538_SDO_SET()	DISPLAY_SDO_SET()
#define LG4538_SDO_CLR()	DISPLAY_SDO_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define LG4538_WR()   		LG4538_WR_CLR(); \
							LG4538_WR_SET();
#else
 #define LG4538_WR()
#endif

#define	LG4538_DATA			DISPLAY_DATAPORT
#define LG4538_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void LG4538_reset(void);
extern void LG4538_init(void);
extern void LG4538_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void LG4538_wr_cmd(uint8_t cmd);
extern void LG4538_wr_dat(uint8_t dat);
extern void LG4538_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void LG4538_clear(void);
extern uint16_t LG4538_rd_cmd(uint8_t cmd);
extern void LG4538_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			LG4538_init
#define Display_rect_if 		LG4538_rect
#define Display_wr_dat_if		LG4538_wr_gram
#define Display_wr_cmd_if		LG4538_wr_cmd
#define Display_wr_block_if		LG4538_wr_block
#define Display_clear_if 		LG4538_clear

#ifdef __cplusplus
}
#endif

#endif /* LG4538_H */
