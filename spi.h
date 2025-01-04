//For Default C++ Functions
#include <iostream>

//Get SPI Setup, but since we are in linux, it's by the file system so, let's get file functionality
//Defines symlobic constant and types, and gives system level functions(unix standard lib?)
#include <unistd.h>

//File Descriptor Manipulator
//Int number that uniquely represents the file opened and gives function to open() and close()
#include<fcntl.h>

//To use memset() to set a specific block of memory to a value
#include<cstring>

//Maniuplates paramters in file descripter
#include <sys/ioctl.h>

//SPI Development in Linux
#include <linux/spi/spidev.h>

//GPIO Library for Some Pin Control
#include <gpiod.h>

//Settings for SPI Devices on Linux
class SPIDevice
{
    //-----------Attributes-----------//
    //Now we must setup our SPI Settings(NOT Define, since it only takes int)
    //create char pointer to address of spi device
    char* SPI_DEV;
    uint32_t SPI_SPEED;
    uint32_t SPI_MODE;
    uint8_t BITS_PER_WORD;
    uint8_t GPIO_DC;
    uint8_t GPIO_RESET;
    gpiod_chip* chipPtr;
    gpiod_line* dcPtr;
    gpiod_line* resetPtr;

    //Setup SPI File Descriptor defualt state
    int spi_fd = -1; 

    public:
    //-----------Empty Constructor-----------//
    SPIDevice() 
    {
        SPI_DEV = "/dev/spidev0.0";
        SPI_SPEED = 1;
        SPI_MODE = 0;
        BITS_PER_WORD = 8;
        GPIO_DC = 9; //Just Defualt for PI5
        GPIO_RESET = 25; //Just Default for Pi5
        chipPtr = nullptr;
        dcPtr = nullptr;
        resetPtr = nullptr;
        spi_fd = -1;

        std::cout << "SPI Device Constructor has been made" << std::endl;
    }
    //-----------Defualt SPI Functions-----------//

    //Given data char pointer and size of character, write through MOSI
    bool spiWrite(const uint8_t* data, size_t length)
    {
        //Send uint8_t data pointer to file with the size of our string
        ssize_t sentBytes = write(spi_fd,data,length);

        //If nothing was sent we got an error
        if(sentBytes < 0)
        {
            std::cerr << "SPI write error: " << strerror(errno) << std::endl;
            return false;
        }
        //If nothing was sent but it's not enough, it's partial
        if(sentBytes != length)
        {
            std::cerr << "SPI partial write: " << sentBytes << " of " << length << std::endl;
            return false;
        }
        return true;
    }


