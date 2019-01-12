/*==============================================*/
/*  Firmware para el proyecto IoTWS
/*  Por Daniel Nedosseikine para MakersUPV
/*  Mayo 2018
/*==============================================*/

//Incluimos las librerías necesarias
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <DHTesp.h>

//Establecemos un modo que nos permite leer el voltaje de entrada
ADC_MODE(ADC_VCC);

//Definimos varias constantes
#define MIN_VOLT 2.75
#define SETUP_PIN 0
#define DHT_PIN 2
#define PORT 80

//Creamos un campo personalizado para poder introducir
//nuestro host desde el móvil
WiFiManagerParameter hostName("host","Host", "", 40);
//WiFiManagerParameter serverPort("port", "Port", "", 2);

//Creamos el objeto dht basado en la clase DHTesp
DHTesp dht;

//Creamos las variables necesarias, en este caso globales
const char* host = "";
float volt;
float temp;
float hum;
String params = "";

//Creamos la función setup, que se ejecuta una sola vez tras
//el encendido del microcontrolador
void setup() {
  //Solo debugg
  Serial.begin(115200);
  Serial.println("\n Starting");
  Serial.println("\n ADC Mode set");

  //Definimos el pin 0 como una entrada (aquí va conectado el botón)
  pinMode(SETUP_PIN, INPUT);

  //Inicializamos el sensor DHT especificando el pin y el modelo
  //Nota: el modelo solo es DHT11, con DHTesp::DHT11 accedemos a
  //un parámetro público de la case DHTesp llamado DHT11
  dht.setup(DHT_PIN, DHTesp::DHT11);
}

//Creamos la función WifiSetUp, esta función nos permitirá configurar
//los parametros necesarios para la conexión WiFi desde el móvil
void WiFiSetUp(){

  //Creamos el objeto wifimanager derivado de la clase WiFiManager
  WiFiManager wifiManager;

  //Añadimos el campo del host a la configuración
  wifiManager.addParameter(&hostName);

  //Configuramos un tiempo máximo que el portal estara abierto
  wifiManager.setTimeout(120);

  //Intentamos abrir una red WiFI con el nombre IotWS_Setup
  if (!wifiManager.startConfigPortal("IoTWS_Setup")) {

    //Si no lo conseguimos reiniciamos el microcontrolador
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  //Debug
  Serial.println("connected...yeey :)");

  //Copiamos el nombre del host a la variable creada antes
  host = hostName.getValue();

  //Debug
  Serial.print("Host: ");
  Serial.println(host);

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
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  //Todo ha ido correcto, devolvemos un 1
  return true;
}

//Creamos una función que manda los datos al servidor
void postData(void){

  //Creamos el objeto client derivado de la clase WiFiClient
  WiFiClient client;

  //Intentamos establecer la conexión con el servidor
  if (!client.connect(host, PORT)) {
    //Debug
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(10000);
    return;
  }
  //Debug
  Serial.println("Succesfully connected to server");
  // This will send the request to the server
  Serial.print("GET /php/postdata.php?");
  Serial.print(params);
  Serial.println(" HTTP/1.1");
  Serial.print("HOST: ");
  Serial.println(host);
  Serial.print("\n\n");

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
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  //Mientras que haya una respuesta, la leemos
  //Nota: no vamos a usar esta información, pero es mejor
  //completar la comunicación Cliente-Servidor
  while(client.available()){
    String line = client.readStringUntil('\r');
    //Debug
    Serial.print(line);
  }

  //Nos desconectamos del servidor
  client.stop();
}

//Creamos la función principal que se ejecutará continuamente
void loop() {

  //Comprobamos si hay que entrar en modo configuración
  if(digitalRead(SETUP_PIN) == LOW){
    WiFiSetUp();
  }

  //Si el host está vacío es que no hemos establecido la configuración
  if(host == ""){
    delay(100);
    return;
  }

  //Creamos una variable para saber si enviar la notificación
  //de batería baja
  bool sendMail = true;

  //Comprobamos si obtenemos las mediciones correctamente
  if(getData()){

    //Construimos un string de la siguiente forma:
    //temperature=0.00&humidity=0.00&voltage=0.00
    params += "temperature=" + String(temp, 2);
    params += "&humidity=" + String(hum, 2);
    params += "&voltage=" + String(volt, 2);

    //Comprobamos si el voltaje es inferior al mínimo
    //y si ya hemos enviado una notificiación
    if(sendMail == true && volt < MIN_VOLT){
      sendMail = false;
      params += "&sendMail=1";
    }
    else{
      params += "&sendMail=0";
    }

    //Si el voltaje vuelve a ser mayor que el mínimo
    //permitímos el envío de nuevas notificiaciones
    if(volt >= MIN_VOLT){
    sendMail = true;
    }

    //Subimos los datos al servidor
    postData();

    delay(20000);
    //Serial.println("Going into deep sleep for 20 seconds");
    //Ponemos el ESP en modo deepSleep durante 20 minutos
    //ESP.deepSleep(20e6); // 20e6 is 20 microseconds
  }

}
