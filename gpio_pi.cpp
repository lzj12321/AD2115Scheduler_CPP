#include "gpio_pi.h"
#include<QMessageBox>
#include<wiringPi.h>


GPIO_PI::GPIO_PI()
{
}

bool GPIO_PI::gpioIni()
{
    if(wiringPiSetupGpio()==-1)
        return false;
    else
    {
        pullUpDnControl(4,PUD_DOWN);
        pullUpDnControl(18,PUD_DOWN);
        pullUpDnControl(22,PUD_DOWN);
        pullUpDnControl(24,PUD_DOWN);

        pullUpDnControl(17,PUD_DOWN);
        pullUpDnControl(27,PUD_DOWN);
        pullUpDnControl(23,PUD_DOWN);
        pullUpDnControl(25,PUD_DOWN);
        pullUpDnControl(0,PUD_DOWN);
        return true;
    }
}

void GPIO_PI::setIOStatus(unsigned int i, bool status)
{
    pinMode(i,OUTPUT);
    digitalWrite(i,status);
}

unsigned int GPIO_PI::getIOStatus(unsigned int i)
{
    pinMode(i,INPUT);
    return digitalRead(i);
}