    //Set GPIOLine's to 1 or 0(For DC AND Reset)
    bool setLine(struct gpiod_line* line,int value)
    {
        if(gpiod_line_set_value(line,value) < 0)
        {
            std::cerr << "Line set Error: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    //Initlaize SPI Function
    bool spiInit()
    {
        //Open file where SPI device remains and read only
        spi_fd = open(SPI_DEV,O_WRONLY);

        //Error check if opened
        if(spi_fd < 0)
        {
            std::cerr << "Cannot open " << SPI_DEV << ": " << strerror(errno) << std::endl;
            return false;
        }
        
        //Set the spi device to the config we set above, using the spidev library
        //SPI Mode in how it shifts and reads data
        if(ioctl(spi_fd,SPI_IOC_WR_MODE,&SPI_MODE) < 0)
        {
            std::cerr << "SPI mode set failed: " << strerror(errno) << std::endl;
            return false;
        }

        //Then Bit's we are going to send to it(Write)
        if(ioctl(spi_fd,SPI_IOC_WR_BITS_PER_WORD,&BITS_PER_WORD) < 0)
        {
            std::cerr << "SPI bits/word set failed: " << strerror(errno) << std::endl;
            return false;
        }

        //Then it's speed for writing
        if(ioctl(spi_fd,SPI_IOC_WR_MAX_SPEED_HZ,&SPI_SPEED) < 0)
        {
            std::cerr << "SPI speed set failed: " << strerror(errno) << std::endl;
            return false;
        }

        return true;
    }
 
    //-----------Getters-----------//
    const char* getSPIDevice() 
    {
        return SPI_DEV;
    }

    uint32_t getSPISpeed() 
    {
        return SPI_SPEED;
    }

    uint32_t getSPIMode() 
    {
        return SPI_MODE;
    }

    uint8_t getSPIBitsPerWord() 
    {
        return BITS_PER_WORD;
    }

    // GPIO Pin Getters
    unsigned int getGPIODC() 
    {
        return GPIO_DC;
    }

    unsigned int getGPIOReset() 
    {
        return GPIO_RESET;
    }

    // GPIO Pointer Getters
    gpiod_chip* getChipPointer() 
    {
        return chipPtr;
    }

    gpiod_line* getDCPointer() 
    {
        return dcPtr;
    }

    gpiod_line* getResetPointer() 
    {
        return resetPtr;
    }

    // SPI File Descriptor Getter
    int getSPIDescriptor() 
    {
        return spi_fd;
    }

    //-----------Setters-----------//
    void setSPIDevice(char* device) {
        SPI_DEV = device;
    }

    void setSPISpeed(uint32_t speed) {
        SPI_SPEED = speed;
    }

    void setSPIMode(uint32_t mode) {
        SPI_MODE = mode;
    }

    void setSPIBitsPerWord(uint8_t bits) {
        BITS_PER_WORD = bits;
    }

    // GPIO Pin Setters
    void setGPIODC(unsigned int dc) {
        GPIO_DC = dc;
    }

    void setGPIOReset(unsigned int reset) {
        GPIO_RESET = reset;
    }

    // GPIO Pointer Setters
    bool setChipPointer(gpiod_chip* chip) {
        chipPtr = chip;
        if(chipPtr == nullptr)
        {
            std::cerr << "Error setting /dev/gpiochip0: " << strerror(errno) << std::endl;
            return false;
        }
        //Set DC pointers to GPIO PIN given previous chip
        setDCPointer(gpiod_chip_get_line(getChipPointer(),getGPIODC()));

        //Set Reset Pointers to GPIO PIN given previous chip
        setResetPointer(gpiod_chip_get_line(getChipPointer(),getGPIOReset()));
        return true;
    }

    bool setDCPointer(gpiod_line* dc) {
        dcPtr = dc;
        if(dcPtr == nullptr)
        {
            std::cerr << "Error: DC line not found\n";
            return false;
        }
        if(gpiod_line_request_output(dcPtr,"DC PIN",0) < 0)
        {
            std::cerr << "Error: cannot request DC as output\n";
            return false;
        }
        return true;
    }
    

    bool setResetPointer(gpiod_line* reset) {
        resetPtr = reset;
        if(resetPtr == nullptr)
        {
            std::cerr <<"Error: Cannot get Reset Line"<< strerror(errno)<<std::endl;
            return false;
        }
        if(gpiod_line_request_output(resetPtr,"Reset PIN",0) < 0)
        {
            std::cerr << "Error: cannot request RESET line\n";
            return false;
        }
        return true;
    }

    // SPI File Descriptor Setter
    void setSPIDescriptor(int descriptor) {
        spi_fd = descriptor;
    }

    //-----------Destructor-----------//
    ~SPIDevice()
    {
        //Close file descriptor
        if (spi_fd >= 0) 
        {
            close(spi_fd);
        }
        else
        {
            std::cerr << "Cannot Close SPI Device"<< ": "<< strerror(errno)<<std::endl;
        }

        //Release GPIO Pins and chip from control
        if(this->dcPtr!=nullptr)
        {
            gpiod_line_release(dcPtr);
        }
        if(this->resetPtr != nullptr)
        {
            gpiod_line_release(resetPtr);
        }
        if(this->chipPtr !=nullptr)
        {
            gpiod_chip_close(chipPtr);
        }

        //Remove Dangling pointer
        dcPtr = nullptr;
        resetPtr = nullptr;
        chipPtr = nullptr;

        std::cout << "SPI Device Destructor has been called" << std::endl;

    };
};

//Now the OLED Display is a SPIDevice
class Display: public SPIDevice
{
        //Font Display for Resolution
        uint8_t font5x7[95][5] = 
        {
            {0x00,0x00,0x00,0x00,0x00}, // 32 ' '
            {0x00,0x00,0x5F,0x00,0x00}, // 33 '!'
            {0x00,0x07,0x00,0x07,0x00}, // 34 '"'
            {0x14,0x7F,0x14,0x7F,0x14}, // 35 '#'
            {0x24,0x2A,0x7F,0x2A,0x12}, // 36 '$'
            {0x23,0x13,0x08,0x64,0x62}, // 37 '%'
            {0x36,0x49,0x55,0x22,0x50}, // 38 '&'
            {0x00,0x05,0x03,0x00,0x00}, // 39 '''
            {0x00,0x1C,0x22,0x41,0x00}, // 40 '('
            {0x00,0x41,0x22,0x1C,0x00}, // 41 ')'
            {0x14,0x08,0x3E,0x08,0x14}, // 42 '*'
            {0x08,0x08,0x3E,0x08,0x08}, // 43 '+'
            {0x00,0x50,0x30,0x00,0x00}, // 44 ','
            {0x08,0x08,0x08,0x08,0x08}, // 45 '-'
            {0x00,0x60,0x60,0x00,0x00}, // 46 '.'
            {0x20,0x10,0x08,0x04,0x02}, // 47 '/'
            {0x3E,0x51,0x49,0x45,0x3E}, // 48 '0'
            {0x00,0x42,0x7F,0x40,0x00}, // 49 '1'
            {0x42,0x61,0x51,0x49,0x46}, // 50 '2'
            {0x21,0x41,0x45,0x4B,0x31}, // 51 '3'
            {0x18,0x14,0x12,0x7F,0x10}, // 52 '4'
            {0x27,0x45,0x45,0x45,0x39}, // 53 '5'
            {0x3C,0x4A,0x49,0x49,0x30}, // 54 '6'
            {0x01,0x71,0x09,0x05,0x03}, // 55 '7'
            {0x36,0x49,0x49,0x49,0x36}, // 56 '8'
            {0x06,0x49,0x49,0x29,0x1E}, // 57 '9'
            {0x00,0x36,0x36,0x00,0x00}, // 58 ':'
            {0x00,0x56,0x36,0x00,0x00}, // 59 ';'
            {0x08,0x14,0x22,0x41,0x00}, // 60 '<'
            {0x14,0x14,0x14,0x14,0x14}, // 61 '='
            {0x00,0x41,0x22,0x14,0x08}, // 62 '>'
            {0x02,0x01,0x51,0x09,0x06}, // 63 '?'
            {0x32,0x49,0x79,0x41,0x3E}, // 64 '@'
            {0x7E,0x11,0x11,0x11,0x7E}, // 65 'A'
            {0x7F,0x49,0x49,0x49,0x36}, // 66 'B'
            {0x3E,0x41,0x41,0x41,0x22}, // 67 'C'
            {0x7F,0x41,0x41,0x22,0x1C}, // 68 'D'
            {0x7F,0x49,0x49,0x49,0x41}, // 69 'E'
            {0x7F,0x09,0x09,0x09,0x01}, // 70 'F'
            {0x3E,0x41,0x49,0x49,0x7A}, // 71 'G'
            {0x7F,0x08,0x08,0x08,0x7F}, // 72 'H'
            {0x00,0x41,0x7F,0x41,0x00}, // 73 'I'
            {0x20,0x40,0x41,0x3F,0x01}, // 74 'J'
            {0x7F,0x08,0x14,0x22,0x41}, // 75 'K'
            {0x7F,0x40,0x40,0x40,0x40}, // 76 'L'
            {0x7F,0x02,0x0C,0x02,0x7F}, // 77 'M'
            {0x7F,0x04,0x08,0x10,0x7F}, // 78 'N'
            {0x3E,0x41,0x41,0x41,0x3E}, // 79 'O'
            {0x7F,0x09,0x09,0x09,0x06}, // 80 'P'
            {0x3E,0x41,0x51,0x21,0x5E}, // 81 'Q'
            {0x7F,0x09,0x19,0x29,0x46}, // 82 'R'
            {0x46,0x49,0x49,0x49,0x31}, // 83 'S'
            {0x01,0x01,0x7F,0x01,0x01}, // 84 'T'
            {0x3F,0x40,0x40,0x40,0x3F}, // 85 'U'
            {0x1F,0x20,0x40,0x20,0x1F}, // 86 'V'
            {0x3F,0x40,0x38,0x40,0x3F}, // 87 'W'
            {0x63,0x14,0x08,0x14,0x63}, // 88 'X'
            {0x07,0x08,0x70,0x08,0x07}, // 89 'Y'
            {0x61,0x51,0x49,0x45,0x43}, // 90 'Z'
            {0x00,0x7F,0x41,0x41,0x00}, // 91 '['
            {0x02,0x04,0x08,0x10,0x20}, // 92 '\'
            {0x00,0x41,0x41,0x7F,0x00}, // 93 ']'
            {0x04,0x02,0x01,0x02,0x04}, // 94 '^'
            {0x40,0x40,0x40,0x40,0x40}, // 95 '_'
            {0x00,0x01,0x02,0x04,0x00}, // 96 '`'
            {0x20,0x54,0x54,0x54,0x78}, // 97 'a'
            {0x7F,0x48,0x44,0x44,0x38}, // 98 'b'
            {0x38,0x44,0x44,0x44,0x20}, // 99 'c'
            {0x38,0x44,0x44,0x48,0x7F}, //100 'd'
            {0x38,0x54,0x54,0x54,0x18}, //101 'e'
            {0x08,0x7E,0x09,0x01,0x02}, //102 'f'
            {0x0C,0x52,0x52,0x52,0x3E}, //103 'g'
            {0x7F,0x08,0x04,0x04,0x78}, //104 'h'
            {0x00,0x44,0x7D,0x40,0x00}, //105 'i'
            {0x20,0x40,0x44,0x3D,0x00}, //106 'j'
            {0x7F,0x10,0x28,0x44,0x00}, //107 'k'
            {0x00,0x41,0x7F,0x40,0x00}, //108 'l'
            {0x7C,0x04,0x18,0x04,0x78}, //109 'm'
            {0x7C,0x08,0x04,0x04,0x78}, //110 'n'
            {0x38,0x44,0x44,0x44,0x38}, //111 'o'
            {0x7C,0x14,0x14,0x14,0x08}, //112 'p'
            {0x08,0x14,0x14,0x18,0x7C}, //113 'q'
            {0x7C,0x08,0x04,0x04,0x08}, //114 'r'
            {0x48,0x54,0x54,0x54,0x20}, //115 's'
            {0x04,0x3F,0x44,0x40,0x20}, //116 't'
            {0x3C,0x40,0x40,0x20,0x7C}, //117 'u'
            {0x1C,0x20,0x40,0x20,0x1C}, //118 'v'
            {0x3C,0x40,0x30,0x40,0x3C}, //119 'w'
            {0x44,0x28,0x10,0x28,0x44}, //120 'x'
            {0x0C,0x50,0x50,0x50,0x3C}, //121 'y'
            {0x44,0x64,0x54,0x4C,0x44}, //122 'z'
            {0x00,0x08,0x36,0x41,0x00}, //123 '{'
            {0x00,0x00,0x7F,0x00,0x00}, //124 '|'
            {0x00,0x41,0x36,0x08,0x00}, //125 '}'
            {0x10,0x08,0x08,0x10,0x08}  //126 '~'
    };
    public:

    //-----------Display Constructor-----------//
    Display(int dcPIN, int resetPIN)
    {    
        setSPIDevice("/dev/spidev0.0");
        setSPISpeed(8000000);
        setSPIMode(0);
        setSPIBitsPerWord(8);
        setGPIODC(dcPIN);
        setGPIOReset(resetPIN);
        std::cout <<"Display Constructor has been made"<<std::endl;
    };

    //-----------Display Functions-----------//

    //Send Commands for Display Config before sending info(FOR FLIPPED CONFIGRATION on breadboard)
    bool oledInit()
    { 
        //Hardware Reset:
        setLine(getResetPointer(), 0);
        usleep(50000);  // Increase delay to 50ms.
        setLine(getResetPointer(), 1);
        usleep(50000);  // Increase delay to 50ms.

        // Typical init sequence for SH1106 (128x64)
        // (Similar to SSD1306, but may differ in column offset usage)
        uint8_t initCmds[] = 
        {
            0xAE,       // Display off
            0xD5, 0x80, // clock div
            0xA8, 0x3F, // multiplex ratio = 1/64
            0xD3, 0x00, // display offset
            0x40,       // start line = 0
            0xA0,       // segment remap(Flipped due to my segment being upside down)
            0xC0,       // com scan direction (Flipped due to my segment being upside down)
            0xDA, 0x12, // com pins
            0x81, 0x7F, // contrast
            0xA4,       // resume
            0xA6,       // normal display
            0xD9, 0x22, // pre-charge
            0xDB, 0x20, // Vcom detect
            0x8D, 0x14, // charge pump
            0xAF        // Display on
        };

        //Send all these commands to the SPI devices
        for(auto c: initCmds)
        {
            //If we can write these commands, keep going, if not return false
            if(oledWriteCMD(c)==false)
            {
                return false;
            }
            // Small delay (100 microseconds) after each command for processing time
            usleep(100);  
        }

        return true;
    }
    
    // Draws a string starting at (x, page)
    void oledDrawString(int x, int page, const char* str) 
    {
        //While we iterate through string and it's valid
        while(*str) 
        {
            if(*str == '\n') 
            {
                page++;
                x = 0;
                str++;
                // Prevent exceeding display height
                if(page >= 8)
                {
                    break;
                } 
                continue;
            }
            //Draw first character of string, onto display x=column , page is the y
            oledDrawChar(x, page, *str++);

            // Move by 6 columns (5 for the character + 1 for spacing)
            x += 6; 

            //Prevent drawing beyond display width and go down under
            if(x >= 128) 
            {
                page++;
                x=0;
                //Last page for display break
                if(page>= 8)
                {
                    break;
                }
            }; 
        }
    };

    //Clear Oled Display
    void oledClear() 
    {
        // The SH1106 often starts at column 2 but we will start at first just in case
        for(int page=0; page<8; page++) 
        {
            //Clear Display
            oledWriteCMD(0xB0 + page);
            // lower col
            oledWriteCMD(0x02); 
            // higher col
            oledWriteCMD(0x10); 

            // Write 128 columns of 0
            for(int col=0; col<128; col++) 
            {
                oledWriteData(0x00);
            }
        }
    }
    private:
    // Writes a single character at (x, page)
    void oledDrawChar(int x, int page, char c) 
    {
        // Ensure character is within range
        // Default to space if out of range
        if(c < 32 || c > 126) c = 32; 

        // Set the page address
        oledWriteCMD(0xB0 + page);

        // Set the column address with a 2-pixel offset for our Display
        int realX = x + 2; 

        // Lower nibble
        oledWriteCMD((realX & 0x0F));  

        // Higher nibble
        oledWriteCMD(((realX >> 4) & 0x0F) | 0x10);  

        // Fetch the glyph
        const uint8_t* glyph = font5x7[c - 32];

        // Write the glyph data (5 columns)
        for(int i = 0; i < 5; i++)
        {
            oledWriteData(glyph[i]);
        }

        // Add a 1-column space fr next message
        oledWriteData(0x00);
    };

    //Set mode of display(command or data by flipping DC to 1 or 0)
    bool commandMode()
    {
        return setLine(this->getDCPointer(),0);
    }
    bool dataMode()
    {
        return setLine(this->getDCPointer(),1);
    }

    //Function to write oled command
    bool oledWriteCMD(uint8_t c)
    {
        //If we are in command mode
        if(commandMode()==true)
        {
            //Return the status of writing the bytes to the SPIDevice
            return spiWrite(&c,1);
        }
        return false;
    }

    //Function to Write actual data
    bool oledWriteData(uint8_t data)
    {
        //If we set it to data mode, write our data and send our char
        if(dataMode()==true)
        {
            spiWrite(&data,1);
            return true;
        }
        return false;
    }

};