#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
const String SSID = "Vini@@";
const String PSWD = "viniciusA";
const String brokerUrl = "test.mosquitto.org";              
const int port = 1883;                     
const char* topico_1 = "placas/on/off/vini";
const char* topico_2 = "vini/topic/placa1/envia";

// MQTT

const String LWTMessage = "Offline";
const int  LWTQoS = 1;
const bool Retain_LWT = true;

//LWT

  const byte trigg_pin_1 = 22;
  const byte echo_pin_1 = 18;
  // Ultra Sonico 1

  const byte trigg_pin_2 = 15;
  const byte echo_pin_2 = 23;
  // Ultra Sonico 2

int array_distancia_UL1[2];
int array_distancia_UL2[2];
int tempo_UL1[2];
int tempo_UL2[2];
bool primeira_vez = true;
//Ultra Sonicos

WiFiClient espClient;
//WiFi

PubSubClient mqttClient(espClient);
//MQTT

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);
//NTPClient

unsigned long atual_millis = 0;
unsigned long resetor_millis = 0;

//Millis

void connectLocalworks();
void connectBroker();
void PublishOnNodeRED();
void setup() {

Serial.begin(115200);
connectLocalworks();
connectBroker();

Serial.println("Teste Serial OK!");

delay(100);

pinMode(trigg_pin_1, OUTPUT);
pinMode(echo_pin_1, INPUT);
//Ultra Sonico 1

Serial.println("Ultra Sonico 1 - OK");
delay(100);
pinMode(trigg_pin_2, OUTPUT);
pinMode(echo_pin_2, INPUT);
Serial.println("Ultra Sonico 2 - OK");
//Ultra Sonico 2

timeClient.begin();
timeClient.setTimeOffset(-10000);
//NPTClient

}

void loop() {


  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conexão Perdida\n");
    connectLocalworks();
    
  }
  // Reconexão do WiFi
  if (!mqttClient.connected()) {
    Serial.println("Erro de Conexão no Broker");
    connectBroker();
  }
  // Reconexão do Broker

    digitalWrite(trigg_pin_1, LOW); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_1, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_1, LOW); 
    delayMicroseconds(10);

    unsigned long duracao_UL1 = pulseIn(echo_pin_1, HIGH);  
    int distancia_UL1= ((duracao_UL1 * 340)/2)/10000;
    // Ultra Sonico 1

    digitalWrite(trigg_pin_2, LOW); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_2, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_2, LOW); 
    delayMicroseconds(10);

    unsigned long duracao_UL2 = pulseIn(echo_pin_2, HIGH);  
    int distancia_UL2= ((duracao_UL2 * 340)/2)/10000;
    // Ultra Sonico 2
     if(primeira_vez == true){
      array_distancia_UL1[0] = distancia_UL1;
      tempo_UL1[0] = millis();
      array_distancia_UL1[1] = array_distancia_UL1[0];
      tempo_UL1[1] = tempo_UL1[0];

      array_distancia_UL2[0] = distancia_UL2;
      tempo_UL2[0] = millis();
      array_distancia_UL2[1] = array_distancia_UL2[0];
      tempo_UL2[1] = tempo_UL2[0];
      primeira_vez = false;
    }
    else{
      array_distancia_UL1[1] = array_distancia_UL1[0];
      tempo_UL1[1] = tempo_UL1[0];
      array_distancia_UL1[0] = distancia_UL1;
      tempo_UL1[0] = millis();

      array_distancia_UL2[1] = array_distancia_UL2[0];
      tempo_UL2[1] = tempo_UL2[0];
      array_distancia_UL2[0] = distancia_UL2;
      tempo_UL2[0] = millis();
    }

    int tempo_total_UL1 = (tempo_UL1[1] - tempo_UL1[0]);
    int tempo_total_UL2 = (tempo_UL2[1] - tempo_UL2[0]);

    byte desvio_UL1 = (array_distancia_UL1[1] - array_distancia_UL1[0])/tempo_total_UL1;
    byte desvio_UL2 = (array_distancia_UL2[1] - array_distancia_UL2[0])/tempo_total_UL2;

    Serial.println("ULTRA SONICO 1");
    Serial.println(desvio_UL1);

    Serial.println("ULTRA SONICO 2");
    Serial.println(desvio_UL2);

    if(desvio_UL1 < desvio_UL2 ){
        timeClient.update();
        String evento = "Entrando";
        String timestamp = timeClient.getFormattedTime();
        Serial.println(timestamp);
        PublishOnNodeRED(evento,timestamp);
    }
    else if(desvio_UL2 < desvio_UL1){
        timeClient.update();
        String evento = "Saindo";
        String timestamp = timeClient.getFormattedTime();
        PublishOnNodeRED(evento,timestamp);
      
    }
    // Fechar do Publicar no Broker

}   
// Fechar do Loop
void connectLocalworks() {
  Serial.println("Iniciando conexão com rede WiFi");
    WiFi.begin(SSID, PSWD);
    
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConectado!");
}
void PublishOnNodeRED(String teste1, String teste2) {
  // Serial.println("Conectando ao broker");
  mqttClient.setServer(brokerUrl.c_str(), port);
  String userId = "ESP-alves9";
  userId += String(random(0xffff), HEX);
  // json
  StaticJsonDocument<200>doc;
  
  char nodeRED[200];
  
  while (!mqttClient.connected()) {
    mqttClient.connect(
      userId.c_str(),
      "",
      "",
      topico_2,
      LWTQoS,
      Retain_LWT,
      nodeRED
    );
    Serial.println(".");
    
  delay(5000);
  }

  doc["evento"] = teste1;
  doc["timestamp"] = teste2;
  serializeJson(doc,nodeRED);

  mqttClient.publish(topico_2, nodeRED, Retain_LWT);
  // Serial.print("Enviado ao Node-RED!");
}

void connectBroker() {
  Serial.println("Conectando ao broker");
  mqttClient.setServer(brokerUrl.c_str(), port);
  String userId = "ESP-alves9";
  userId += String(random(0xffff), HEX);

  // json
  StaticJsonDocument<200>doc;
    doc["status"] =LWTMessage;
    char buffer[200];
    serializeJson(doc,buffer);

    
  while (!mqttClient.connected()) {
    mqttClient.connect(
      userId.c_str(),
      "",
      "",
      topico_1,
      LWTQoS,
      Retain_LWT,
      buffer
    );
    Serial.println(".");
    
  delay(5000);
  }

  doc["status"] = "Online";

  serializeJson(doc, buffer);

  mqttClient.publish(topico_1, buffer, Retain_LWT);
  Serial.print("Conectado com sucesso!");
}
