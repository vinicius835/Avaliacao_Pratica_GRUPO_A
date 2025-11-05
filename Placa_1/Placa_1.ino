#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

const String SSID = "Nome da Rede";
const String PSWD = "Senha da Rede";
const String brokerUrl = "test.mosquitto.org";              
const int port = 1883;                     
const char* topico_1 = "placas/on/off/vini";
// MQTT

const String LWTMessage = "Offline";
const int  LWTQoS = 1;
const bool Retain_LWT = true;

//LWT

const byte trigg_pin_1 = 10;
const byte echo_pin_1 = 8;
// Ultra Sonico 1

const byte trigg_pin_2 = 12;
const byte echo_pin_2 = 20;
// Ultra Sonico 2


WiFiClient espClient;                //Criando Cliente WiFi
PubSubClient mqttClient(espClient);
WiFiClient EspClient;

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
Serial.println("Teste Serial OK!");
pinMode(trigg_pin_1, OUTPUT);
pinMode(echo_pin_1, INPUT);
//Ultra Sonico 1

pinMode(trigg_pin_2, OUTPUT);
pinMode(echo_pin_2, INPUT);
//Ultra Sonico 2
Serial.print("FOI");

}

void loop() {
  // put your main code here, to run repeatedly:
    digitalWrite(trigg_pin_1, LOW); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_1, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_1, LOW); 
    delayMicroseconds(10);
    unsigned long duracao_UL1 = pulseIn(echo_pin_1, HIGH);  
    int distancia_UL1= ((duracao_UL1 * 340)/2)/10000;

    digitalWrite(trigg_pin_2, LOW); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_2, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trigg_pin_2, LOW); 
    delayMicroseconds(10);
    unsigned long duracao_UL2 = pulseIn(echo_pin_2, HIGH);  
    int distancia_UL2= ((duracao_UL2 * 340)/2)/10000;

    Serial.println("ULTRA SONICO 1");
    Serial.println(distancia_UL1);
    delay(150);
    Serial.println("ULTRA SONICO 2");
    Serial.println(distancia_UL2);
}
void connectLocalworks() {
  Serial.println("Iniciando conexão com rede WiFi");
    WiFi.begin(SSID, PSWD);
    
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConectado!");
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
  doc["evento"] = "[EM_ANDAMENTO]";
  doc["timestamp"] = "[EM_ANDAMENTO]";
  serializeJson(doc, buffer);

  mqttClient.publish(topico_1, buffer, Retain_LWT);
  Serial.print("Conectado com sucesso!");
}
