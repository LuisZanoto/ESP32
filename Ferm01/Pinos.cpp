/*
  Pinos.cpp - Library
*/

#include "Arduino.h"
#include "Pinos.h"

#include <EEPROM.h>

Pinos::Pinos(int pin)// Construtor
{
  pinMode(pin, OUTPUT); // led azul
  pinMode(23, OUTPUT); // rele
 
  _pin = pin;
}

void Pinos::PiscaLed()
{
  digitalWrite(_pin, HIGH);
  delay(50);
  digitalWrite(_pin, LOW);
  delay(250);  
}
void Pinos::Rele_1(bool vl){
  if (vl == true){
   digitalWrite(23, HIGH);
  }
  else{
   digitalWrite(23, LOW);
  }
}

float Pinos::Le_ad(int ad_pin){
      //int sensorValue = 0;
      //float resp = 0;
      //analogSetAttenuation(ADC_0db); // faixa 100mv - 950mv
      //sensorValue = analogRead(ad_pin);
      //resp = (sensorValue *1200) / (4095) ;  //milivolts  
      //resp = resp /10.0;  
      //return resp;
      
}

void Pinos::writeString(char add, String dados)
    {
      EEPROM.begin(512);
      int _size = dados.length();
      int i;
      for(i=0;i<_size;i++)
      {
        EEPROM.write(add+i,dados[i]);
      }
      EEPROM.write(add+_size,'\n');   //Add LF character for String Data
      EEPROM.commit();
      EEPROM.end();
    }

String Pinos::read_String(char add)
    {
      EEPROM.begin(512);
      int i;
      char data[100]; //Max 100 Bytes
      int len=0;
      unsigned char k;
      k=EEPROM.read(add);
      while(k != '\n' && len<99)   //Read until LF
      {    
        k=EEPROM.read(add+len);
        data[len]=k;
        len++;
      }
    //** retira lixo
    String rasc = String(data);
    rasc = rasc.substring(0, len -1);
    EEPROM.end();
    return String(rasc);
    }

String Pinos::read_String_zero(char add)
    {
      Serial.println("\nLendo_zero");
      EEPROM.begin(512);
      int i;
      char data[20]; //Max 20 Bytes
      int len=0;
      unsigned char k;
      k=EEPROM.read(add);
      while(k != '\n' && len<9)   //Read until LF ou len <10
      {    
        //Serial.println(len);
        k=EEPROM.read(add+len);
        data[len]=k;
        len++;        
      }
    ////** retira lixo
    String rasc = String(data);
    rasc = rasc.substring(0, len -1);
    EEPROM.end();
    return String(rasc);
    }



String Pinos::Raiz(String st_l){  
  
      String content = "<!DOCTYPE HTML>\r\n<html><h2>HteckLab - Parametros de Configuracao: </h2><h3>";
      content += "<head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'>";
      content += "<meta name='viewport' content='width=device-width, initial-scale=1.0'><title>HteckLab - IOT</title>";
      content += "<style>body { background-color: rgb(4, 54, 54); color: white; }</style></head>";
      content += "<body>";
      content += "<form method='POST' action='/scan'><input type='submit' value='scan'></form>";
      content += "<p>";
      content += st_l; // scan de todas wifi disponíveis
      content += "</p>";
      content += "<form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32 size='15'>";
      content += "<p><label>Senha: </label><input name='pass' length=64 size='20' ></p>";
      content += "<p><label>Token Telegram: </label><input name='token' length=128 size='70'></p>";
      content += "<p><input type='submit'></form></p></h3></body></html>";
      return content;  
}

String Pinos::RespHtml(){  
  
      String content2 = "<!DOCTYPE HTML>\r\n<html><h2>HteckLab - Parametros de Configuracao: </h2><h3>";
      content2 += "<head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'>";
      content2 += "<meta name='viewport' content='width=device-width, initial-scale=1.0'><title>HteckLab - IOT</title>";
      content2 += "<style>body { background-color: rgb(4, 54, 54); color: white; }</style></head>";
      content2 += "<body>";
      content2 += "<p><label>Configuração Salva !</p>";
      content2 += "<p><label>Reiniciar o Dispositivo !</p>";
      //content2 += "</p><form method='POST' action='/init'><input type='submit' value='reiniciar'></form>";
      content2 += "</h3></body></html>";
      return content2;  
}
