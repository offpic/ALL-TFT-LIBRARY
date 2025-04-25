/********************************************************************************/
/*!
	@file			otm8009a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -INANBO-T43C-8009-V2		(OTM8009A)		16bit mode.		@n
					 -BBM397003I4				(OTM8012A)		16bit mode.		@n
					 -NSF397WV4402				(OTM8009A)		16bit mode.

    @section HISTORY
		2014.05.01	V1.00	Stable Release
		2014.08.03	V2.00	Added OTM8012A Device.
		2014.10.15	V3.00	Fixed 8-bit access bug.
		2015.05.15	V4.00	Added Normally White Screens.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef OTM8009A_H
#define OTM8009A_H 0x0600

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* OTM8009A unique value */
/* mst be need for OTM8009A */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				480
#define MAX_Y				800

/* Display Contol Macros */
#define OTM8009A_RES_SET()	DISPLAY_RES_SET()
#define OTM8009A_RES_CLR()	DISPLAY_RES_CLR()
#define OTM8009A_CS_SET()	DISPLAY_CS_SET()
#define OTM8009A_CS_CLR()	DISPLAY_CS_CLR()
#define OTM8009A_DC_SET()	DISPLAY_DC_SET()
#define OTM8009A_DC_CLR()	DISPLAY_DC_CLR()
#define OTM8009A_WR_SET()	DISPLAY_WR_SET()
#define OTM8009A_WR_CLR()	DISPLAY_WR_CLR()
#define OTM8009A_RD_SET()	DISPLAY_RD_SET()
#define OTM8009A_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define OTM8009A_WR()   	OTM8009A_WR_CLR(); \
							OTM8009A_WR_SET();
#else
 #define OTM8009A_WR()
#endif

#define	OTM8009A_DATA		DISPLAY_DATAPORT
#define OTM8009A_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void OTM8009A_reset(void);
extern void OTM8009A_init(void);
extern void OTM8009A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void OTM8009A_wr_cmd(uint16_t cmd);
extern void OTM8009A_wr_dat(uint16_t dat);
extern void OTM8009A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void OTM8009A_clear(void);
extern uint16_t OTM8009A_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			OTM8009A_init
#define Display_rect_if 		OTM8009A_rect
#define Display_wr_dat_if		OTM8009A_wr_dat
#define Display_wr_cmd_if		OTM8009A_wr_cmd
#define Display_wr_block_if		OTM8009A_wr_block
#define Display_clear_if 		OTM8009A_clear

#ifdef __cplusplus
}
#endif

#endif /* OTM8009A_H */
