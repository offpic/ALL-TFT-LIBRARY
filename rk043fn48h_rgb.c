/********************************************************************************/
/*!
	@file			rk043fn48h_rgb.c
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

/* Includes ------------------------------------------------------------------*/
#include "rk043fn48h_rgb.h"
/* check header file version for fool proof */
#if RK043FN48H_RGB_H != 0x0200
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
inline void RK043FN48H_reset(void)
{

}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void RK043FN48H_wr_cmd(uint8_t cmd)
{

}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void RK043FN48H_wr_dat(uint8_t dat)
{	

}

/**************************************************************************/
/*! 
    Write LCD GRAM Data.
*/
/**************************************************************************/
inline void RK043FN48H_wr_gram(uint16_t gram)
{	

}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void RK043FN48H_wr_block(uint8_t *p,unsigned int cnt)
{

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void RK043FN48H_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void RK043FN48H_clear(void)
{

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void RK043FN48H_init(void)
{
	/* Set LCD-Controller to RGB Interface */
	Display_RGBIF_Init();

	/* Flush Display */
	Display_clear_if();

#if 0	/* test code RED */
	Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_RED);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
