
#include <Arduino.h>
#include "homie.h"
//#include "raffstore.h"
//#include <ArduinoJson.h>
//#include <YAMLDuino.h>

//AsyncMqttClient mqttClient;
//TimerHandle_t mqttReconnectTimer;
//TimerHandle_t mqttHeartbeatTimer;

HOMIE_Property::HOMIE_Property(const char* name) {
  strncpy(_name, name, sizeof(_name));
  for(int i = 0; i < strlen(_name); ++i) {
    _name[i] = tolower(_name[i]);
  }
  description(name);
  _settable = false;
  _retained = true;
  datatype("integer");
  format("");
  unit("");
  ID(_name);
  _only_direct = false;
}

HOMIE_Property& HOMIE_Property::description(const char* description) {
  strncpy(_description, description, sizeof(_description));
  return *this;
}

HOMIE_Property& HOMIE_Property::settable(bool settable) {
  _settable = settable;
  return *this;
}

HOMIE_Property& HOMIE_Property::retained(bool retained) {
  _retained = retained;
  return *this;
}

HOMIE_Property& HOMIE_Property::onlyDirect(bool direct) {
  _only_direct = direct;
  return *this;
}

HOMIE_Property& HOMIE_Property::datatype(const char* datatype) {
  strncpy(_datatype, datatype, sizeof(_datatype));
  return *this;
}

HOMIE_Property& HOMIE_Property::format(const char* format) {
  strncpy(_format, format, sizeof(_format));
  return *this;
}

HOMIE_Property& HOMIE_Property::unit(const char* unit) {
  strncpy(_unit, unit, sizeof(_unit));
  return *this;
}

HOMIE_Property& HOMIE_Property::ID(const char* id) {
  strncpy(_ID, id, sizeof(_ID));
  return *this;
}

HOMIE_Property& HOMIE_Property::setSetCallback(void (*callback)(HOMIE_Property* prop, const char* msg)) {
  _set_callback = callback;
  return *this;
}

HOMIE_Property& HOMIE_Property::setGetCallback(bool (*callback)(HOMIE_Property* prop, char* result)) {
  _get_callback = callback;
  return *this;
}

const char* HOMIE_Property::getName() {
  return _name;
}

const char* HOMIE_Property::getDatatype() {
  return _datatype;
}

const char* HOMIE_Property::getFormat() {
  return _format;
}

const char* HOMIE_Property::getUnit() {
  return _unit;
}

const char* HOMIE_Property::getID() {
  return(_ID);
}

bool HOMIE_Property::settable() {
  return _settable;
}

bool HOMIE_Property::retained() {
  return _retained;
}

void HOMIE_Property::processSet(const char* topic, const char* msg) {
  char* p = strchr(topic, '/');
  if(!p) {
    return;
  }
  int len = p - topic;
  if(strlen(_name) != len) {
    return;
  }
  if(strncmp(topic, _name, p - topic)) {
    return;
  }
  if(strncmp(p, "/set", 4)) {
    return;
  }
  if(_settable && _set_callback) {
    _set_callback(this, msg);
  }
}

void HOMIE_Property::publishValue(const char* topic, AsyncMqttClient* mqttClient, bool direct) {
  if(_only_direct && !direct) {
    return;
  }
  char my_topic[strlen(topic) + strlen(_name) + 2];
  strncpy(my_topic, topic, sizeof(my_topic));
  strncat(my_topic, "/", sizeof(my_topic) - strlen(my_topic));
  strncat(my_topic, _name, sizeof(my_topic) - strlen(my_topic));
  if(_get_callback) {
    char result_msg[100];
    if(_get_callback(this, result_msg)) {
      mqttClient->publish(my_topic, 1, _retained, result_msg);
    }
  }
}

void HOMIE_Property::unpublishConfig(const char* topic, AsyncMqttClient* mqttClient) {
  char my_topic[strlen(topic) + strlen(_name) + 2];
  strncpy(my_topic, topic, sizeof(my_topic));
  strncat(my_topic, "/", sizeof(my_topic) - strlen(my_topic));
  strncat(my_topic, _name, sizeof(my_topic) - strlen(my_topic));
  sub_publish(mqttClient, my_topic, "/$name", "", true);
  sub_publish(mqttClient, my_topic, "/$settable", "", true);
  sub_publish(mqttClient, my_topic, "/$datatype", "", true);
  if(!_retained) {
    sub_publish(mqttClient, my_topic, "/$retained", "", true);
  }
  if(_format != "") {
    sub_publish(mqttClient, my_topic, "/$format", "", true);
  }
  if(_unit != "") {
    sub_publish(mqttClient, my_topic, "/$unit", "", true);
  }
}

