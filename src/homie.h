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
  HOMIE_Property(const char* name);
  HOMIE_Property& description(const char* description);
  HOMIE_Property& settable(bool settable);
  HOMIE_Property& retained(bool retained);
  HOMIE_Property& onlyDirect(bool direct);
  HOMIE_Property& datatype(const char* datatype);
  HOMIE_Property& format(const char* format);
  HOMIE_Property& unit(const char* unit);
  HOMIE_Property& ID(const char* id);
  HOMIE_Property& setSetCallback(void (*callback)(HOMIE_Property* prop, const char* msg));
  HOMIE_Property& setGetCallback(bool (*callback)(HOMIE_Property* prop, char* result));
  const char* getName();
  const char* getDatatype();
  const char* getFormat();
  const char* getUnit();
  const char* getID();
  bool settable();
  bool retained();
  void processSet(const char* topic, const char* msg);
  void publishValue(const char* topic, AsyncMqttClient* _mqttClient, bool direct = false);
  void unpublishConfig(const char* topic, AsyncMqttClient* _mqttClient);
  void publishConfig(const char* topic, AsyncMqttClient* _mqttClient);
private:
  void sub_publish(AsyncMqttClient* mqttclient, const char* topic, const char* subtopic, const char* msg, bool retain = false);
  char _name[20];  
  char _description[40];  
  char _ID[20];  
  bool _settable;
  bool _retained;
  char _datatype[15];  
  char _format[80];
  char _unit[10];
  bool _only_direct;

  void (*_set_callback)(HOMIE_Property* prop, const char* msg);
  bool (*_get_callback)(HOMIE_Property* prop, char* result);
};

class HOMIE_Node {
public:
  HOMIE_Node(const char* name);
  HOMIE_Node& description(const char* description);
  HOMIE_Node& type(const char* type);
  void addProperty(HOMIE_Property* prop);
  void removeProperties();
  const char* getName();
  void processSet(const char* topic, const char* msg);
  void publishValue(const char* topic, AsyncMqttClient* mqttClient, HOMIE_Property* ident = nullptr);
  void unpublishConfig(const char* topic, AsyncMqttClient* mqttClient);
  void publishConfig(const char* topic, AsyncMqttClient* mqttClient);
private:
  void sub_publish(AsyncMqttClient* mqttclient, const char* topic, const char* subtopic, const char* msg, bool retain = false);
  char _name[20];
  char _description[40];
  char _type[20];
  std::vector<HOMIE_Property*> _properties;
};

class HOMIE_Device {
public:
  HOMIE_Device(const char* mqtt_server, uint16_t mqtt_port = 1883);
  HOMIE_Device& addNode(HOMIE_Node* node);
  bool removeNode(const char* name);
  HOMIE_Device& setFirmware(const char* fw);
  HOMIE_Device& setVersion(const char* ver);
  void begin(const char* name, const char* base = "homie");
  void setHeartbeatInterval(uint32_t hb_time_ms);
  void sendUpdates(HOMIE_Property* ident = nullptr);
  void connect();
  bool connected();
  int reconnectCount();
  void publishConfig();
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
  void sub_publish(const char* subtopic, const char* msg, bool retain = false);
  void sub_publish(const char* subtopic, int data, bool retain = false);
  //static MY_MQTT* __our_obj;
  char _mqtt_server[50];
  uint16_t _mqtt_port;
  AsyncMqttClient* _mqttClient;
  int _reconnect_count;
  char _name[20];
  char _firmware[20];
  char _version[10];
  char _fullbase[30];
  char _will_topic[50];
  const char* _last_will = "lost";
  uint32_t _heartbeat_interval_ms;
  TimerHandle_t _mqttReconnectTimer;
  TimerHandle_t _mqttHeartbeatTimer;
  std::vector<HOMIE_Node*> _nodes;
};

#endif