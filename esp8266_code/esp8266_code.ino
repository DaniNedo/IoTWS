#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#include "DHTesp.h"

#define SETUP_PIN 0

DHTesp dht;

//#define DHT_PIN 2
//#define DHTTYPE DHT11

ADC_MODE(ADC_VCC);
#define MIN_VOLT 2.75

WiFiManagerParameter serverIP("server", "Server ip", "", 40);
WiFiManagerParameter serverPort("port", "Port", "", 2);
WiFiManagerParameter hostName("host","Host", "", 40);

//DHT dht(DHTPIN, DHTTYPE);

/*const char * serverIP = "145.14.144.245";
const uint16_t port = 80;
const char * host; */

const char* ip = "";
int port;
const char* host = "";

float volt;
float temp;
float hum;

void setup() {
  Serial.begin(115200);
  Serial.println("\n Starting");
 
  Serial.println("\n ADC Mode set");

  pinMode(SETUP_PIN, INPUT);
  
  dht.setup(2);
}

void WiFiSetUp(){
  WiFiManager wifiManager;
  wifiManager.addParameter(&serverIP);
  wifiManager.addParameter(&serverPort);
  wifiManager.addParameter(&hostName);
  wifiManager.setTimeout(120);
  
  if (!wifiManager.startConfigPortal("OnDemandAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.println("connected...yeey :)");
  
  ip = serverIP.getValue();
  port = atoi(serverPort.getValue());
  host = hostName.getValue();

  Serial.print("IP: ");
  Serial.println(ip);
  Serial.print("Port: ");
  Serial.println(port);
  Serial.print("Host: ");
  Serial.println(host);
  
}

float roundNum(float num){
  num *= 100;
  if(num >= 0) num += 0.5; else num -= 0.5;
  long rounded = num;
  num = rounded;
  num /= 100;
}

bool getData(){
  volt = ESP.getVcc()/1024;

  temp = dht.getTemperature();
  
  hum = dht.getHumidity();
  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  return true;
}

void postData(String params){
  WiFiClient client;
  
  if (!client.connect(ip, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(10000);
    return;
  }
  Serial.println("Succesfully connected to server");
  // This will send the request to the server
  Serial.print("GET /php/postdata.php?");
  Serial.print(params);
  //client.print("temperature=23.4&humidity=40&voltage=3.2&sendmail=1");
  Serial.println(" HTTP/1.1");
  Serial.print("HOST: ");
  Serial.println(host);
  Serial.print("\n\n");
  
  //---------------------------------------//
  
  client.print("GET /php/postdata.php?");
  client.print(params);
  //client.print("/php/postdata.php?temperature=23.4&humidity=40&voltage=3.2&sendmail=1");
  client.println(" HTTP/1.1");
  client.print("HOST: ");
  //client.println("daninedo97.000webhostapp.com");
  client.println(host);
  client.println("Connection: keep-alive");
  client.println("Upgrade-Insecure-Requests: 1");
  client.println("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebkit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.162 Safari/537.36");
  client.println("Accept-Language: es-ES,es;q=0.9,en;q=0.8");
  client.println();
  
  unsigned long timeout = millis();
  
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  client.stop();
}

void loop() {
  if(digitalRead(SETUP_PIN) == LOW){
    WiFiSetUp();
  }

  if(ip == ""){
    delay(100);
    return;
  }
  
  String params = "";
  bool sendMail = true;
  
  if(getData()){

    
    params += "temperature=" + String(temp, 2);
    params += "&humidity=" + String(hum, 2);
    params += "&voltage=" + String(volt, 2);
    
    if(sendMail == true && volt < MIN_VOLT){
      sendMail = false;
      //generamos string con send mail
      params += "&sendMail=1";
    }else{
      params += "&sendMail=0";
    }
    
    if(volt >= MIN_VOLT){
    sendMail = true;
    }
    //Serial.println(params);
    postData(params);
    delay(20000);
    //Serial.println("Going into deep sleep for 20 seconds");
    //ESP.deepSleep(20e6); // 20e6 is 20 microseconds
  }

}
