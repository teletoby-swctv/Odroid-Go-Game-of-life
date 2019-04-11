/*
 Conway's Game of Life
 https://de.wikipedia.org/wiki/Conways_Spiel_des_Lebens
 The Game of Life, also known simply as Life,
 is a cellular automaton devised by the British mathematician John Horton Conway in 1970.

The game is a zero-player game, meaning that its evolution is determined by its initial state,
requiring no further input. One interacts with the Game of Life by creating an initial configuration
and observing how it evolves, or, for advanced players, by creating patterns with particular properties.

original by:
https://github.com/wegfawefgawefg/LifeSouperWatch

ODROID-GO version by teletoby
https://github.com/teletoby-swctv/Odroid-Go-Game-of-life

 */


#include <odroid_go.h>

#include <driver/adc.h>              // Battery
#include <esp_adc_cal.h>
#define RESISTANCE_NUM    2
#define DEFAULT_VREF      1100
#define NO_OF_SAMPLES     64
static esp_adc_cal_characteristics_t adc_chars;

#define FIELD_WIDTH 40
#define SQUARE_SIZE 240 / FIELD_WIDTH
#define NUM_FIELD_BYTES (FIELD_WIDTH * FIELD_WIDTH) / 8

int Round;
char lr[20];
//  allocate memory for the fields
unsigned char fieldA[NUM_FIELD_BYTES];
unsigned char fieldB[NUM_FIELD_BYTES];

//  make a pointer to current field and nextField
unsigned char *currentField;
unsigned char *nextField;
unsigned char *swapField;



void setup() {
  Serial.begin(115200);
  Serial.println("Odroid-Go Game of life");
  Serial.println("https://github.com/teletoby-swctv/Odroid-Go-Game-of-life");


    // Turn on backlight 
   pinMode(14, OUTPUT);
   digitalWrite(14, HIGH);

  //  set the pointer to the current field
  currentField = fieldA;
  nextField = fieldB;

  //  fill the field with random values
  for( unsigned char i = 0; i < NUM_FIELD_BYTES; i++ )
  {
    currentField[i] = random( 256 );
  }

  GO.begin();
  GO.Speaker.setVolume(1);
  //GO.lcd.setRotation(3);
  GO.lcd.clearDisplay();

  GO.lcd.setTextSize(1);
  GO.lcd.setTextFont(2);
  GO.lcd.setTextColor(LIGHTGREY);
  GO.lcd.setCursor(245, 10);
  GO.lcd.setTextSize(2);
  GO.lcd.println(" Game");
  GO.lcd.setCursor(258, 40);
  GO.lcd.println(" of");
  GO.lcd.setCursor(250, 70);
  GO.lcd.println(" life");

  GO.lcd.setTextSize(2);
  GO.lcd.setTextFont(1);
  GO.lcd.setCursor(240, 120);
  GO.lcd.println(" Round");
  
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);
  showBatteryVoltage(readBatteryVoltage());
}

void loop() {
  GO.Speaker.setVolume(1);
  GO.update();
  GO.Speaker.setVolume(1);
  if (GO.BtnB.wasPressed()){
    int n = (random(NUM_FIELD_BYTES))-10;
    for( unsigned char i = n; i < n+3; i++ )
    {
    currentField[i] = currentField[i]+random(150);
    Round=0;
    showBatteryVoltage(readBatteryVoltage());
    //GO.Speaker.tone(200, 20);
    }
  }

    if (GO.BtnA.wasPressed()){
    for( unsigned char i = 0; i < NUM_FIELD_BYTES; i++ )
    {
    currentField[i] = currentField[i]+random(random(256));
    Round=0;
    //GO.Speaker.tone(200, 20);
    }
  }
  //  draw the field
  drawField();

  //  apply conways game of life
  applyLogic();
  
  //  swap the fields
  swapFields();
}


//  function to get the value of a bit based on x and y
boolean getBit( unsigned char x, unsigned char y )
{
  //  get the bit index for a one dimensional array
  unsigned int index = x + y * FIELD_WIDTH;
  //  get which char the bit is in
  unsigned char charNumber = index / 8;
  //  get which bit of 8 in the single char is the one we're looking for
  unsigned char charSubIndex = index - (8 * charNumber);
  //  get the bit value
  unsigned char bitChar = currentField[charNumber];
  boolean bitValue =  bitRead( bitChar, charSubIndex );  //( bitChar >> charSubIndex ) & 00000001 ;

  return bitValue;
}

//  function to set the value of a bit based on x and y
void setBit( unsigned char x, unsigned char y, boolean newBitValue )
{
  //  get the bit index for a one dimensional array
  unsigned int index = x + y * FIELD_WIDTH;
  //  get which char the bit is in
  unsigned char charNumber = index / 8;
  //  get which bit of 8 in the single char is the one we're looking for
  unsigned char charSubIndex = index - (8 * charNumber);
  //  get the bit value
  unsigned char bitChar = nextField[charNumber];

  if( newBitValue == true )
  {
    //  create the mask
    byte mask = 00000001 << charSubIndex;
    //  apply the mask
    nextField[charNumber] = bitChar | mask ;
  }
  else
  {
    //  create the negative mask
    byte mask = ~( 00000001 << charSubIndex);
    //  apply the mask
    nextField[charNumber] = bitChar & mask ;
  }
}

