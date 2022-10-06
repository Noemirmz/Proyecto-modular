#include <ArduinoJson.h>
#include <AsyncWebServer_WT32_ETH01.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"


//Libreria para importar archivos css, html y js externos.
#define RXp2 16
#define TXp2 17 //Comentario para probar la rama titulo nuevo
#define Photoresistor 4



//------------------Servidor Web en puerto 80---------------------

AsyncWebServer server(80);
//AsyncWebSocket ws("/ws");
//AsyncWebSocketClient * globalClient = NULL;


//---------------------Credenciales de WiFi-----------------------

const char* ssid     = "LACER";
const char* password = "Fourier1.61";
//const char* ssid     = "MEGACABLE-2.4G-25A7";
//const char* password = "Ueww6yfZ4E";

//---------------------VARIABLES GLOBALES-------------------------
short contconexion = 0;

String header; // Variable para guardar el HTTP request

String Estado_Fotogoniometro = "Apagado";

bool estado = false;

String gradosDeSensado;

float gradoSensor = 0;

float miliSensor = 0;

short longitud = 20;   

const size_t capacity = JSON_ARRAY_SIZE(longitud) + longitud*JSON_OBJECT_SIZE(2);

DynamicJsonDocument doc(capacity);

String JSON;


//----Función Processor para regresar valores a página Web-------------------------

String processor(const String& var) {
  if (var == "ESTADO")
  {
    if (estado == true)
    {
      Estado_Fotogoniometro = "Encendido";
    }
    else
    {
      Estado_Fotogoniometro = "Apagado";
    }
    return Estado_Fotogoniometro;
  }
   return Estado_Fotogoniometro;
}

