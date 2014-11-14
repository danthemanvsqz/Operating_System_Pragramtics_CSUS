// q_mgmt.h, 159 1

#ifndef _Q_MGMT_H_
#define _Q_MGMT_H_

#include "types.h"

int EmptyQ(q_t *);
int FullQ(q_t *);
void InitQ(q_t *);
void EnQ(int, q_t *);
int DeQ(q_t *);
int MsgFullQ(msg_q_t *p);
int MsgEmptyQ(msg_q_t *p);
void MsgEnQ(msg_t *p, msg_q_t *q);
msg_t *MsgDeQ(msg_q_t *p);
void MyBZero(char *, int size);
void MyStrCpy(char *dest, char *src);
void MyMemCpy(char *dest, char *src, int size);

#endif
