#include <stdlib.h>
#include <SoftwareSerial.h>
#include <dht.h>
#include <Wire.h>
#define SSID "AndroidAP"
#define PASS "M01289199113"
#define IP "184.106.153.149" // thingspeak.com IP
#define DHT11_PIN 7
String GET = "GET /update?key=6YI0Q0W33SZCN21C&field1=";
SoftwareSerial monitor(0, 1);
dht DHT;

int nRainIn = A0;
int nRainDigitalIn = 2;
int RainVal;
boolean bIsRaining = false;
String strRaining;

//Variables
int luminancePin = A5;
int dustPin = 12;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long delay_time = 60000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

//setup
void setup()
{
  //start serial communications
  Serial.begin(9600);
  monitor.begin(9600);
  Serial.println("Initializing...");

  //configure Arduino pins
  pinMode(dustPin, INPUT);
  pinMode(2,INPUT);
  
    //communication with wifi module
    monitor.flush();
    monitor.println("AT");
    delay(2000);
    
    if(monitor.find("OK")){
      Serial.println("Communication with ESP8266 module: OK");
    }
    else {
      Serial.println("ESP8266 module ERROR");
    }

  //connect wifi router  
  connectWiFi(); 
     
  Serial.print("Sampling (");
  Serial.print(sampletime_ms/1000);
  Serial.println("s)...");
  
  //initialize timer
  starttime = millis();

}

void loop(){

  //measuring dust particles
  duration = pulseIn(dustPin, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;
  
  //30 seconds cicle
  if ((millis() - starttime) >= sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    lowpulseoccupancy = 0;
    
    //read other sensors
    char buffer[10];
    
    //light sensor
    float luminance = analogRead(luminancePin);

   
    //temperature and humidity
    int chk = DHT.read11(DHT11_PIN);
    float humidity = DHT.humidity;
    float temperature = DHT.temperature;
    
  //Rain
  RainVal = analogRead(nRainIn);
  bIsRaining = !(digitalRead(nRainDigitalIn));
  
  if(bIsRaining){
    strRaining = "YES";
  }
  else{
    strRaining = "NO";
  }
  
   

    //convert sensor values to strings
    String luminanceStr = dtostrf(luminance, 4, 1, buffer);
    luminanceStr.replace(" ","");
  
    String humidityStr = dtostrf(humidity, 4, 1, buffer);
    humidityStr.replace(" ","");
    
    String temperatureStr = dtostrf(temperature, 4, 1, buffer);
    temperatureStr.replace(" ","");

    String rainStr = dtostrf(RainVal, 4, 1, buffer);
    rainStr.replace(" ","");
    
    String dustStr = dtostrf(concentration, 4, 1, buffer);
    dustStr.replace(" ","");
 
        
    //send data to ThingSpeak
    updateSensors(luminanceStr, humidityStr, temperatureStr, dustStr, rainStr);

    //wait next sampling cycle
    Serial.print("Wait ");
    Serial.print(delay_time/1000);
    Serial.println("s for next sampling");
    Serial.println();
    delay(delay_time);
    
    //initialize new cycle
    Serial.println();
    Serial.print("Sampling (");
    Serial.print(sampletime_ms/1000);
    Serial.println("s)...");
    starttime = millis();
  }
}

//Send data to ThingSpeak
void updateSensors(String luminanceStr, String humidityStr, String temperatureStr,  String dustStr, String rainStr) {

  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  monitor.println(cmd);
  delay(2000);
 
  cmd = GET;
  cmd += luminanceStr;
  cmd += "&field2=";
  cmd += humidityStr;
  cmd += "&field3=";
  cmd += temperatureStr;
  cmd += "&field4=";
  cmd += rainStr;
  cmd += "&field5=";
  cmd += dustStr;
  cmd += "&field6=";
  delay(1000);
  int strsize = cmd.length();
  monitor.println("AT+CIPSEND=" + String(strsize));
  delay(2000);
  
  monitor.print(cmd);
  if(monitor.find("OK")){
    Serial.println("Transmission completed with success");
  }else{
    Serial.println("Transmission failed!");
  }
}

void sendDebug(String cmd){
  Serial.print("SEND: ");
  Serial.println(cmd);
  monitor.println(cmd);
} 
 
boolean connectWiFi(){
  Serial.println("Connecting wi-fi...");
  String cmd ="AT+CWMODE=1";
  monitor.println(cmd);
  delay(2000);
  monitor.flush(); //clear buffer
  cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  monitor.println(cmd);
  delay(5000);
  
  if(monitor.find("OK")){
    Serial.println("Connection succeeded!");
    return true;
  }else{
    Serial.println("Connection failed!");
    return false;
  }
  Serial.println();
}
