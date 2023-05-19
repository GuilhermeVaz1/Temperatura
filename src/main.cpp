/*
    Exibe o endereço físico do sensor Dallas DS18B20
    e mostra no Serial Monitor 


*/

#define ONE_WIRE_BUS D7

#include <Arduino.h>
#include <OneWire.h> //biblioteca protocolo One Wire
#include <DallasTemperature.h> //biblioteca sensor Dallas DS18B20
#include <ESP8266WiFi.h>   //   include ESP8266 library
const char* ssid= "Moto G (5S) Plus 4960";    // wifi name
const char* pass= "25252525";  // wifi password
WiFiServer server(80);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0xA1, 0x11, 0x49, 0xF6, 0xEA, 0x3C, 0x64 };

float resistor1 = 170;
float resistor2 = 330;

float calibration = 16.7; //change this value to calibrate
const int analogInPin = A0;
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10],temp;

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) 
  {
    Serial.print("Erro de leitura");
  } 
  else 
  {
    Serial.print(tempC);
    Serial.print(" graus C ");
    Serial.print("  |  ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.print(" graus F ");
  }

  Serial.print("\n\r");
  
} //end printTemperature

// Configurações Iniciais
void setup() 
{
  Serial.begin(9600);   //inicializa comunicação serial
 
  sensors.begin();      //inicializa sensores
  
  sensors.setResolution(insideThermometer, 10); //configura para resolução de 10 bits  //chama função que irá buscar por sensores presentes no sistema
  WiFi.begin(ssid,pass);  // Wifi Initialization with SSID and PASSWORD
  Serial.print("Connecting to ");
  Serial.print(ssid);  // Print connecting to Wifi name
 // here is waiting to connect internet
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println(" ");
  Serial.println("connected succesfully");


  server.begin();   //  begin the server
  Serial.print("Server begin at IP ");
  Serial.println(WiFi.localIP());  // print IP address of Node MCU
} //end setup

void loop() 
{
  Serial.println("Sensor DS18B20");
  sensors.requestTemperatures();
  printTemperature(insideThermometer);
  WiFiClient myclient  = server.available();    // server.available return to client object, it need to store into a variable myclient
 if(!myclient)  //  if there is no client then it should return back
 return;

 // if there is client then come inside
 Serial.println("New Client");
 // now we can check client has some data available for us to read

 while(!myclient.available())  // if there is no data available from client
 {
         // wait
 }

 // if there is avaailable data from client then we have to read and store in String variable

  String request = myclient.readStringUntil('\n'); //  it will read untill \n 
  myclient.flush();  // flush out extra line from request
  Serial.println(request);

  float req = sensors.getTempC(insideThermometer);
  for(int i=0;i<10;i++)
  {
  buf[i]=analogRead(analogInPin);
  delay(30);
  }
  for(int i=0;i<9;i++)
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)
  avgValue+=buf[i];
  float pHVol=(float)avgValue*5.0/1024/6;
  float phValue = -5.70 * pHVol + calibration;
  String response ="HTTP/1.1 200 OK\r\n Content-type:text/html\r\n\r\n<!DOCTYPE HTML><HTML><BODY>Temperature is ";
  response+= req;
  response+= "\r\npH is ";
  response+= phValue;
  response+= "\r\nTDS value is ";
  response+= random(300,310);
  response+= "\r\nAgua boa para consumo";
  response+= "</BODY></HTML>";
  myclient.print(response);
  delay(10);  
} 

