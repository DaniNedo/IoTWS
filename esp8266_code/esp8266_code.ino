/*==============================================*/
/*  Firmware para el proyecto IoTWS
/*  Por Daniel Nedosseikine para MakersUPV
/*  Enero 2019
/*==============================================*/

//Incluimos las librerías necesarias
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <DHTesp.h>

//Establecemos un modo que nos permite leer el voltaje de entrada
ADC_MODE(ADC_VCC);

//Definimos varias constantes
#define DHT_PIN 2
#define PORT 80

//Para el funcionamiento final del código no nos hace
//falta el modo debug. Descomentar si se necesita este modo.
#define DEBUG

//Creamos un campo personalizado para poder introducir
//nuestro host desde el móvil
WiFiManagerParameter hostName("host","Host", "", 40);

//Creamos el objeto dht basado en la clase DHTesp
DHTesp dht;

//Creamos las variables necesarias, en este caso globales
String host = "";
float volt;
float temp;
float hum;
String params = "";

//Creamos la función writeToEEPROM que nos permite grabar datos 
//en la memoria EEPROM
void writeToEEPROM(char addr, String data)
{
  //Guardamos byte a byte el String que se ha pasado a la función
  int _size = data.length();
  for(int i=0;i<_size;i++)
  {
    EEPROM.write(addr + i, data[i]);
  }
  //Añadimos el caracter nulo
  EEPROM.write(addr + _size,'\0');
  EEPROM.commit();
}

//Creamos la función readFromEEPROM que nos permite leer datos
//de la memoria EEPROM
String readFromEEPROM(char add)
{
  //Leemos la información hasta llegar al caracter nulo
  int i;
  char data[100]; 
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  //Devolvemos el String generado
  return String(data);
}

//Creamos la función saveHost, que guardará el nombre de nuestro host
//que hemos introducido desde la plataforma de configuración
void saveHost(){
  
  //Copiamos el nombre del host en la variable host
  host = hostName.getValue();
  //Guardamos el nombre del host en la memoria EEPROM
  writeToEEPROM(0, host);
}

//Creamos la función WifiSetUp, esta función nos permitirá configurar
//los parametros necesarios para la conexión WiFi desde el móvil
void WiFiSetup(){

  //Creamos el objeto wifimanager derivado de la clase WiFiManager
  WiFiManager wifiManager;
 
  //wifiManager.resetSettings();
  
  //Añadimos el campo del host a la configuración
  wifiManager.addParameter(&hostName);

  //Configuramos un tiempo máximo que el portal estara abierto
  wifiManager.setTimeout(120);

  //Si la configuración WiFi cambia llamaremos a la función saveHost
  wifiManager.setSaveConfigCallback(saveHost);

  //Incovamos la función autoConnect, que tratará de conectarse a una
  //red WiFi utilizando las credenciales previamente guardadas
  //(si existen). En caso de no poderse conectar, se abrira el portal
  //de configuración WiFi.
  wifiManager.autoConnect("IoTWS_Setup");

  #ifdef DEBUG
    Serial.println("Conectado!");
  #endif
  delay(100);
  
  //Recuperamos el nombre del host desde la memoria EEPROM
  host = readFromEEPROM(0);

  #ifdef DEBUG
    Serial.print("Host: ");
    Serial.println(host);
  #endif
}

//Creamos una función que actualiza las mediciones
bool getData(void){

  //Calculamos el voltaje de entrada
  volt = ESP.getVcc()/1024;

  //Obtenemos la temperatura
  temp = dht.getTemperature();

  //Obtenemos la humedad
  hum = dht.getHumidity();

  //Si alguno de los valores no es un número
  //entonces ha habido un fallo, devolvemos un 0
  if (isnan(temp) || isnan(hum)) {
    #ifdef DEBUG
      Serial.println("Fallo al leer del sensor");
    #endif
    return false;
  }

  //Todo ha ido correcto, devolvemos un 1
  return true;
}

//Creamos una función que manda los datos al servidor
bool postData(void){

  //Creamos el objeto client derivado de la clase WiFiClient
  WiFiClient client;

  //Intentamos establecer la conexión con el servidor
  if (!client.connect(host, PORT)) {

    #ifdef DEBUG
      Serial.println("Fallo de conexion con el servidor");
      Serial.println("Esperamos 5 segundos");
    #endif

    delay(5000);
    return false;
  }

  #ifdef DEBUG
    Serial.println("Conexión con el servidor establecida");
    Serial.print("GET /php/postdata.php?");
    Serial.print(params);
    Serial.println(" HTTP/1.1");
    Serial.print("HOST: ");
    Serial.println(host);
    Serial.print("\n\n");
  #endif

  //Generamos una petición HTTP básica
  client.print("GET /php/postdata.php?");
  client.print(params);
  client.println(" HTTP/1.1");
  client.print("HOST: ");
  client.println(host);
  client.println("Connection: keep-alive");
  client.println("Upgrade-Insecure-Requests: 1");
  client.println("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebkit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.162 Safari/537.36");
  client.println("Accept-Language: es-ES,es;q=0.9,en;q=0.8");
  client.println();

  //Creamos una variable que almacene el tiempo inicial
  unsigned long timeout = millis();

  //Comprobamos si obtenemos respeusta del servidor
  while (client.available() == 0) {
    //Si pasan más de 5 segundos es que ha habido un fallo
    if (millis() - timeout > 5000) {

      #ifdef DEBUG
        Serial.println("Sobrepasado tiempo de espera");
      #endif

      client.stop();
      return false;
    }
  }

  //Mientras que haya una respuesta, la leemos
  //Nota: no vamos a usar esta información, pero es mejor
  //completar la comunicación Cliente-Servidor
  while(client.available()){
    String line = client.readStringUntil('\r');

    #ifdef DEBUG
      Serial.print(line);
    #endif
  }

  //Nos desconectamos del servidor
  client.stop();
  return true;
}

//Creamos la función setup, que se ejecuta una sola vez tras
//el encendido del microcontrolador
void setup() {
  //Mostramos un mensaje por el monitor serie
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("\n Encendido");
    Serial.println("\n Modo ADC");
  #endif

  //Inicializamos la memoria EEPROM
  EEPROM.begin(512);

  //Inicializamos el sensor DHT especificando el pin y el modelo
  //Nota: el modelo solo es DHT11, con DHTesp::DHT11 accedemos a
  //un parámetro público de la case DHTesp llamado DHT11
  dht.setup(DHT_PIN, DHTesp::DHT11);

  //Invocamos la función WiFiSetup
  WiFiSetup();
}

//Creamos la función principal que tratará de recopilar y subir los 
//datos al servidor
void loop() {
  //Si el host está vacío es que no hemos establecido la configuración
  if(host == ""){
    delay(100);
    return;
  }

  //Comprobamos si obtenemos las mediciones correctamente
  if(getData()){

    //Construimos un string de la siguiente forma:
    //temperature=0.00&humidity=0.00&voltage=0.00
    params += "temperature=" + String(temp, 2);
    params += "&humidity=" + String(hum, 2);
    params += "&voltage=" + String(volt, 2);

    //Subimos los datos al servidor
    if(postData()){

      #ifdef DEBUG
        Serial.println("Entrando en modo Deep Sleep durante 10 minutos");
      #endif
      //Ponemos el ESP en modo Deep Sleep durante 10 minutos
      //Hay que indicar el periodo en microsegundos
      ESP.deepSleep(10e8);
    }
  }
  delay(1);
}