void HOMIE_Property::publishConfig(const char* topic, AsyncMqttClient* mqttClient) {
  char my_topic[strlen(topic) + strlen(_name) + 6]; //also space for "/set"
  strncpy(my_topic, topic, sizeof(my_topic));
  strncat(my_topic, "/", sizeof(my_topic) - strlen(my_topic));
  strncat(my_topic, _name, sizeof(my_topic) - strlen(my_topic));
  sub_publish(mqttClient, my_topic, "/$name", _description, true);
  sub_publish(mqttClient, my_topic, "/$settable", _settable ? "true" : "false", true);
  if(!_retained) {
    sub_publish(mqttClient, my_topic, "/$retained", "false", true);
  }
  sub_publish(mqttClient, my_topic, "/$datatype", _datatype, true);
  if(_format != "") {
    sub_publish(mqttClient, my_topic, "/$format", _format, true);
  }
  if(_unit != "") {
    sub_publish(mqttClient, my_topic, "/$unit", _unit, true);
  }
  if(_settable) {
    strncat(my_topic, "/set", sizeof(my_topic) - strlen(my_topic));
    mqttClient->subscribe(my_topic, 1);
  }

}

void HOMIE_Property::sub_publish(AsyncMqttClient* mqttClient, const char* topic, const char* subtopic, const char* msg, bool retain) {
  if (!mqttClient->connected()) {
    return;
  }
  char new_subtopic[strlen(topic) + strlen(subtopic) + 2];
  strncpy(new_subtopic, topic, sizeof(new_subtopic));
  strncat(new_subtopic, subtopic, sizeof(new_subtopic) - strlen(new_subtopic));
  mqttClient->publish(new_subtopic, 1, retain, msg);
}

//////////////////////////////////////

HOMIE_Node::HOMIE_Node(const char* name) {
  strncpy(_name, name, sizeof(_name));
  for(int i = 0; i < strlen(_name); ++i) {
    _name[i] = tolower(_name[i]);
  }
  description(name);
  type("");
}

HOMIE_Node& HOMIE_Node::description(const char* name) {
  strncpy(_description, name, sizeof(_description));
  return *this;
}

HOMIE_Node& HOMIE_Node::type(const char* type) {
  strncpy(_type, type, sizeof(_type));
  return *this;
}

void HOMIE_Node::addProperty(HOMIE_Property* prop) {
  _properties.push_back(prop);
}

void HOMIE_Node::removeProperties() {
  for(auto it : _properties) {
    delete it;
  }
  _properties.clear();
}

const char* HOMIE_Node::getName() {
  return(_name);
}

void HOMIE_Node::processSet(const char* topic, const char* msg) {
  char* p = strchr(topic, '/');
  //int idx = topic.indexOf("/");
  if(!p) {
    return;
  }
  int len = p - topic;
  if(strlen(_name) != len) {
    return;
  }
  if(strncmp(topic, _name, p - topic)) {
    return;
  }
  for(auto it : _properties) {
    it->processSet(p + 1, msg);
  }
}

void HOMIE_Node::publishValue(const char* topic, AsyncMqttClient* mqttClient, HOMIE_Property* ident) {
  char new_topic[strlen(topic) + strlen(_name) + 2];
  strncpy(new_topic, topic, sizeof(new_topic));
  strncat(new_topic, "/", sizeof(new_topic) - strlen(new_topic));
  strncat(new_topic, _name, sizeof(new_topic) - strlen(new_topic));
  for(auto it : _properties) {
    if(!ident) {
      it->publishValue(new_topic, mqttClient);
    } else if(ident == it) {
      it->publishValue(new_topic, mqttClient, true);
    }
  }
}

void HOMIE_Node::unpublishConfig(const char* topic, AsyncMqttClient* mqttClient) {
  char new_topic[strlen(topic) + strlen(_name) + 2];
  strncpy(new_topic, topic, sizeof(new_topic));
  strncat(new_topic, "/", sizeof(new_topic) - strlen(new_topic));
  strncat(new_topic, _name, sizeof(new_topic) - strlen(new_topic));
  //strncat(subtopic, )
  sub_publish(mqttClient, new_topic, "/$name", "", true);
  sub_publish(mqttClient, new_topic, "/$type", "", true);
  sub_publish(mqttClient, new_topic, "/$properties", "", true);
  for(auto it : _properties) {
    it->unpublishConfig(new_topic, mqttClient);
  }
}

