#ifndef NET_WORK_H
#define NET_WORK_H

#include "global.h"
#include "data.h"

#define NUM_TOTAL_IP 2

extern unsigned char ID[50];
extern void ClearData(int n);
extern int checkFdStatus(int *fdArray, int n, unsigned char isRead, int nt);
extern float getDataAverMinMaxSum(int n, int date, int class);
extern float getSenserValue(int n);
extern char *GetParaName(int i);
extern char *GetParaLabel(int i);
extern int ipToString(char *p, uint8_t n);
extern uint16_t getDestPort(uint8_t n);
#endif