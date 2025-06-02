
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <ArduinoJson.h>

const int DHT_PIN = 15;
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;
const int BUZZER_PIN = 27;
const int LED_PIN = 26;

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "safezone/sensores/001";

WiFiClient espClient;
PubSubClient client(espClient);
DHTesp dht;

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32Client_SAFEZONE")) {
      Serial.println("Conectado ao broker MQTT");
    } else {
      delay(1000);
    }
  }
}

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH);
  return duracao * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  connectWiFi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  TempAndHumidity data = dht.getTempAndHumidity();
  float temperatura = data.temperature;
  float umidade = data.humidity;
  float distancia = medirDistancia();
  bool alerta = temperatura > 30.0 || umidade < 40.0 || distancia < 50.0;

  digitalWrite(BUZZER_PIN, alerta);
  digitalWrite(LED_PIN, alerta);

  StaticJsonDocument<256> jsonDoc;
  jsonDoc["id"] = "sensor-001";
  jsonDoc["temperatura"] = temperatura;
  jsonDoc["umidade"] = umidade;
  jsonDoc["nivel"] = distancia;
  jsonDoc["alerta"] = alerta;

    char buffer[256];
  serializeJson(jsonDoc, buffer);
  client.publish(mqtt_topic, buffer);

  Serial.println("ID: sensor-001");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println(" %");

  Serial.print("Nível do Rio: ");
  Serial.print(distancia, 2);
  Serial.println(" cm");

  Serial.print("Alerta: ");
  Serial.println(alerta ? "ATIVO" : "Normal");

  Serial.println("--------------------------");

  delay(5000);

}
