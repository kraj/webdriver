#ifndef __WPE_DRIVER_COMMON_H__
#define __WPE_DRIVER_COMMON_H__

#define WD_MSG_SIZE 128
#define WPE_WD_CMD_KEY  1234
#define WPE_WD_STATUS_KEY  4321

enum WDStatus {
   WD_NONE,
   WD_SUCCESS,
   WD_FAILURE
};

enum WDCommand {
   WD_CMD_NONE,
   WD_CREATE_VIEW,
   WD_REMOVE_VIEW,
   WD_RELOAD,
   WD_JS_CMD_START,
   WD_GET_URL,
   WD_JS_CMD_END
};

struct WDStatusBuf {
    WDStatus status;
    char rspMsg[WD_MSG_SIZE];
};

struct WDCommandBuf {
    WDCommand command;
    char message[WD_MSG_SIZE]; 
};


const int WD_CMD_SIZE = sizeof(WDCommandBuf);
const int WD_STATUS_SIZE = sizeof(WDStatusBuf);

#endif //__WPE_DRIVER_COMMON_H__