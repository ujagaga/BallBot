#ifndef IR_CTRL_H
#define IR_CTRL_H

#define CMD_IR_OFF    0
#define CMD_IR_ON     1
#define CMD_TV_ON_OFF 2
#define CMD_AC_ON     3
#define CMD_AC_OFF    4

void IRCTRL_send(int cmd);

#endif