//  function to draw the field
void drawField()
{
  unsigned char squareX = 0;
  unsigned char squareY = 0;
  for( unsigned char y = 0; y < FIELD_WIDTH; y++ )
  {
    squareX = 0;
    for( unsigned char x = 0; x < FIELD_WIDTH; x++ )
    {
      
      boolean bitValue = getBit( x, y );
     
      GO.lcd.fillRect( squareX+1, squareY+1, SQUARE_SIZE-2, SQUARE_SIZE-2, bitValue*YELLOW ); // bitValue*random(200,55536)

      //Serial.print(bitValue);
      squareX += SQUARE_SIZE;
    }
    squareY += SQUARE_SIZE;
  }
        Round=Round+1;
        sprintf(lr,"%6d", Round);
        GO.lcd.fillRect( 240, 145, 80, 30, 0 );
        GO.lcd.setCursor(240, 150);
        GO.lcd.setTextSize(2);
        GO.lcd.println(lr);
        if (Round>9999 || GO.BtnStart.wasPressed()){
          delay(3000);
          ESP.restart();
        }
}

//  function to swap the field pointers
void swapFields()
{
  swapField = currentField;
  currentField = nextField;
  nextField = swapField;
}

//  apply cellular automota logic to entire field
void applyLogic()
{
  
  //  go through each cell
  for( signed char y = 0; y < FIELD_WIDTH; y++ )
  {
    for( signed char x = 0; x < FIELD_WIDTH; x++ )
    {
      unsigned char neighborTotal = 0;
      
      //  total the number of alive neighbors
      //  //  check top left neighbor
      if( checkUpperBounds( x - 1, y - 1 ) )
      {
        if( getBit( checkWrapAround( x - 1 ), checkWrapAround( y - 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check upper neighbor
      if( checkUpperBounds( x , y - 1 ) )
      {
        if( getBit( checkWrapAround( x ), checkWrapAround( y - 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check top right neighbor
      if( checkUpperBounds( x + 1, y - 1 ) )
      {
        if( getBit( checkWrapAround( x + 1 ), checkWrapAround( y - 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check left neighbor
      if( checkUpperBounds( x - 1, y ) )
      {
        if( getBit( checkWrapAround( x - 1 ), checkWrapAround( y )) )
        {
          neighborTotal++;
        }
      }
      //  //  check right neighbor
      if( checkUpperBounds( x + 1, y ) )
      {
        if( getBit( checkWrapAround( x + 1 ), checkWrapAround( y )) )
        {
          neighborTotal++;
        }
      }
      //  //  check bottom left neighbor
      if( checkUpperBounds( x - 1, y + 1 ) )
      {
        if( getBit( checkWrapAround( x - 1 ), checkWrapAround( y + 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check lower neighbor
      if( checkUpperBounds( x, y + 1 ) )
      {
        if( getBit( checkWrapAround( x ), checkWrapAround( y + 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check bottom right neighbor
      if( checkUpperBounds( x + 1, y + 1 ) )
      {
        if( getBit( checkWrapAround( x + 1 ), checkWrapAround( y + 1 )) )
        {
          neighborTotal++;
        }
      }
      //  check rule
      boolean cellValue = getBit( x, y );
      boolean nextCellValue = false;
      if( cellValue ) //  if cell is currently alive
      {
        //  2 or 3 neighbors lives on
        if( neighborTotal == 2 || neighborTotal == 3 )
        {
          nextCellValue = true;  
        }
        //  fewer than 2 neighbors means death
        //  more than 3 neighbors means death
        else
        {
          nextCellValue = false;
        }
        
      }
      else  //  if cell is currently dead
      {
        //  3 neighbors becomes alive
        if( neighborTotal == 3 )
        {
          nextCellValue = true;  
        }
        else
        {
          nextCellValue = false;  
        }
      }

      //  set next state in next field frame
      setBit( x, y, nextCellValue );
    }
  }
}


//  wrap around coordinates
unsigned char checkWrapAround( signed char a )
{
  if( a < 0 )
  {
    return FIELD_WIDTH + a;
  }
  else
  {
    return a;
  }
}

//  check upper bounds
boolean checkUpperBounds( signed char x, signed char y )
{
    if( (x < FIELD_WIDTH) && (y < FIELD_WIDTH) )
    {
      return true;  
    }
    else
    {
      return false;
    }
}

double readBatteryVoltage() {
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t) ADC1_CHANNEL_0);
    }
    adc_reading /= NO_OF_SAMPLES;

    return (double) esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars) * RESISTANCE_NUM / 1000;
}

void showBatteryVoltage(double voltage) {
    //GO.lcd.clear();
    GO.lcd.fillRect( 280, 0, 40, 10, 0 );
    GO.lcd.setCursor(280, 0);
    GO.lcd.setTextSize(1);
    GO.lcd.printf("%1.2lf V \n", voltage);
}
