/*
  Pinos.h - library
*/
#ifndef Pinos_h
#define Pinos_h

#include "Arduino.h"

class Pinos
{
  public:
     
    Pinos(int pin); // Construtor
    void Rele_1(bool v);// rele
    void PiscaLed();
    float Le_ad(int ad_pin);
    void writeString(char add, String dados);
    String read_String(char add);
    String read_String_zero(char add);
    String Raiz(String st_l);
    String RespHtml();
    
    
  private:
    int _pin;
};

#endif
