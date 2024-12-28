#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <vector>

#define MQTT_RECONNECT 10000L
#define MQTT_HEARTBEAT 60000L

class HOMIE_Property {
public:
  HOMIE_Property(String name);
  HOMIE_Property& description(String description);
  HOMIE_Property& settable(bool settable);
  HOMIE_Property& onlyDirect(bool direct);
  HOMIE_Property& datatype(String datatype);
  HOMIE_Property& format(String format);
  HOMIE_Property& ID(String id);
  HOMIE_Property& setSetCallback(void (*callback)(HOMIE_Property* prop, String msg));
  HOMIE_Property& setGetCallback(String (*callback)(HOMIE_Property* prop));
  String getName();
  String getDatatype();
  String getFormat();
  String getID();
  bool settable();
  void processSet(String topic, String msg);
  void sendValue(String topic, AsyncMqttClient* _mqttClient, bool direct = false);
  void sendRemoveConfig(String topic, AsyncMqttClient* _mqttClient);
  void sendConfig(String topic, AsyncMqttClient* _mqttClient);
private:
  String _name;  
  String _description;  
  String _ID;  
  bool _settable;
  String _datatype;  
  String _format;
  String _unit;
  bool _only_direct;

  void (*_set_callback)(HOMIE_Property* prop, String msg);
  String (*_get_callback)(HOMIE_Property* prop);
};

class HOMIE_Node {
public:
  HOMIE_Node(String name);
  HOMIE_Node& description(String description);
  HOMIE_Node& type(String type);
  void addProperty(HOMIE_Property* prop);
  String getName();
  void processSet(String topic, String msg);
  void sendValue(String topic, AsyncMqttClient* _mqttClient, HOMIE_Property* ident = nullptr);
  void sendRemoveConfig(String topic, AsyncMqttClient* _mqttClient);
  void sendConfig(String topic, AsyncMqttClient* _mqttClient);
private:
  String _name;
  String _description;
  String _type;
  std::vector<HOMIE_Property*> _properties;
};

class HOMIE_Device {
public:
  HOMIE_Device(String mqtt_server, uint16_t mqtt_port = 1883);
  HOMIE_Device& addNode(HOMIE_Node* node);
  bool removeNode(String name);
  HOMIE_Device& setFirmware(String fw);
  HOMIE_Device& setVersion(String ver);
  void begin(String name, String base = "homie");
  void setHeartbeatInterval(uint32_t hb_time_ms);
  void sendUpdates(HOMIE_Property* ident = nullptr);
  void connect();
//protected:
//  void publish_int(const char* subtopic, int data);
//  void publish_float(const char* subtopic, float data);
//  void addCallback(void (*func)(String topic, String msg));
private:
  static HOMIE_Device* getThis( HOMIE_Device* me = nullptr);
  static void s_connect();
  static void s_onConnect(bool sessionPresent);
  static void s_onDisconnect(AsyncMqttClientDisconnectReason reason);
  static void s_onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  static void s_Heartbeat();
  void onConnect(bool sessionPresent);
  //void onDisconnect(AsyncMqttClientDisconnectReason reason);
  void onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  void Heartbeat();
  void publishRemoveNode(HOMIE_Node* node);
  void publishConfig();
  void publish(String topic, const char* msg, bool retain = false);
  void publish(String topic, String msg, bool retain = false);
  void publish(String topic, int data, bool retain = false);
  //static MY_MQTT* __our_obj;
  char _mqtt_server[50];
  uint16_t _mqtt_port;
  String _name;
  String _firmware;
  String _version;
  String _base;
  String _fullbase;
  char _will_topic[50];
  uint32_t _heartbeat_interval_ms;
  AsyncMqttClient* _mqttClient;
  TimerHandle_t _mqttReconnectTimer;
  TimerHandle_t _mqttHeartbeatTimer;
  void (*_callback)(String topic, String msg);
  std::vector<HOMIE_Node*> _nodes;
};

#endif