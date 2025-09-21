#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include "wifi_connection.h"

WebSocketsServer* wsServer = nullptr;

void WS_process(){
  if(wsServer){
    wsServer->loop();  
  }   
}

void WS_ServerBroadcast(String msg){
  if(wsServer){
    wsServer->broadcastTXT(msg);  
  } 
}

static void serverEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{ 
  if(type == WStype_TEXT){
    //Serial.printf("[%u] get Text: %s\r\n", num, payload);
    char textMsg[length + 1];     // +1 for null terminator
    for(int i = 0; i < length; i++){
      textMsg[i] = payload[i];          
    }
        
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, textMsg);    

    // Test if parsing succeeds.
    if (!error) {
      JsonObject root = doc.as<JsonObject>();
      
      if(root.containsKey("APLIST")){  
        String APList = "{\"APLIST\":\"" + WIFIC_getApList() + "\"}";
        wsServer->sendTXT(num, APList);   
      }
    }      
  }   
}

void WS_dispose() {
  if (wsServer) {
    wsServer->close();    // Disconnect clients and stop listening
    delete wsServer;      // Free heap memory
    wsServer = nullptr;
  }
}

void WS_init() {
    WS_dispose();  // Safety: clean up any old instance
    wsServer = new WebSocketsServer(81);
    wsServer->begin();
    wsServer->onEvent(serverEvent);
}