void HOMIE_Node::publishConfig(const char* topic, AsyncMqttClient* mqttClient) {
  char new_topic[strlen(topic) + strlen(_name) + 2];
  strncpy(new_topic, topic, sizeof(new_topic));
  strncat(new_topic, "/", sizeof(new_topic) - strlen(new_topic));
  strncat(new_topic, _name, sizeof(new_topic) - strlen(new_topic));
  sub_publish(mqttClient, new_topic, "/$name", _description, true);
  sub_publish(mqttClient, new_topic, "/$type", _type, true);
  int l = 0;
  for(auto it : _properties) {
    l += strlen(it->getName()) + 1;
  }
  char props[l];
  bool first = true;
  for(auto it : _properties) {
    if(!first) {
      strncpy(props, ",", sizeof(props));
    }
    strncpy(props, it->getName(), sizeof(props));
    first  = false;
  }

  sub_publish(mqttClient, new_topic, "/$properties", props, true);
  for(auto it : _properties) {
    it->publishConfig(new_topic, mqttClient);
  }
}

void HOMIE_Node::sub_publish(AsyncMqttClient* mqttClient, const char* topic, const char* subtopic, const char* msg, bool retain) {
  if (!mqttClient->connected()) {
    return;
  }
  char new_subtopic[strlen(topic) + strlen(subtopic) + 2];
  strncpy(new_subtopic, topic, sizeof(new_subtopic));
  strncat(new_subtopic, subtopic, sizeof(new_subtopic) - strlen(new_subtopic));
  mqttClient->publish(new_subtopic, 1, retain, msg);
}


////////////////////////////////////////

HOMIE_Device::HOMIE_Device(const char* mqtt_server, uint16_t mqtt_port) : _mqtt_port(mqtt_port) {
  getThis(this);
  strncpy(_mqtt_server, mqtt_server, sizeof(_mqtt_server)); // we need permanent allocated char*
  _mqttClient = nullptr;
  _heartbeat_interval_ms = MQTT_HEARTBEAT;
  _reconnect_count = 0;
}

HOMIE_Device& HOMIE_Device::addNode(HOMIE_Node* node) {
  _nodes.push_back(node);
  return *this;
}

bool HOMIE_Device::removeNode(const char* name) {
  HOMIE_Node* p = nullptr;
  //auto p = std::find_if(_nodes.begin(), _nodes.end(), [&name] (const HOMIE_Node &n) { return n.getName() == name; });
  for(auto it : _nodes) {
    if(it->getName() == name) {
      p = it;
    }
  }
  if(!p) {
    return false;
  }
  p->unpublishConfig(_fullbase, _mqttClient);
  p->removeProperties();
  /*
  std::erase_if(_nodes, [&name] (const HOMIE_Node& n) { return n.getName() == name; });
  */
  std::erase(_nodes, p);
  delete p;
  publishConfig();
  return true;
}

HOMIE_Device& HOMIE_Device::setFirmware(const char* fw) {
  strncpy(_firmware, fw, sizeof(_firmware));
  return *this;
}

HOMIE_Device& HOMIE_Device::setVersion(const char* ver) {
  strncpy(_version, ver, sizeof(_version));
  return *this;
}

// create a static object pointer
HOMIE_Device* HOMIE_Device::getThis(HOMIE_Device* me) {
  static HOMIE_Device* p_me;
  if(me) {
    p_me = me;
  }
  return p_me;
}

void HOMIE_Device::s_onConnect(bool sessionPresent) {
  getThis()->onConnect(sessionPresent);
}

void HOMIE_Device::onConnect(bool sessionPresent) {
  //Serial.println("Connected to MQTT.");
  //Serial.print("Session present: ");
  //Serial.println(sessionPresent);
  publishConfig();
  for(auto it : _nodes) {
    it->publishValue(_fullbase, _mqttClient);
  }
  xTimerStart(_mqttHeartbeatTimer, 0);
  _reconnect_count = 0;
}

void HOMIE_Device::s_onDisconnect(AsyncMqttClientDisconnectReason reason) {
  xTimerStop(getThis()->_mqttHeartbeatTimer, 0);
}

void HOMIE_Device::s_onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  getThis()->onMessage(topic, payload, properties, len, index, total);
}

void HOMIE_Device::onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if(!len) {
    return;
  }
  char pl[len+1];
  pl[len] = '\0';
  strncpy(pl, payload, len);
  if(strcmp(topic, _fullbase)) {
    return;
  }
  char new_topic[strlen(topic) - strlen(_fullbase) + 1];
  strncpy(new_topic, &topic[strlen(_fullbase) + 1], sizeof(new_topic));
  for(auto it : _nodes) {
    it->processSet(new_topic, pl);
  }
}

void HOMIE_Device::s_Heartbeat() {
  getThis()->Heartbeat();
}

