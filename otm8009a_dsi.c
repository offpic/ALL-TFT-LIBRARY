/********************************************************************************/
/*!
	@file			otm8009a_dsi.c
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

/* Includes ------------------------------------------------------------------*/
#include "otm8009a_dsi.h"
/* check header file version for fool proof */
#if OTM8009A_DSI_H != 0x0400
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void OTM8009A_reset(void)
{

}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void OTM8009A_wr_cmd(uint8_t cmd)
{
	(void)cmd;
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void OTM8009A_wr_dat(uint8_t dat)
{	
	(void)dat;
}

/**************************************************************************/
/*! 
    Write LCD GRAM Data.
*/
/**************************************************************************/
inline void OTM8009A_wr_gram(uint16_t gram)
{	
	(void)gram;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void OTM8009A_wr_block(uint8_t *p,unsigned int cnt)
{
	(void)p;
	(void)cnt;
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void OTM8009A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	(void)x;
	(void)width;
	(void)y;
	(void)height;
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void OTM8009A_clear(void)
{

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void OTM8009A_init(void)
{
	/* Set LCD-Controller to DSI Interface */
	Display_DSIIF_Init();

	/* Flush Display */
	Display_clear_if();

#if 0	/* test code RED */
	Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_RED);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
