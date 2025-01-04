/************************************************************************
 * oled.cpp
 * 
 * A minimal user print and cpu stats program for an SH1106 (128x64) OLED on Raspberry Pi 5
 *   - SPI: /dev/spidev0.0
 *   - DC (MISO) -> GPIO9 (Pin letting the board know if the incoming data is a command or Data to dipslay)
 *   - CS -> CE0 (Chip Select)
 *   - SCLK -> GPIO11 (Source Clock)
 *   - MOSI -> GPIO10(Master output slave input pin)
 *   - RESET -> GPIO25 (Reset clears the buffer)
 *  Note: The CPU Usage runs mpstat commands so make sure to install that package
 * Build:
 *   g++ -o oled oled.cpp -lgpiod
 * Run:
 *   sudo ./oled
 ************************************************************************/

//For Default C++ Functions
#include <iostream>

//For spi display library
#include "spi.h"

//Implement smart pointers
#include <memory>  

//Implement multithreading(fetching cpu info and displaying data)
#include <thread>

//Strings for getline in cin and choosing a custom delimeter
#include<string>

//My custom systemStats command
#include "systemStats.h"

//Note after doing gpioinfo, we can see these lines are under gpiochip0 and are active high
int main()
{
    //Create display object with default Display and spi values, given GPIO Pins
    Display OLEDdisplay = Display(9,25);

    //Set Display Chip Pointer to current gpio chip and open it 
    //This also handles setting up the lines for Reset and DC
    if(OLEDdisplay.setChipPointer(gpiod_chip_open("/dev/gpiochip0"))==false)
    {
        std::cerr << "Cannot Set GPIO Chip"<< ": "<< strerror(errno)<<std::endl;
        return 1;
    }

    //Now Initalize SPI Part of Display
    if(OLEDdisplay.spiInit()==false)
    {
        std::cerr << "Cannot Init SPI Device"<< ": "<< strerror(errno)<<std::endl;
        return 1;
    }

    //Now Initalize Display itself with config 
    if(OLEDdisplay.oledInit()==false)
    {
        std::cerr << "Cannot Init OLED Device"<< ": "<< strerror(errno)<<std::endl;
        return 1;
    }

     //Clear Anything on OLED Screen
    OLEDdisplay.oledClear();

    //Prompt User for Action
    OLEDdisplay.oledDrawString(0, 1,"Options\n1.User Input\n2.CPU Status\n");
    
    //Prompt user to enter something or show CPU Stats
    int state;
    std::cout << "Enter one of the following option numbers:\n" << "1.User Input\n" << "2.CPU Status\n";
    std::cin >> state;

    //Ignore Left over buffer
    std::cin.ignore(); 

    //Clear Anything on OLED Screen
    OLEDdisplay.oledClear();
    
    //To Show User Input(Max at 500 Chars)
    if(state ==1)
    {
        /*For Input*/

        //Display Progress:
        OLEDdisplay.oledDrawString(0, 0,"User is Typing...");

        //Unique pointer of character pointer that creates char[500] 
        char userInput[500];

        //Call cin .getline() method to pass our char pointer instead of string
        std::cin.getline(userInput,500);

        //Clear Display:
        OLEDdisplay.oledDrawString(0, 0,"                 ");

        //Draw String and fram it on position 0x0 on LED
        OLEDdisplay.oledDrawString(0, 0, userInput);

        // 1 second
        usleep(1000 * 500); 
    
    }
    else
    {
        //Create Current CPU Stats
        CPUStats cpu;
        
        for(int i = 0; i < 10; i++) 
        {
            //Run cpu stat and making file stuff in another thread
            //Pass in function pointer (memory address of function) and pass in the refrence to the cpu we care about
            std::thread t1(&CPUStats::updateStats,&cpu);

            //Draw String and fram it on position 0x0 on LED
            //Note:String.data() returns char* for strings
            OLEDdisplay.oledDrawString(0, 0, "CPU Stats:");

            //Display the fetching of the data
            OLEDdisplay.oledDrawString(0, 1,"Fetching Data....");
            
            //If done .join() with parent thread
            t1.join();

            //Clear Displaay Row
            OLEDdisplay.oledDrawString(0, 1,"                   ");
            
            //Combine Data(use string as it's arhtmetic supprots chars and can go to char* easily)
            //Note:String.data() returns char* for strings
            std::string userData= "User: " + cpu.user + "%";
            OLEDdisplay.oledDrawString(0, 2,userData.data());

            //Now Repeat for all the others
            std::string systemData= "System: " + cpu.system + "%";
            OLEDdisplay.oledDrawString(0, 3,systemData.data());
            std::string irqData= "IRQ: " + cpu.irq + "%";
            OLEDdisplay.oledDrawString(0, 4,irqData.data());
            std::string idleData= "Idle: " + cpu.idle + "%";
            OLEDdisplay.oledDrawString(0, 5,idleData.data());   

            //Timeout for 1 second
            usleep(1000000);
        }
        OLEDdisplay.oledDrawString(0, 1,"Fetching Stopped");
        
    }
    

    return 0;
}