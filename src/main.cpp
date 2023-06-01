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
#define TdsSensorPin A0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0xA1, 0x11, 0x49, 0xF6, 0xEA, 0x3C, 0x64 };
#define VREF 3.3 // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point
int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature;

float resistor1 = 170;
float resistor2 = 330;

float calibration = 16.7; //change this value to calibrate
const int analogInPin = A0;
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10],temp;

int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++){
    for (i = 0; i < iFilterLen - j - 1; i++){
      if (bTab[i] > bTab[i + 1]){
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

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
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);
  Serial.begin(9600);   //inicializa comunicação serial
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
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
  temperature = sensors.getTempC(insideThermometer);
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
  digitalWrite(D0, HIGH);
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
  digitalWrite(D0, LOW);
  float pHVol=(float)avgValue*5.0/1024/6;
  pHVol = pHVol + 0.5;
  float phValue = -5.70 * pHVol + calibration;
  // digitalWrite(D1, HIGH);
  // static unsigned long analogSampleTimepoint = millis();
  // if(millis()-analogSampleTimepoint > 40U) //every 40 milliseconds,read the analog value from the ADC
  // {
  //   analogSampleTimepoint = millis();
  //   analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
  //   analogBufferIndex++;
  //   if(analogBufferIndex == SCOUNT)
  //     analogBufferIndex = 0;
  // }
  // static unsigned long printTimepoint = millis();
  // if(millis()-printTimepoint > 800U)
  // {
  // for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
  // analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
  // averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF/ 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  // float compensationCoefficient=1.0+0.02*(temperature-25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  // float compensationVolatge=averageVoltage/compensationCoefficient; //temperature compensation
  // tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
  // }
  // digitalWrite(D1, LOW);
  String response ="HTTP/1.1 200 OK\r\n Content-type:text/html\r\n\r\n<!DOCTYPE HTML><HTML><BODY>Temperature is ";
  response+= temperature;
  response += " º C<br>";
  response+= "pH is ";
  response+= phValue;
  response+= "<br>TDS value is ";
  response+= tdsValue;
  response+= " ppm<br>";
  response+= "Agua boa para consumo";
  response+= "</BODY></HTML>";
  myclient.print(response);
  delay(10);  
} 

