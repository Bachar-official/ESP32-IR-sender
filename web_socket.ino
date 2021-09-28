#include <ESPmDNS.h>
#include <WiFi.h>
#include <IRremote.h>
#include <WiFiUdp.h>
#include "ESPAsyncWebServer.h"
#include "Conditioner.h"

#define WIFI_SSID "SSD"
#define WIFI_PASS "password"
#define HOSTNAME "kitchen"
#define PORT 80
#define SERVER_PORT 1337
#define UDP_PORT 4210

//300
uint16_t profileOff[73] = {9030,4420, 480,1720, 530,570, 530,570, 530,570, 530,570, 480,620, 480,620, 480,620, 480,570, 530,570, 530,570, 530,1720, 480,620, 480,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 480,620, 480,620, 480,620, 480,1720, 480,620, 480,1720, 530,570, 530,570, 480,1720, 530,570, 530};
//200
uint16_t profileCold17[73] = {9030,4370, 530,1720, 480,620, 480,570, 530,1720, 480,620, 480,620, 480,570, 530,570, 530,1720, 480,620, 480,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 480,620, 480,620, 480,620, 480,570, 530,570, 530,1720, 480,620, 480,1720, 480,620, 480,620, 480,1720, 530,570, 480};
//201
uint16_t profileCold22[73] = {9030,4370, 530,1720, 480,620, 480,570, 530,1720, 480,620, 480,620, 480,570, 530,570, 530,570, 530,1720, 480,1720, 480,620, 480,620, 480,620, 480,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 480,620, 480,1720, 530,570, 530,1670, 530,570, 530,570, 530,1670, 530,570, 530};
//202
uint16_t profileHot30[73] = {9030,4420, 530,570, 530,570, 480,1720, 530,1670, 530,570, 530,570, 530,570, 530,570, 530,570, 530,1670, 530,1670, 530,1670, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 480,620, 480,620, 480,620, 480,570, 530,570, 530,1720, 480,620, 480,1720, 530,570, 480,620, 480,1720, 530,570, 530};
//100
uint16_t profileAuto[73] = {9080,4370, 530,570, 530,570, 530,570, 530,1670, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 480,620, 480,620, 480,620, 480,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,1670, 530,570, 530,1670, 530,570, 530,570, 530,1670, 530,570, 530};
//101
uint16_t profileFan[73] = {9030,4420, 530,1670, 530,1670, 530,570, 530,1670, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,570, 480,620, 480,620, 480,620, 480,570, 530,570, 530,570, 530,570, 530,570, 530,570, 530,1670, 530,570, 530,570, 530,570, 530,1670, 530,570, 530,1720, 480,570, 530,570, 530,1720, 480,620, 480};

WiFiUDP udp;
AsyncWebServer server(SERVER_PORT);
Conditioner cond("Kitchen", 300, WiFi.localIP().toString() + ":" + String(SERVER_PORT, DEC));
char incomingPacket[255]; //buffer for incoming packets
String ping = "ping"; // answer for this message

void setup() {  
  Serial.begin(115200);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IrSender.begin(4, ENABLE_LED_FEEDBACK);
  cond.setIp(WiFi.localIP().toString() + ":" + String(SERVER_PORT, DEC));
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  Serial.println(WiFi.localIP());

  udp.begin(SERVER_PORT);
  Serial.println("Listening for broadcast packets");
  
  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", PORT);

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    int profile = 0;
    String date = "";
    String user = "";
    AsyncWebParameter* p0 = request->getParam(0);
    AsyncWebParameter* p1 = request->getParam(1);
    AsyncWebParameter* p2 = request->getParam(2);
    if (p0->name() == "profile") {
      profile = p0->value().toInt();
      sendProfile(profile);
      request->send(200, "application/json", cond.operation(profile));
    }
    if (p1->name() == "date") {
      date = p1->value();
      cond.setDate(date);
    }
    if (p2->name() == "user") {
      user = p2->value();
      cond.setUser(user);
    }
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest * request) {
    String date = "";
    String user = "";
    AsyncWebParameter* p0 = request->getParam(0);
    AsyncWebParameter* p1 = request->getParam(1);
    if (p0->name() == "date") {
      date = p0->value();
      cond.setDate(date);
    }
    if (p1->name() == "user") {
      user = p1->value();
      cond.setUser(user);
    }
    IrSender.sendRaw(profileOff, sizeof(profileOff) / sizeof(profileOff[0]), 38);
    request->send(200, "application/json", cond.operation(300));
  });
}

void sendProfile (int profile) {
  switch(profile) {
    case 200: IrSender.sendRaw(profileCold17, sizeof(profileCold17) / sizeof(profileCold17[0]), 38); break;
    case 201: IrSender.sendRaw(profileCold22, sizeof(profileCold22) / sizeof(profileCold22[0]), 38); break;
    case 202: IrSender.sendRaw(profileHot30, sizeof(profileHot30) / sizeof(profileHot30[0]), 38); break;
    case 100: IrSender.sendRaw(profileAuto, sizeof(profileAuto) / sizeof(profileAuto[0]), 38); break;
    case 101: IrSender.sendRaw(profileFan, sizeof(profileFan) / sizeof(profileFan[0]), 38); break;
    default: break;
  }
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    Serial.printf("received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    if(String(incomingPacket) == ping)
    {
      Serial.printf("Answering to %s\n", udp.remoteIP().toString().c_str()); 
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.print(cond.response());
      udp.endPacket();
    }    
  }
}