//---------------------------SETUP--------------------------------
void setup() {
  //Para mostrar en el puerto seria
  Serial.begin(115200);
  Serial.println("");

  //Para mandar informacion al arduino
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Conexión WIFI
  WiFi.begin(ssid, password);
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and contconexion < 10) {
    ++contconexion;
    delay(500);
    Serial.print("Conectandose a internet...");
  }

  if (contconexion < 10) {
    //para usar con ip fija
    //IPAddress ip(192,168,1,180);
    //IPAddress gateway(192,168,1,1);
    //IPAddress subnet(255,255,255,0);
    //WiFi.config(ip, gateway, subnet);

    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP());

    //A partir de aqui vamos a resolver las solicitudes del cliente

    //---------------------Cuando Ingresen a nuestra página------------------------
    //Primera opcion
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    //Segunda opcion
    //server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    //--------Cuando nuestro archivo HTML solicite el archivo CSS Y JS------------------
    server.on("/estilos.css", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/estilos.css", "text/css");
    });
    server.on("/diseno.js", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/diseno.js", "text/javascript");
    });
    server.on("/https://cdn.anychart.com/releases/8.11.0/js/anychart-core.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/https://cdn.anychart.com/releases/8.11.0/js/anychart-core.min.js", "text/javascript");
    });
    server.on("/https://cdn.anychart.com/releases/8.11.0/js/anychart-polar.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/https://cdn.anychart.com/releases/8.11.0/js/anychart-polar.min.js", "text/javascript");
    });
    //-------------------------------API REST-------------------------------------
    server.on("/DATA", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(capacity,"application/json",JSON);
          
    });
        


    //-----------------Cuando pulsemos el botón de Iniciar-------------------------
    server.on("/ON", HTTP_GET, [](AsyncWebServerRequest * request){
      //Aqui enviaremos el codigo de la rutina al arduino
      String msnIniciar = "M302 S0 E040 F1";
      short i=0;
      short numOfElements = (msnIniciar.length());
      short grado=0;
      float resGrados=1.8;
      float intencidades[longitud]; 
      
      //Se envia el codigo G al arduino
      while (i<numOfElements){
        Serial.print(msnIniciar[i]);
        Serial2.write(msnIniciar[i]); //Write the serial data
        i++;
      }
      i=0;
            
      JsonArray arr = doc.to<JsonArray>();//Se inicializa un arreglo en el JsonDocument
      arr.clear();
      
      while(i<longitud){//Se lee la cantidad de datos que el usuario eligió por medio de la seoluión de su medida
        JsonObject objeto = arr.createNestedObject();        
        objeto["x"] = grado;
        //objeto["value"] = random(1,100);
        objeto["valor"] = analogRead(Photoresistor); 
        delay(200);
        i++;
        grado = grado+resGrados;
      }
      serializeJson(doc, JSON);  
      estado=true;
      request->send(SPIFFS, "/index.html", String(), false, processor);// Funcion que envia el estado del fotogoniometro

    });

    //----------------Cuando pulsemos el botón de apagar---------------------------
  
    server.on("/OFF", HTTP_GET, [](AsyncWebServerRequest * request) {
      String msnFinalizar= "G28 Y Z\n";             
      short i=0;
      short numOfElements = (msnFinalizar.length());
        
      //Se envia el codigo G al arduino
      while (i<numOfElements){
                Serial.print(msnFinalizar[i]);
        Serial2.write(msnFinalizar[i]); //Write the serial data
        i++;
      }      
      estado= false;
      request->send(SPIFFS, "/index.html", String(), false, processor); // Funcion que envia el estado del fotogoniometro

    });

    //-----------------Cuando mandamos un hipotenusa /SetText---------------------
    server.on("/SetText", HTTP_POST, [](AsyncWebServerRequest * request) {
      
      //Obtiene la hipotenusa
      String hipotenusa = request->arg("hipotenusa");             
      //Obtiene el ángulo
      String angulo = request->arg("angulo");   
      //Obtiene el ángulo y lo guarda en una variable global
      gradosDeSensado= request->arg("anguloFoco");   

<<<<<<< HEAD
      gradoSensor = 180 - (int)angulo - 90;
=======
      gradoSensor = 180 - angulo.toInt() - 90;
>>>>>>> b75105691db2aaaffe88ddd9db1a2ce01ddd1b2b

      miliSensor = (gradoSensor*40)/360;
      
      short i=0;
      float Z = cos(angulo.toInt()*3.1416/180)*hipotenusa.toInt();
      float Y = sin(angulo.toInt()*3.1416/180)*hipotenusa.toInt();
      if(Z > 8){
      
      String codigoG = "G0 Y";
<<<<<<< HEAD
      codigoG+=(String)x;
      codigoG+=" Z";
      codigoG+=(String)y;
      codigoG+=" X";
      codigoG+= (String)miliSensor;
=======
      codigoG+=(String)(Y-9);
      codigoG+=" Z";
      codigoG+=(String)Z;
      codigoG+=" X";
      codigoG+=(String)miliSensor;
>>>>>>> b75105691db2aaaffe88ddd9db1a2ce01ddd1b2b
      codigoG+='\n';
      short numOfElements = (codigoG.length());
      
      //Se envia el codigo G al arduino
      while (i<numOfElements){
        Serial.print(codigoG[i]);
        Serial2.write(codigoG[i]); //Write the serial data
        i++;
      }
    }
      request->redirect("/");
    });
    //-----------------Cuando mandamos un codigo libre /Libre---------------------
    server.on("/Libre", HTTP_POST, [](AsyncWebServerRequest * request) {      
      //Obtiene la hipotenusa
      String codigolibre = request->arg("codigog");                        
      short i=0;           
      codigolibre+='\n';
      short numOfElements = (codigolibre.length());
      
      //Se envia el codigo G al arduino
      while (i<numOfElements){
        Serial.print(codigolibre[i]);
        Serial2.write(codigolibre[i]); //Write the serial data
        i++;
      }
      i=0;
      request->redirect("/");
    });
    //-----------------Cuando mandamos un codigo libre /Libre---------------------
    server.on("/Libre", HTTP_POST, [](AsyncWebServerRequest * request) {      
      //Obtiene la hipotenusa
      String codigolibre = request->arg("codigog");                        
      short i=0;           
      codigolibre+='\n';
      short numOfElements = (codigolibre.length());
      
      //Se envia el codigo G al arduino
      while (i<numOfElements){
        Serial.print(codigolibre[i]);
        Serial2.write(codigolibre[i]); //Write the serial data
        i++;
      }
      i=0;
      request->redirect("/");
    });

    //Iniciamos servidor
    server.begin();

  }
  //Si despues de los 10 intentos no se conecta al servidor.
  else {
    Serial.println("");
    Serial.println("Error de conexion");
  }
}

/*void sendData(size_t capacity,String JSON){
  AsyncWebServerRequest * request;
  request->send(capacity,"text/plain",JSON);
  
}*/
void loop() {
  //No se usará debido a la librería del servidor web asíncrono
}
