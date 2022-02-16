#include <HTTP_Method.h>
#include <WebServer.h>
#include "CTBot.h"
#include "DHT.h"
#include "WiFi.h"
#include "Pinos.h"// já incluido arduino.h e  EEPROM.h

#define DHT11PIN 16

const char* APssid = "HteckLab_192_168_4_1";  // Enter SSID here
const char* APpassword = "4564564567";  //Enter Password here
String wssid = "";
String wpasswd = "";
String wtoken = ""   ; 
bool wifiapaga = false;
String w_disp = "Nenhuma Lista de wifi disponível";
bool vlr = false; // rele
String ref_temp = "0"; // temperatura de referencia
String comando_anterior = "0"; // comando anterior do telegram
String resp_zero = ""; // memoria zero
int dois_seg = 0;
float media_temp[61]; 
float media_temp2 = 0;

hw_timer_t *timer = NULL; //faz o controle do temporizador (interrupção por tempo)

Pinos o_pin(2); // Instancia Classe 
DHT dht(DHT11PIN,DHT11);

CTBot myBot; // Instancia Bot Telegram

WebServer server(80);// porta 80 webserver


//função que o temporizador irá chamar, para reiniciar o ESP32
void IRAM_ATTR resetModule(){
    Serial.println("(watchdog) reiniciar\n"); //imprime no log
    //esp_restart_noos(); //reinicia o chip
    ESP.restart();
}


void setup() {
  
  Serial.begin(115200);
  dht.begin();

  Cao_Guarda();
 
// se memoria vazia, inicia webserver
// lendo memoria
  resp_zero = o_pin.read_String_zero(0);
  Serial.print("\nEndereço zero = ");
  Serial.println(resp_zero);
  if (resp_zero == "Cheia"){
    wssid = o_pin.read_String(10);
    wpasswd = o_pin.read_String(100);
    wtoken = o_pin.read_String(200);
    ref_temp = o_pin.read_String(400);
    
    Serial.println(wssid);
    Serial.println(wpasswd);
    Serial.println(wtoken);
    wifi_conn(); // se falhar, escreve memoria_zero = "AP"
  }
  else{
    iniciaWebServer();
  }

 
}


void Cao_Guarda(){
    timer = timerBegin(0, 80, true); //timerID 0, div 80
    //timer, callback, interrupção de borda
    timerAttachInterrupt(timer, &resetModule, true);
    //timer, tempo (us), repetição
    timerAlarmWrite(timer, 20000000, true); // 6000000 = 6 seg
    timerAlarmEnable(timer); //habilita a interrupção 
}

void iniciaWebServer(){
  WiFi.mode(WIFI_AP);// WIFI_AP
  WiFi.softAP(APssid, APpassword);
  delay(500);
  w_disp = Lista(); // Lista Wifi disponíveis
  
  server.on("/", handleRoot);
  server.on("/scan",handleScan);
  server.on("/setting",handleSetting);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Servidor NodeMcu IP= ");
  Serial.print(WiFi.softAPIP()); 
}


void handleRoot() {  
  w_disp = Lista(); // Lista Wifi disponíveis
  server.send(200, "text/html", o_pin.Raiz(w_disp));
}
void handleScan() {
  w_disp = Lista(); // Lista Wifi disponíveis
  server.send(200, "text/html", o_pin.Raiz(w_disp));
}


void handleSetting(){
  w_disp = Lista(); // Lista Wifi disponíveis
  wssid = server.arg("ssid");
  wpasswd = server.arg("pass");
  wtoken = server.arg("token");
  Serial.println(wssid);
  Serial.println(wpasswd);
  Serial.println(wtoken);
  o_pin.writeString(0,"Cheia");
  o_pin.writeString(10,wssid);
  o_pin.writeString(100,wpasswd);
  o_pin.writeString(200,wtoken);
  //wifi_conn();
  server.send(200, "text/html", o_pin.RespHtml()); // **** escrever salvo na memoria - ok
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String PegaIp(){
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      return ipStr;
}

String Lista(){
String st;  
int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.println(")");
      //
      delay(10);
    }
  }
  Serial.println("");
  st = "<h4>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found    
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";    
    st += "<br>";
  }
  st += "</h4>";
  return st;
  
}

void wifi_conn(){
  int time_out = 0;
 
  Serial.println("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED){ // se diferente de conectado, conecta
    Serial.println(wssid.c_str());
    Serial.println(wpasswd.c_str());
    WiFi.begin(wssid.c_str(), wpasswd.c_str());
  }
  while ((WiFi.status() != WL_CONNECTED)&& (time_out < 30)) {
    time_out = time_out + 1;   
    Serial.println(time_out);
    delay(500);
    timerWrite(timer, 0); //reseta o temporizador (alimenta o watchdog)
   }
   
   if (time_out >=30){ // falhou
    Serial.println("Falhou wifi, iniciar web server");
    //o_pin.writeString(0,"AP"); // escreve "AP" na memoria zero pra levantar modo AP
    //ESP.restart();
    o_pin.writeString(0,"AP");
    o_pin.writeString(450,"Time_out");
    iniciaWebServer();
   }
   else{
      Serial.print("Endereço wifi = ");
      Serial.println(PegaIp());
      setup_bot(); // testa token
   }  

}

void setup_bot(){
  // set the telegram bot token
  myBot.setTelegramToken(wtoken);
  
  // check if all things are ok
  if (myBot.testConnection()){
    Serial.println("\ntestConnection Telegram = OK");
   
  }
  else{
    Serial.println("\ntestConnection Telegram = FALHA !");
    //o_pin.writeString(0,"AP"); // escreve "AP" na memoria zero pra levantar modo AP
    o_pin.writeString(0,"AP");
    
  }
    
}

