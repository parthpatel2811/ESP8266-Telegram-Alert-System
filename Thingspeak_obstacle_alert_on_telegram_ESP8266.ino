#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <WiFiClientSecure.h>

#define EchoPin D3
#define TriggerPin D2
#define alertPin D1

const char* ssid = "Shubham parth";
const char* password = "Bssp@082827";

const char* botToken = "8469338799:AAH9W9BRaUGCWeU0IdxZ3lvaMPT2BIkg5e8";
const char* chatID   = "5889537133";

WiFiClient  client;
WiFiClientSecure tgClient;

unsigned long myChannelNumber = 3350941;
const char * myWriteAPIKey = "P84FJ1PW7M04EK1A";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(EchoPin,INPUT);
  pinMode(TriggerPin,OUTPUT);
  pinMode(alertPin,OUTPUT);

  WiFi.begin(ssid,password);
   Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  ThingSpeak.begin(client);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(TriggerPin,LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin,LOW);

  long duration = pulseIn(EchoPin, HIGH, 30000);
  float distance = (0.0342*duration)/2;
  delay(1000);
  Serial.print("Distance: ");
  Serial.println(distance);
  
  static bool alertSent = false;   // prevents spam
  int status; 
  if(distance<=10){
    status = 1;
    Serial.println("ALERT! Obstacle");
    digitalWrite(alertPin,HIGH);
    if (!alertSent) {              // send only once
    sendTelegram(distance, status);
    alertSent = true;
  }
  }
  else{
    digitalWrite(alertPin, LOW);
    status = 0;
    if (alertSent) {
      sendTelegram(distance, 0);   // SAFE message
      alertSent = false;
    }
  }
}

void sendTelegram(float distance, int status) {
  tgClient.setInsecure();  // skip SSL cert (OK for this use)
  String URL_send;
  if (!tgClient.connect("api.telegram.org", 443)) {
    Serial.println("Telegram connect failed");
    return;
  }
  if(distance<=10){
    String msg = "🚨Obstacle Detected!%0A";
    msg += "Distance: " + String(distance, 1) + " cm%0A";
    msg += "Status: " + String(status);

    String url = "/bot" + String(botToken) +
               "/sendMessage?chat_id=" + String(chatID) +
               "&text=" + msg;
    URL_send = url; 
  }

   else{
    String msg = "SAFE%0A";
    msg += "Distance: " + String(distance, 1) + " cm%0A";
    msg += "Status: " + String(status);

    String url = "/bot" + String(botToken) +
               "/sendMessage?chat_id=" + String(chatID) +
               "&text=" + msg;
    URL_send = url; 
  }
  tgClient.print(String("GET ") + URL_send + " HTTP/1.1\r\n" +
                 "Host: api.telegram.org\r\n" +
                 "Connection: close\r\n\r\n");

  Serial.println("Telegram sent");
}
