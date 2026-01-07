#pragma once

#define TCP_SERVER_URL          "10.42.0.1"
#define TCP_SERVER_PORT         9010

// Capture and send an image every few ms
#define STREAM_RATE_MS          200
#define RESPONSE_IMG            0
#define RESPONSE_TXT            1
#define RESPONSE_DBG            2


extern void TCPC_Process(void);
extern bool TCPC_Debug(String message);