void HOMIE_Device::Heartbeat() {
  if (_mqttClient->connected()) {
    sub_publish("/$stats/uptime", millis()/1000, true);
    // TODO handle additional stats
  } else {
    if (WiFi.isConnected()) {
      _reconnect_count++;
      if(_reconnect_count > 10) {
        delete _mqttClient;
        _mqttClient = new AsyncMqttClient();
        _mqttClient->setServer(_mqtt_server, _mqtt_port);
        _mqttClient->setWill(_will_topic, 1, true, _last_will);
        _mqttClient->onConnect(s_onConnect);
        _mqttClient->onDisconnect(s_onDisconnect);
        _mqttClient->onMessage(s_onMessage);
      }
      _mqttClient->connect();
    }
  }
}

void HOMIE_Device::setHeartbeatInterval(uint32_t hb_time_ms) {
  _heartbeat_interval_ms = hb_time_ms;
}


void HOMIE_Device::begin(const char* name, const char* base) {
  strncpy(_name, name, sizeof(_name));
  strncpy(_fullbase, base, sizeof(_fullbase));
  strncat(_fullbase, "/", sizeof(_fullbase) - strlen(_fullbase));
  strncat(_fullbase, name, sizeof(_fullbase) - strlen(_fullbase));
  strncpy(_will_topic, _fullbase, sizeof(_will_topic)); // we need permanent allocated char*
  strncat(_will_topic, "/$state", sizeof(_will_topic) - strlen(_will_topic));
  _mqttClient = new AsyncMqttClient();
  _mqttClient->setServer(_mqtt_server, _mqtt_port);
  _mqttClient->setWill(_will_topic, 1, true, _last_will);
  _mqttClient->onConnect(s_onConnect);
  _mqttClient->onDisconnect(s_onDisconnect);
  _mqttClient->onMessage(s_onMessage);
  _mqttReconnectTimer = xTimerCreate("mqttReconnTimer", pdMS_TO_TICKS(MQTT_RECONNECT), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(s_connect));
  _mqttHeartbeatTimer = xTimerCreate("mqttHBTimer", pdMS_TO_TICKS(_heartbeat_interval_ms), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(s_Heartbeat));
  if(WiFi.isConnected()) {
    _mqttClient->connect();
  }
  xTimerStart(_mqttReconnectTimer, 0);
}

void HOMIE_Device::s_connect() {
  getThis()->connect();
}

void HOMIE_Device::connect() {
  if(_mqttClient) {
    if(_mqttClient->connected()) {
      return;
    }
    if(WiFi.isConnected()) {
      _reconnect_count++;
      _mqttClient->connect();
    }
  }
}

bool HOMIE_Device::connected() {
  return _mqttClient->connected();
}

int HOMIE_Device::reconnectCount() {
  return _reconnect_count;
}

void HOMIE_Device::publishConfig() {
  sub_publish("/$state", "init", true);
  sub_publish("/$homie", "3.0.1", true);
  sub_publish("/$name", _name, true);
  sub_publish("/$localip", WiFi.localIP().toString().c_str(), true);
  sub_publish("/$mac", WiFi.macAddress().c_str(), true);
  sub_publish("/$fw/name", _firmware, true);
  sub_publish("/$fw/version", _version, true);
  sub_publish("/$implementation", "arduino esp32", true);
  sub_publish("/$stats", "uptime", true);
  sub_publish("/$stats/uptime", millis()/1000, true);
  sub_publish("/$stats/interval", _heartbeat_interval_ms/1000, true);
  int l = 0;
  for(auto it : _nodes) {
    l += strlen(it->getName()) + 1;
  }
  char nodes[l];
  bool first = true;
  for(auto it : _nodes) {
    if(!first) {
      strncpy(nodes, ",", sizeof(nodes));
    }
    strncpy(nodes, it->getName(), sizeof(nodes));
    first  = false;
  }
  sub_publish("/$nodes", nodes, true);
  for(auto it : _nodes) {
    it->publishConfig(_fullbase, _mqttClient);
  }
  sub_publish("/$state", "ready", true);
}

void HOMIE_Device::sub_publish(const char* subtopic, const char* msg, bool retain) {
  if (!_mqttClient->connected()) {
    return;
  }
  char buffer[strlen(_fullbase) + strlen(subtopic) + 2];
  strncpy(buffer, _fullbase, sizeof(buffer));
  strncat(buffer, subtopic, sizeof(buffer) - strlen(buffer));
  _mqttClient->publish(buffer, 1, retain, msg);
}

void HOMIE_Device::sub_publish(const char* subtopic, int data, bool retain) {
  char str[9];
  sub_publish( subtopic, itoa(data, str, 10), retain);
}

void HOMIE_Device::sendUpdates(HOMIE_Property* ident) {
  if(!_mqttClient->connected()) {
    return;
  }
  for(auto it : _nodes) {
    it->publishValue(_fullbase, _mqttClient, ident);
  }
}