void msg_bot(){
  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (CTBotMessageText == myBot.getNewMessage(msg)){

    String rasc = String (msg.text);
    bool flag_nt = false;
    if (rasc.length()==4){// se comprimento da mensagem for 4 exemplo: NTxx
      String rasc2 = rasc.substring(0, 2);
      if (rasc2.equalsIgnoreCase("NT")){// se tiver NT        

          String rasc_temp = rasc.substring(2,4);
          int temp_01 = rasc_temp.toInt();
          ref_temp = String(temp_01);
          if (temp_01 >=0 && temp_01<40){// se tiver dentro da faixa (0-40)
            flag_nt = true;
            o_pin.writeString(400,ref_temp);
          }          
      }
    }
    

    
    if (msg.text.equalsIgnoreCase("1")) {              
      String menu_txt; 
      menu_txt = "******** Relatório diário ********\n";
      menu_txt += "00:00= Ainda não Implementado !\n";
      menu_txt += "01:00= xxºC\n"; 
      menu_txt += "02:00= xxºC\n";                            
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }
    else if (msg.text.equalsIgnoreCase("2")) {              
      String menu_txt; 
      float temp = dht.readTemperature();
      menu_txt = "******** Temperatura Atual ********\n";
      menu_txt += String(temp,2); 
      menu_txt += " ºC\n";                          
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }
    else if (msg.text.equalsIgnoreCase("3")) {              
      String menu_txt; 
      menu_txt = "*** Verifica Temp. Ref. na EEPROM ***\n";
      menu_txt += o_pin.read_String(400);
      menu_txt += " ºC\n";                          
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }
    else if (msg.text.equalsIgnoreCase("4")) {              
      String menu_txt; 
      menu_txt = "***** Alterar Temp. de Ref.(0-40ºC) *****\n";
      menu_txt += "Digitar NTxx, onde xx é a nova temperatura\n";                       
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }
    else if (msg.text.equalsIgnoreCase("5")) {              
      String menu_txt; 
      menu_txt = "***** Média Temperatura (60seg)  *****\n";
      menu_txt += String(media_temp2,2); 
      menu_txt += " ºC\n";                          
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }    
    else if (msg.text.equalsIgnoreCase("6")) {              
      String menu_txt; 
      menu_txt = "***** Esquecer WIFI Atual *****\n";
      menu_txt += "Digite->wifiapaga";                 
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }  

    else if (msg.text.equalsIgnoreCase("wifiapaga")) {              
      String menu_txt; 
      menu_txt = "***** Apagando Wifi *****\n";
      wifiapaga = true;
      menu_txt += "Confirma ? (wsim)(wnao)";                     
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }   
    else if (msg.text.equalsIgnoreCase("wsim")) {              
      String menu_txt;      
      if (wifiapaga == true){
        o_pin.writeString(0,"AP");
        menu_txt = "***** Config Wifi apagada  *****\n"; 
      }
      else{
         menu_txt = "Comando inválido";
      }
      wifiapaga = false;                       
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }   
    else if (msg.text.equalsIgnoreCase("wnao")) {              
      String menu_txt;
      wifiapaga = false;
      menu_txt = "***** Wifi não modificada  *****\n";                             
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }         
    
    else if (flag_nt) {              
      String menu_txt; 
      menu_txt = "Temp. de Ref.(0-40ºC) alterada para ";
      menu_txt += ref_temp;
      menu_txt += " ºC\n";                      
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }   
    else {              
      String menu_txt; 
      menu_txt = "***********  COMANDOS:  *********\n";
      menu_txt += "1- Relatório Diário\n";
      menu_txt += "2- Temperatura Atual\n";   
      menu_txt += "3- Verifica Temperatura Referencia\n";  
      menu_txt += "4- Alterar Temperatura Referencia\n"; 
      menu_txt += "5- Media de temperatura\n"; 
      menu_txt += "6- Esquecer WIFI Atual\n";                         
      myBot.sendMessage(msg.sender.id, menu_txt);  
    }

  }

}
void ctrl_Temp(){
    //o_pin.Rele_1(vlr);
    float ref_temp_2 = ref_temp.toFloat(); 
    float soma = 0;
    //media_temp[] =
    //soma todas temperaturas e tira media
    for (int i = 0 ; i<=60 ; i++){
       soma = soma + media_temp[i];
    }
    media_temp2 = soma /60;

    //Serial.print("\nMedia temp = ");
    //Serial.println(String(media_temp2,2));
    // se valor lido for maior que referencia, aciona rele.
    if (media_temp2 > ref_temp_2){
      o_pin.Rele_1(HIGH);
    }
    else{
      o_pin.Rele_1(LOW);
    }
}



void loop() {
  server.handleClient(); // Web Server
  
  static uint32_t segundo = millis();
  if (millis() - segundo > 500) {
    segundo = millis();
    o_pin.PiscaLed();
    dois_seg = dois_seg + 1;
    //float rasc_tempera = dht.readTemperature();
    media_temp[dois_seg] = dht.readTemperature();
    //Serial.println (rasc_tempera);
  }
  if (dois_seg >= 60){
    ctrl_Temp();
    dois_seg = 0;
  }
  if (WiFi.status() == WL_CONNECTED){// checa mensagens telegram
    // reseta watchdog timer
    timerWrite(timer, 0); //reseta o temporizador (alimenta o watchdog) 
    // checa mensagens telegram
    msg_bot();
  }

  if (resp_zero =="AP"){ // modo AP
    timerWrite(timer, 0); //reseta o temporizador (alimenta o watchdog) 
    delay(500);
  }
  
  
  }
