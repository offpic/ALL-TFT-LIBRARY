/********************************************************************************/
/*!
	@file			hx8367a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P3440-W-E TFT module(4/3-wire spi mode).

    @section HISTORY
		2015.05.16	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef HX8367A_H
#define HX8367A_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8367A unique value */
/* mst be need for HX8367A */
#define OFS_COL				10
#define OFS_RAW				0
#define MAX_X				220
#define MAX_Y				176
/* MUST be need SPI access */
#define HX8367ASPI_4WIREMODE
/*#define HX8367ASPI_3WIREMODE*/

/* HX8367A SPI 8Bit 3-Wire mode Settings */
#define HX8367A_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(HX8367A_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(HX8367A_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(HX8367A_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(HX8367A_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define HX8367A_RES_SET()	DISPLAY_RES_SET()
#define HX8367A_RES_CLR()	DISPLAY_RES_CLR()
#define HX8367A_CS_SET()	DISPLAY_CS_SET()
#define HX8367A_CS_CLR()	DISPLAY_CS_CLR()
#if defined(USE_HX8367A_SPI_TFT)
 #if defined(HX8367ASPI_4WIREMODE)
  #define HX8367A_DC_SET()	DISPLAY_DC_SET()
  #define HX8367A_DC_CLR()	DISPLAY_DC_CLR()
 #else
  #define HX8367A_DC_SET()
  #define HX8367A_DC_CLR()
 #endif
#else
 #define HX8367A_DC_SET()	DISPLAY_DC_SET()
 #define HX8367A_DC_CLR()	DISPLAY_DC_CLR()
#endif
#define HX8367A_WR_SET()	DISPLAY_WR_SET()
#define HX8367A_WR_CLR()	DISPLAY_WR_CLR()
#define HX8367A_RD_SET()	DISPLAY_RD_SET()
#define HX8367A_RD_CLR()	DISPLAY_RD_CLR()
#define HX8367A_SCK_SET()	DISPLAY_SCK_SET()
#define HX8367A_SCK_CLR()	DISPLAY_SCK_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8367A_WR()   	HX8367A_WR_CLR(); \
							HX8367A_WR_SET();
#else
 #define HX8367A_WR()
#endif

#define	HX8367A_DATA		DISPLAY_DATAPORT
#define HX8367A_CMD			DISPLAY_CMDPORT

/* Display Control Functions Prototype */
extern void HX8367A_reset(void);
extern void HX8367A_init(void);
extern void HX8367A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8367A_wr_cmd(uint8_t cmd);
extern void HX8367A_wr_dat(uint8_t dat);
extern void HX8367A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8367A_clear(void);
extern uint8_t HX8367A_rd_cmd(uint8_t cmd);
extern void HX8367A_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8367A_init
#define Display_rect_if 		HX8367A_rect
#define Display_wr_dat_if		HX8367A_wr_gram
#define Display_wr_cmd_if		HX8367A_wr_cmd
#define Display_wr_block_if		HX8367A_wr_block
#define Display_clear_if 		HX8367A_clear

#ifdef __cplusplus
}
#endif

#endif /* HX8367A_H */
