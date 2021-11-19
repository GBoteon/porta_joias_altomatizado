#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Servo.h>

Servo myservo; 

int pos=0;

const char* ssid = "******************";
const char* password = "******************";
const char* mqtt_server = "broker.mqtt-dashboard.com";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String inString = "";
String senha = "";
int isObstacle = HIGH;

int green = 0;
int red = 2;
int sensor = 5;
int servo = 4;
    
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  inString="";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    inString+=(char)payload[i];
  }
  Serial.println();
  
  if (String(topic) == String("portajoia/senha")) {
    if (senha == inString){
      isObstacle = digitalRead(sensor);
      if (isObstacle == LOW) {
        digitalWrite(green, HIGH);
        for (pos = 0; pos <= 160; pos += 10) {
          myservo.write(pos);
        }
        delay(500);
        digitalWrite(green, LOW);
      } else {
        digitalWrite(green, HIGH);
        for (pos = 180; pos >= 0; pos -= 10) {
          myservo.write(pos);
        }
        delay(500);
        digitalWrite(green, LOW);
      }
    } else {
      digitalWrite(red, HIGH);
      delay(500);
      digitalWrite(red, LOW);
    }
  }
  
  if (String(topic) == String("portajoia/servo")) {
    pos=inString.toInt();
    myservo.write(pos);
  }
  
  if (String(topic) == String("portajoia/reset")) {
    isObstacle = digitalRead(sensor);
    if (isObstacle == LOW) {
       digitalWrite(green, HIGH);
       delay(500);
       digitalWrite(green, LOW);
       senha = String(inString);
    } else {
      digitalWrite(red, HIGH);
      delay(500);
      digitalWrite(red, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("portajoia/senha");
      client.subscribe("portajoia/servo");
      client.subscribe("portajoia/reset");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(green, OUTPUT);
  pinMode(sensor, INPUT);
  pinMode(red, OUTPUT);
  myservo.attach(servo);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    isObstacle = digitalRead(sensor);
    if (isObstacle == LOW) {
      client.publish("portajoia/sensor", "Aberta");
    } else {
      client.publish("portajoia/sensor", "Fechada");
    }
  }
}
