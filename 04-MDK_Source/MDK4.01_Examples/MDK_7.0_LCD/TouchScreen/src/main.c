/**************************************************************************************
* File Name          : main.c
* Date First Issued  : 01/10/2010
* Description        : Main program body
***************************************************************************************
***************************************************************************************
* History:
* 01/10/2010:          V1.0
**************************************************************************************/

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <board.h>
#include <board_memories.h>
#include <pio/pio.h>
#include <dbgu/dbgu.h>
#include <tsd/tsd.h>
#include <lcd/lcdd.h>
#include <lcd/draw.h>
#include <lcd/color.h>
#include <stdio.h>
#include <string.h>
#include <utility/trace.h>
#include <utility/bmp.h>
#include <irq/irq.h>
#include <dma/dma.h>
#include <drivers/dmad/dmad.h>
#if defined(cortexm3)
#include <systick/systick.h>
#endif

//------------------------------------------------------------------------------
//         Local definition
//------------------------------------------------------------------------------
///  Base address which to store the decoded image
#define IMAGE_BASE_ADDRESS        (AT91C_DDR2 + 0x0200000)
/// Address at which to display screen saver.
#define DISPLAY_ADDRESS           (AT91C_DDR2 + 0x0300000)
/// DMA transfer WORD width.
#define DMA_TRANSFER_WIDTH      2
/// Numbers of color channel of image (RGB).
#define PICTURE_COLOR_CHANNEL   3
/// Width of LCD screen.
#define IMAGE_WIDTH             (BOARD_LCD_WIDTH * PICTURE_COLOR_CHANNEL)
/// Height of LCD screen.
#define IMAGE_HEIGHT            BOARD_LCD_HEIGHT
/// Calculate the number of width transfers.
#define DMA_TRANSFER_SIZE(size)  ((size) >> DMA_TRANSFER_WIDTH)

//------------------------------------------------------------------------------
//         Local types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

 /// LCD buffer.
static unsigned char *pLcdBuffer = (unsigned char *) (DISPLAY_ADDRESS);

#if defined(cortexm3)
/// Global timestamp in milliseconds since start of application.
volatile unsigned int timestamp = 0;
#endif

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------
#if defined(cortexm3)
//------------------------------------------------------------------------------
/// Handler for SysTick interrupt. Increments the timestamp counter.
//------------------------------------------------------------------------------
void SysTick_Handler(void)
{
    timestamp++;

    // Call TSD_TimerHandler per 10ms
    if ((timestamp % 10) == 0) {
        
        TSD_TimerHandler();
    }
}

//------------------------------------------------------------------------------
/// Delay millisecond. Use SysTick interrupt for function implement, so ensure
/// the SysTick interrupt is enabled before using this function.
/// \param ms   millisecond to be delay
//------------------------------------------------------------------------------
void DelayMS(unsigned int ms)
{
    unsigned int st = timestamp;

    while (timestamp - st < ms);
}
#endif

//------------------------------------------------------------------------------
/// Callback called when the pen is pressed on the touchscreen
/// \param x horizontal position (in pixel if ts calibrated)
/// \param y vertical position (in pixel if ts calibrated)
//------------------------------------------------------------------------------
void TSD_PenPressed(unsigned int x, unsigned int y)
{
    printf("Pen pressed at  (%03u, %03u)\n\r", x, y);
}

//------------------------------------------------------------------------------
/// Callback called when the pen is moved on the touchscreen
/// \param x horizontal position (in pixel if ts calibrated)
/// \param y vertical position (in pixel if ts calibrated)
//------------------------------------------------------------------------------
void TSD_PenMoved(unsigned int x, unsigned int y)
{
    printf("Pen moved at    (%03u, %03u)\n\r", x, y);
}

//------------------------------------------------------------------------------
/// Callback called when the touchscreen is released on the touchscreen
/// \param x horizontal position (in pixel if ts calibrated)
/// \param y vertical position (in pixel if ts calibrated)
//------------------------------------------------------------------------------
void TSD_PenReleased(unsigned int x, unsigned int y)
{
    printf("Pen released at (%03u, %03u)\n\r", x, y);
}

//------------------------------------------------------------------------------
/// Initializes the touchscreen and outputs measurements on the DBGU.
//------------------------------------------------------------------------------
int main(void)
{
    unsigned int bResult;

    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic Touchscreen Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

#if (!defined(sdram) && !defined(cortexm3))
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);
#endif

#if (!defined(ddram) && defined(AT91C_DDR2))
    BOARD_ConfigureDdram(DDR_MICRON_MT47H64M8, BOARD_DDRAM_BUSWIDTH);
#endif

#if defined(cortexm3)
    // Configuration 1ms tick
    SysTick_Configure(1, BOARD_MCK/1000, SysTick_Handler);
#endif

    // Initialize LCD
    LCDD_Initialize();
    LCDD_Fill(pLcdBuffer, COLOR_DARKCYAN);
#ifdef BOARD_LCD_HX8347   
    LCDD_Start();
#else    // peripheral LCDC
    LCDD_DisplayBuffer(pLcdBuffer);
#endif

	// Calibration setup
    LCDD_Fill(pLcdBuffer, COLOR_WHITE);
    LCDD_DrawString(pLcdBuffer, 30, 50, "LCD calibration", COLOR_BLACK);
    LCDD_DrawString(pLcdBuffer, 1, 100, " Touch the dots to\ncalibrate the screen", COLOR_DARKBLUE);
	LCDD_DisplayBuffer(pLcdBuffer);

    // Initialize touchscreen without calibration
    TSD_Initialize(0);

    while(1) {
        bResult = TSD_Calibrate(pLcdBuffer);
        if(bResult) {
            printf("-I- Calibration successful !\n\r");
            break;
        }
        else {
            printf("-E- Error too big ! Retry...\n\r");
        }
    }

    // Infinite loop
    while (1);
}

