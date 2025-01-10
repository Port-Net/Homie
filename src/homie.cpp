
#include <Arduino.h>
#include "homie.h"
//#include "raffstore.h"
//#include <ArduinoJson.h>
//#include <YAMLDuino.h>

//AsyncMqttClient mqttClient;
//TimerHandle_t mqttReconnectTimer;
//TimerHandle_t mqttHeartbeatTimer;

HOMIE_Property::HOMIE_Property(String name) : _name(name) {
  _description = name;
  _name.toLowerCase();
  _settable = false;
  _retained = false;
  _datatype = String("integer");
  _format = String("");
  _unit = "";
  _ID = _name;
  _only_direct = false;
}

HOMIE_Property& HOMIE_Property::description(String description) {
  _description = description;
  return *this;
}

HOMIE_Property& HOMIE_Property::settable(bool settable) {
  _settable = settable;
  return *this;
}

HOMIE_Property& HOMIE_Property::retained(bool retained) {
  _retained = retained;
  _retained_set = true;
  return *this;
}

HOMIE_Property& HOMIE_Property::onlyDirect(bool direct) {
  _only_direct = direct;
  return *this;
}

HOMIE_Property& HOMIE_Property::datatype(String datatype) {
  _datatype = datatype;
  return *this;
}

HOMIE_Property& HOMIE_Property::format(String format) {
  _format = format;
  return *this;
}

HOMIE_Property& HOMIE_Property::unit(String unit) {
  _unit = unit;
  return *this;
}

HOMIE_Property& HOMIE_Property::ID(String id) {
  _ID = id;
  return *this;
}

HOMIE_Property& HOMIE_Property::setSetCallback(void (*callback)(HOMIE_Property* prop, String msg)) {
  _set_callback = callback;
  return *this;
}

HOMIE_Property& HOMIE_Property::setGetCallback(String (*callback)(HOMIE_Property* prop)) {
  _get_callback = callback;
  return *this;
}

String HOMIE_Property::getName() {
  return _name;
}

String HOMIE_Property::getDatatype() {
  return _datatype;
}

String HOMIE_Property::getFormat() {
  return _format;
}

String HOMIE_Property::getUnit() {
  return _unit;
}

String HOMIE_Property::getID() {
  return(_ID);
}

bool HOMIE_Property::settable() {
  return _settable;
}

bool HOMIE_Property::retained() {
  return _retained;
}

void HOMIE_Property::processSet(String topic, String msg) {
  if(topic != _name + String("/set")) {
    return;
  }
  if(_settable && _set_callback) {
    _set_callback(this, msg);
  }
}

void HOMIE_Property::publishValue(String topic, AsyncMqttClient* mqttClient, bool direct) {
  if(_only_direct && !direct) {
    return;
  }
  String my_topic = topic + String("/") + _name;
  if(_get_callback) {
    String msg = _get_callback(this);
    mqttClient->publish(my_topic.c_str(), 1, _retained, msg.c_str());
  }
}

void HOMIE_Property::unpublishConfig(String topic, AsyncMqttClient* mqttClient) {
  String my_topic = topic + String("/") + _name;
  //mqttClient->publish(new_topic.c_str(), 1, false, msg.c_str());
  mqttClient->publish( (my_topic + String("/$name")).c_str(), 1, true, "");
  mqttClient->publish( (my_topic + String("/$settable")).c_str(), 1, true, "");
  mqttClient->publish( (my_topic + String("/$datatype")).c_str(), 1, true, "");
  if(_format != "") {
    mqttClient->publish( (my_topic + String("/$format")).c_str(), 1, true, "");
  }
  if(_unit != "") {
    mqttClient->publish( (my_topic + String("/$unit")).c_str(), 1, true, "");
  }
}

