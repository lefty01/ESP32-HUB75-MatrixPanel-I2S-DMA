/*************************************************************************
 * Description: 
 * 
 * The underlying implementation of the ESP32-HUB75-MatrixPanel-I2S-DMA only
 * supports output to 1/16 or 1/32 scan panels - which means outputting 
 * two lines at the same time, 16 or 32 rows apart. This cannot be changed
 * at the DMA layer as it would require a messy and complex rebuild of the 
 * library's DMA internals.
 *
 * However, it is possible to connect 1/8 scan panels to this same library and
 * 'trick' the output to work correctly on these panels by way of adjusting the
 * pixel co-ordinates that are 'sent' to the ESP32-HUB75-MatrixPanel-I2S-DMA
 * library.
 * 
 **************************************************************************/
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"

// Virtual Display to re-map co-ordinates such that they draw correctly on a 32x16 1/4 Scan panel
#include "1_8_ScanPanel.h" 


  // Panel configuration
  #define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module. 
  #define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.
  
  
  #define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
  #define NUM_COLS 2 // Number of INDIVIDUAL PANELS per ROW
  
  // ^^^ NOTE: DEFAULT EXAMPLE SETUP IS FOR A CHAIN OF TWO x 1/8 SCAN PANELS
    
  // Change this to your needs, for details on VirtualPanel pls read the PDF!
  #define SERPENT true
  #define TOPDOWN false
  
  // placeholder for the matrix object
  MatrixPanel_I2S_DMA *dma_display = nullptr;

  // placeholder for the virtual display object
  OneEightScanPanel  *OneEightMatrixDisplay = nullptr;
  
  /******************************************************************************
   * Setup!
   ******************************************************************************/
  void setup()
  {
    delay(250);
   
    Serial.begin(115200);
    Serial.println(""); Serial.println(""); Serial.println("");
    Serial.println("*****************************************************");
    Serial.println("*         1/8 Scan Panel Demonstration              *");
    Serial.println("*****************************************************");
  
/* 
     // 62x32 1/8 Scan Panels don't have a D and E pin!
	 
	 HUB75_I2S_CFG::i2s_pins _pins = {
      R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, 
      A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, 
      LAT_PIN, OE_PIN, CLK_PIN
	 };
*/
  	HUB75_I2S_CFG mxconfig(
  				PANEL_RES_X*2,   	        // DO NOT CHANGE THIS
  				PANEL_RES_Y/2,   	        // DO NOT CHANGE THIS
  				NUM_ROWS*NUM_COLS     		// DO NOT CHANGE THIS
  				//,_pins			// Uncomment to enable custom pins
  	);
    
    mxconfig.clkphase = false; // Change this if you see pixels showing up shifted wrongly by one column the left or right.
    
  	//mxconfig.driver   = HUB75_I2S_CFG::FM6126A;     // in case that we use panels based on FM6126A chip, we can set it here before creating MatrixPanel_I2S_DMA object
  
  	// OK, now we can create our matrix object
  	dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  
  	// let's adjust default brightness to about 75%
  	dma_display->setBrightness8(96);    // range is 0-255, 0 - 0%, 255 - 100%
  
  	// Allocate memory and start DMA display
  	if( not dma_display->begin() )
  	  Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

   
    dma_display->clearScreen();
    delay(500);
    
  	// create OneEightMatrixDisplaylay object based on our newly created dma_display object
  	OneEightMatrixDisplay = new OneEightScanPanel((*dma_display), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, SERPENT, TOPDOWN);

  }

  
  void loop() {

      // What the panel sees from the DMA engine!
      for (int i=PANEL_RES_X*2+10; i< PANEL_RES_X*(NUM_ROWS*NUM_COLS)*2; i++)
      {
        dma_display->drawLine(i, 0, i, 7, dma_display->color565(255, 0, 0)); // red
        delay(10);
      }
          
      dma_display->clearScreen();
      delay(1000);
/*
      // Try again using the pixel / dma memory remapper
      for (int i=PANEL_RES_X+5; i< (PANEL_RES_X*2)-1; i++)
      {
        OneEightMatrixDisplay->drawLine(i, 0, i, 7, dma_display->color565(0, 0, 255)); // blue    
        delay(10);
      } 
*/

      // Try again using the pixel / dma memory remapper
      int offset = PANEL_RES_X*((NUM_ROWS*NUM_COLS)-1);
      for (int i=0; i< PANEL_RES_X; i++)
      {
        OneEightMatrixDisplay->drawLine(i+offset, 0, i+offset, 7, dma_display->color565(0, 0, 255)); // blue
        OneEightMatrixDisplay->drawLine(i+offset, 8, i+offset, 15, dma_display->color565(0, 128,0)); // g        
        OneEightMatrixDisplay->drawLine(i+offset, 16, i+offset, 23, dma_display->color565(128, 0,0)); // red
        OneEightMatrixDisplay->drawLine(i+offset, 24, i+offset, 31, dma_display->color565(0, 128, 128)); // blue        
        delay(10);
      } 

      delay(1000);


      // Print on each chained panel 1/8 module!
      // This only really works for a single horizontal chain
      for (int i = 0; i < NUM_ROWS*NUM_COLS; i++)
      {
        OneEightMatrixDisplay->setTextColor(OneEightMatrixDisplay->color565(255, 255, 255));
        OneEightMatrixDisplay->setCursor(i*PANEL_RES_X+7, OneEightMatrixDisplay->height()/3); 
      
        // Red text inside red rect (2 pix in from edge)
        OneEightMatrixDisplay->print("Panel " + String(i+1));
        OneEightMatrixDisplay->drawRect(1,1, OneEightMatrixDisplay->width()-2, OneEightMatrixDisplay->height()-2, OneEightMatrixDisplay->color565(255,0,0));
      
        // White line from top left to bottom right
        OneEightMatrixDisplay->drawLine(0,0, OneEightMatrixDisplay->width()-1, OneEightMatrixDisplay->height()-1, OneEightMatrixDisplay->color565(255,255,255));
      }

      delay(2000);
      dma_display->clearScreen();
  
  } // end loop
