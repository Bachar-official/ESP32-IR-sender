#pragma once
#include <Arduino.h>

class Conditioner {
  public:
  Conditioner(){};
  Conditioner(String condName, int state, String ip) {
    _name = condName;
    _state = state;
    _ip = ip;
    _date = "2000-01-01 00:00:00.000000";
}
  String response() {
    return "{\"name\":\"" + _name
  + "\", \"ipAddress\":\"" + _ip
  + "\", \"status\":\"" + _state
  + "\", \"date\":\"" + _date
  + "\"}";
};
  String operation(int state) {
    _state = state;
    return "{\"status\":\"" + String(_state, DEC) + "\"}";
};
	void setIp(String ip) {
		_ip = ip;
	}
  void setDate(String date) {
    _date = date;
  }
  private:
  String _name;
  int _state;
  String _ip;
  String _date;
};
