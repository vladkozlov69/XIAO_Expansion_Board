#include <Arduino.h>
#include <U8x8lib.h>
#include "RTClib.h"
#include <SPI.h>  
#include <SD.h> 

#define BUTTON_PIN   1
#define SD_CS_PIN    2
#define BUZZER_PIN   3
#define SPI_CLK_PIN  8
#define MISO_PIN     9
#define MOSI_PIN    10

RTC_PCF8563 rtc;

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);


// set up variables using the SD utility library functions:  
Sd2Card card;  
SdVolume volume;  
SdFile root;  

unsigned long screen_update_timer = millis();

void setup(void) 
{
    Serial.begin(115200);
    while (!Serial) 
    {  
        ; // wait for serial port to connect. Needed for native USB port only  
    } 

    // display
    u8x8.begin();
    u8x8.setFlipMode(1);   
    u8x8.setFont(u8x8_font_chroma48medium8_r);

    // initialize the LED pin as an output:
    pinMode(LED_BUILTIN, OUTPUT);
    // initialize the pushbutton pin as an input:
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(BUZZER_PIN, OUTPUT);

    if (! rtc.begin()) 
    {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        abort();
    }

    if (rtc.lostPower()) 
    {
        Serial.println("RTC is NOT initialized, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    rtc.start();

    Serial.print("\nInitializing SD card...");  

    // we'll use the initialization code from the utility libraries  
    // since we're just testing if the card is working!  
    if (!card.init(SPI_HALF_SPEED, SD_CS_PIN)) 
    {  
        Serial.println("initialization failed. Things to check:");  
        Serial.println("* is a card inserted?");  
        Serial.println("* is your wiring correct?");  
        Serial.println("* did you change the chipSelect pin to match your shield or module?");  
    } 
    else 
    {  
        // print the type of card  
        Serial.println();  
        Serial.print("Card type:         ");  
        switch (card.type()) 
        {  
            case SD_CARD_TYPE_SD1:  
                Serial.println("SD1");  
                break;  
            case SD_CARD_TYPE_SD2:  
                Serial.println("SD2");  
                break;  
            case SD_CARD_TYPE_SDHC:  
                Serial.println("SDHC");  
                break;  
            default:  
                Serial.println("Unknown");  
        }  

        // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32  
        if (!volume.init(card)) 
        {  
            Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");   
        }  
        else
        {
            Serial.print("Clusters:          ");  
            Serial.println(volume.clusterCount());  
            Serial.print("Blocks x Cluster:  ");  
            Serial.println(volume.blocksPerCluster());  

            Serial.print("Total Blocks:      ");  
            Serial.println(volume.blocksPerCluster() * volume.clusterCount());  
            Serial.println();  

            // print the type and size of the first FAT-type volume  
            uint32_t volumesize;  
            Serial.print("Volume type is:    FAT");  
            Serial.println(volume.fatType(), DEC);  

            volumesize = volume.blocksPerCluster();    // clusters are collections of blocks  
            volumesize *= volume.clusterCount();       // we'll have a lot of clusters  
            volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)  
            Serial.print("Volume size (Kb):  ");  
            Serial.println(volumesize);  
            Serial.print("Volume size (Mb):  ");  
            volumesize /= 1024;  
            Serial.println(volumesize);  
            Serial.print("Volume size (Gb):  ");  
            Serial.println((float)volumesize / 1024.0);  

            Serial.println("\nFiles found on the card (name, date and size in bytes): ");  
            root.openRoot(volume);  

            // list all files in the card with date and size  
            root.ls(LS_R | LS_DATE | LS_SIZE); 
        }
    }
}
 
void loop(void) 
{
    // check if the pushbutton is pressed. If it is, the buttonState is LOW:
    if (digitalRead(BUTTON_PIN) == LOW) 
    {
        // turn LED off:
        digitalWrite(LED_BUILTIN, LOW);

        for (long i = 0; i < 40; i++) 
        {
            digitalWrite(BUZZER_PIN, HIGH);
            delayMicroseconds(250);
            digitalWrite(BUZZER_PIN, LOW);
            delayMicroseconds(250);
        }
    } 
    else 
    {
        // turn LED on:
        digitalWrite(LED_BUILTIN, HIGH);
    }

    if (millis() - screen_update_timer >= 1000)
    {
        DateTime nowTime = rtc.now();
        u8x8.setCursor(0, 0);
        u8x8.print(nowTime.day());
        u8x8.print("/");
        u8x8.print(nowTime.month());
        u8x8.print("/");
        u8x8.print(nowTime.year());
        u8x8.setCursor(0, 1);
        u8x8.print(nowTime.hour());
        u8x8.print(":");
        u8x8.print(nowTime.minute());
        u8x8.print(":");
        u8x8.println(nowTime.second());

        screen_update_timer = millis();
    }
}