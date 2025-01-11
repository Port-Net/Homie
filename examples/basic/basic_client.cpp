#include <Arduino.h>
#include "homie.h"

#define VERSION "0.1"
#define MY_MQTT_SERVER "192.168.1.2"

HOMIE_Device homie(MY_MQTT_SERVER);

void homie_set_callback(HOMIE_Property* prop, const char* msg) {
  String propname(prop->getName());
  String msgstring(msg);
  if(propname == "state") {
    if(msgstring == "True") {
      Serial.println("Switched on");
    } else {
      Serial.println("Switched off");
    }
  } else if(propname == "brightness") {
    int br = msgstring.toInt();
    Serial.printf("Brightness set to %d\r\n", br);
  } else if(propname == "color") {
    int col = msgstring.toInt();
    Serial.printf("Color set to %d\r\n", col);
  }
}

bool homie_get_callback(HOMIE_Property* prop, char* buffer) {
  String propname(prop->getName());
  if(propname == "state") {
    strcpy(buffer, "true");
    return true;
  } else if(prop->getName() == "brightness") {
    sprintf(buffer, "%d", 42);
    return true;
  } else if(prop->getName() == "color") {
    strcpy(buffer, String(4711).c_str());
    return true;
  }
  return false;
}

bool homie_get_test_callback(HOMIE_Property* prop, char* buffer) {
  __unused(prop);
  return true;
  strcpy(buffer, "testvalue");
}

void setup() {
  
  //setup wifi

  HOMIE_Node* node;
  HOMIE_Property* prop;
  node = new HOMIE_Node("light");
  prop = new HOMIE_Property("state");
  prop->description("Light power state").settable(true).datatype("bool").setSetCallback(homie_set_callback);
  node->addProperty(prop);
  prop = new HOMIE_Property("brightness");
  prop->description("Brightnes ").settable(true).datatype("integer").setSetCallback(homie_set_callback);
  node->addProperty(prop);
  prop = new HOMIE_Property("temperature");
  prop->description("Color temperature").settable(true).datatype("integer");
  prop->setGetCallback(homie_get_callback).setSetCallback(homie_set_callback);
  node->addProperty(prop);
  homie.addNode(node);

  node = new HOMIE_Node("test");
  node->description("Some Infos");    
  prop = new HOMIE_Property("testprop");
  prop->setGetCallback(homie_get_test_callback);
  node->addProperty(prop);
  homie.addNode(node);

  homie.setFirmware("ESP32-fw");
  homie.setVersion(VERSION);
  homie.begin("lighttest");

}

void loop() {
  // put your main code here, to run repeatedly:
}
