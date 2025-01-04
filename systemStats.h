#include <string>
#include <fstream>
#include <sstream>

//Can't make copy of it so pass by refrence and skip next spot unitl next ' '
void skipWhiteSpace(std::istringstream &averageLine)
{
    std::string temp;
    averageLine >> temp;
}
struct CPUStats
{

    //Percntage of User on CPU
    std::string user;

    //Perctange of System Task On CPU
    std::string system;

    //Percntage of IRQ(Interrupt Request)
    std::string irq;

    //Perctange of Idling Time
    std::string idle;

    void updateStats()
    {
        //To not use /proc and do time interval math, let's run mpstat and make a textfile

        //Create command to run it and spew output in .txt
        const char* command = "mpstat -P ALL 1 1 > cpu_usage.txt";

        //Run command in system
        std::system(command);

        //Then Parse through textfile and create input file stream
        std::ifstream file("cpu_usage.txt");

        //Check if we actually opened it
        if(file.is_open()==true)
        {
            //If opened parse through it
            std::string line;

            //While getline() is not at average skip
            while(getline(file,line))
            {
                if(line.substr(0,16)=="Average:     all")
                {

                    //If we find line with average stats, instead of parsing let's turn it
                    //into a stringstream that has a delimter of 0 so we can pass it to the cpu quickly
                    std::istringstream averageLine(line);
                    
                    //Skip average and all
                    skipWhiteSpace(averageLine);
                    skipWhiteSpace(averageLine);

                    //Then pass Stats accordingly skip 
                    averageLine >> this->user;
                    skipWhiteSpace(averageLine);
                    averageLine >> this->system;
                    skipWhiteSpace(averageLine);
                    averageLine >> this->irq;
                    skipWhiteSpace(averageLine);
                    skipWhiteSpace(averageLine);
                    skipWhiteSpace(averageLine);
                    skipWhiteSpace(averageLine);
                    averageLine >> this->idle;
                }
            }
        }
        else
        {
            std::cout << "File Failed to open"<<std::endl;
        }

    }
    void printStats()
    {
        std::cout << "User: " << this->user << std::endl;
        std::cout << "System: " << this->system << std::endl;
        std::cout << "Interrupts: " << this->irq << std::endl;
        std::cout << "Idle: " << this->idle << std::endl;
    }
};