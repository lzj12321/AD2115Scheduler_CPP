#ifndef GPIO_PI_H
#define GPIO_PI_H

class GPIO_PI
{
public:
    GPIO_PI();
    bool gpioIni();
    void setIOStatus(unsigned int,bool);
    unsigned int getIOStatus(unsigned int);
};



#endif // GPIO_PI_H