void HOMIE_Property::publishConfig(String topic, AsyncMqttClient* mqttClient) {
  String my_topic = topic + String("/") + _name;
  //mqttClient->publish(new_topic.c_str(), 1, false, msg.c_str());
  mqttClient->publish( (my_topic + String("/$name")).c_str(), 1, true, _description.c_str());
  mqttClient->publish( (my_topic + String("/$settable")).c_str(), 1, true, _settable ? "true" : "false");
  if(_retained_set) {
    mqttClient->publish( (my_topic + String("/$retained")).c_str(), 1, true, _retained ? "true" : "false");
  }
  mqttClient->publish( (my_topic + String("/$datatype")).c_str(), 1, true, _datatype.c_str());
  if(_format != "") {
    mqttClient->publish( (my_topic + String("/$format")).c_str(), 1, true, _format.c_str());
  }
  if(_unit != "") {
    mqttClient->publish( (my_topic + String("/$unit")).c_str(), 1, true, _unit.c_str());
  }
  if(_settable) {
    mqttClient->subscribe((my_topic + String("/set")).c_str(), 1);
  }

}

//////////////////////////////////////

HOMIE_Node::HOMIE_Node(String name) : _name(name) {
  _description = name;
  _name.toLowerCase();
  _type = "";
}

HOMIE_Node& HOMIE_Node::description(String name) {
  _description = name;
  return *this;
}

HOMIE_Node& HOMIE_Node::type(String type) {
  _type = type;
  return *this;
}

void HOMIE_Node::addProperty(HOMIE_Property* prop) {
  _properties.push_back(prop);
}

void HOMIE_Node::removeProperties() {
  _properties.clear();
}

String HOMIE_Node::getName() {
  return(_name);
}

void HOMIE_Node::processSet(String topic, String msg) {
  int idx = topic.indexOf("/");
  if(idx <= 0) {
    return;
  }
  String node = topic.substring(0, idx);
  if(node == _name) {
    String new_topic = topic.substring(idx + 1);
    for(auto it : _properties) {
      it->processSet(new_topic, msg);
    }
  }
}

void HOMIE_Node::publishValue(String topic, AsyncMqttClient* mqttClient, HOMIE_Property* ident) {
  String new_topic = topic + String("/") + _name;
  for(auto it : _properties) {
    if(!ident) {
      it->publishValue(new_topic, mqttClient);
    } else if(ident == it) {
      it->publishValue(new_topic, mqttClient, true);
    }
  }
}

void HOMIE_Node::unpublishConfig(String topic, AsyncMqttClient* mqttClient) {
  String new_topic = topic + String("/") + _name;
  mqttClient->publish( (new_topic + String("/$name")).c_str(), 1, true, "");
  mqttClient->publish( (new_topic + String("/$type")).c_str(), 1, true, "");
  mqttClient->publish( (new_topic + String("/$properties")).c_str(), 1, true, "");
  for(auto it : _properties) {
    it->unpublishConfig(new_topic, mqttClient);
  }
}

void HOMIE_Node::publishConfig(String topic, AsyncMqttClient* mqttClient) {
  String new_topic = topic + String("/") + _name;
  mqttClient->publish( (new_topic + String("/$name")).c_str(), 1, true, _description.c_str());
  mqttClient->publish( (new_topic + String("/$type")).c_str(), 1, true, _type.c_str());
  String props = String("");
  for(auto it : _properties) {
    if(!props.isEmpty()) {
      props += String(",");
    }
    props += it->getName();
  }
  mqttClient->publish( (new_topic + String("/$properties")).c_str(), 1, true, props.c_str());
  for(auto it : _properties) {
    it->publishConfig(new_topic, mqttClient);
  }
}

////////////////////////////////////////

HOMIE_Device::HOMIE_Device(String mqtt_server, uint16_t mqtt_port) : _mqtt_port(mqtt_port) {
  getThis(this);
  strncpy(_mqtt_server, mqtt_server.c_str(), sizeof(_mqtt_server)); // we need permanent allocated char*
  _mqttClient = nullptr;
  _heartbeat_interval_ms = MQTT_HEARTBEAT;
  _reconnect_count = 0;
}

HOMIE_Device& HOMIE_Device::addNode(HOMIE_Node* node) {
  _nodes.push_back(node);
  return *this;
}

