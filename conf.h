//.h con configuraciones pa pillar de dir de data, el dir
//
#ifndef __smtp_CONF_H
#define __smtp_CONF_H

#define PORT 1025  // para el 25 se necesitan privilegios
#define MAX_CLIENTS 32 //para el listen
#define MAX_RCPT_USR 50
#define BUF_SIZE 1024

const char dData[] = "data/";
const char dUInfo[] = "dUInfo";
const char dUStat[] = "dUStat";

extern int mail_stat;
extern int rcpt_user_num;
extern char from_user[64];
extern char rcpt_user[MAX_RCPT_USR][30];

#endif
