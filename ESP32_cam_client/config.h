#ifndef CONFIG_H
#define CONFIG_H

#define TCP_SERVER_URL          "192.168.0.200"
#define TCP_SERVER_PORT         9010

// Capture and send an image every few ms
#define STREAM_RATE_MS          200

extern void MAIN_debug(String message);

#endif