bool HOMIE_Device::removeNode(String name) {
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

  /*
  std::erase_if(_nodes, [&name] (const HOMIE_Node& n) { return n.getName() == name; });
  */
  std::erase(_nodes, p);
  publishConfig();
  return true;
}

HOMIE_Device& HOMIE_Device::setFirmware(String fw) {
  _firmware = fw;
  return *this;
}

HOMIE_Device& HOMIE_Device::setVersion(String ver) {
  _version = ver;
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
  String topicstring(topic);
  char pl[len+1];
  pl[len] = '\0';
  strncpy(pl, payload, len);
  String payloadstring(pl);
  if(!topicstring.startsWith(_fullbase + String("/"))) {
    return;
  }
  topicstring = topicstring.substring(_fullbase.length() + 1);
  for(auto it : _nodes) {
    it->processSet(topicstring, payloadstring);
  }
}

void HOMIE_Device::s_Heartbeat() {
  getThis()->Heartbeat();
}

void HOMIE_Device::Heartbeat() {
  if (_mqttClient->connected()) {
    publish( _fullbase + String("/$stats/uptime"), millis()/1000, true);
    // TODO handle additional stats
  } else {
    if (WiFi.isConnected()) {
      _reconnect_count++;
      if(_reconnect_count > 10) {
        delete _mqttClient;
        _mqttClient = new AsyncMqttClient();
        _mqttClient->setServer(_mqtt_server, _mqtt_port);
        _mqttClient->setWill(_will_topic, 1, true, "lost");
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


void HOMIE_Device::begin(String name, String base) {
  _name = name;
  _base = base;
  _fullbase = base + String("/") + name;
  strncpy(_will_topic, (_fullbase + String("/$state")).c_str(), sizeof(_will_topic)); // we need permanent allocated char*
  _mqttClient = new AsyncMqttClient();
  _mqttClient->setServer(_mqtt_server, _mqtt_port);
  _mqttClient->setWill(_will_topic, 1, true, "lost");
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
  publish(_fullbase + String("/$state"), "init", true);
  publish(_fullbase + String("/$homie"), "3.0.1", true);
  publish(_fullbase + String("/$name"), _name, true);
  publish(_fullbase + String("/$localip"), WiFi.localIP().toString(), true);
  publish(_fullbase + String("/$mac"), WiFi.macAddress(), true);
  publish(_fullbase + String("/$fw/name"), _firmware, true);
  publish(_fullbase + String("/$fw/version"), _version, true);
  publish(_fullbase + String("/$implementation"), "arduino esp32", true);
  publish(_fullbase + String("/$stats"), "uptime", true);
  publish(_fullbase + String("/$stats/uptime"), millis()/1000, true);
  publish(_fullbase + String("/$stats/interval"), _heartbeat_interval_ms/1000, true);
  String nodes = String("");
  for(auto it : _nodes) {
    if(!nodes.isEmpty()) {
      nodes += String(",");
    }
    nodes += it->getName();
  }
  publish(_fullbase + String("/$nodes"), nodes, true);
  for(auto it : _nodes) {
    it->publishConfig(_fullbase, _mqttClient);
  }
  publish(_fullbase + String("/$state"), "ready", true);
}

void HOMIE_Device::publish(String topic, const char* msg, bool retain) {
  if (!_mqttClient->connected()) {
    return;
  }
  _mqttClient->publish( topic.c_str(), 1, retain, msg);
}

void HOMIE_Device::publish(String topic, String msg, bool retain) {
  if (!_mqttClient->connected()) {
    return;
  }
  _mqttClient->publish( topic.c_str(), 1, retain, msg.c_str());
}

void HOMIE_Device::publish(String topic, int data, bool retain) {
  if (!_mqttClient->connected()) {
    return;
  }
  _mqttClient->publish( topic.c_str(), 1, retain, String(data).c_str());
}

void HOMIE_Device::sendUpdates(HOMIE_Property* ident) {
  if(!_mqttClient->connected()) {
    return;
  }
  for(auto it : _nodes) {
    it->publishValue(_fullbase, _mqttClient, ident);
  }
}
