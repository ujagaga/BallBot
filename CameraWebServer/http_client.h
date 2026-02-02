#pragma once

extern void HTTPC_fwUpdate(); 
extern bool HTTPC_fwUpdateInProgress(void);
extern void HTTPC_fwUpdateRequest(char* fwDownloadUrl, char* logUrl);
extern void HTTPC_process(void);
