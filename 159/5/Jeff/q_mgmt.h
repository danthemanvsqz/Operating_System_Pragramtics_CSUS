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
void MsgDeQ(msg_t *p, msg_q_t *q);

void MyBZero(char *p, int size);
void MyStrCpy(char *dest, char *src);
void MyMemCpy(msg_t *dest, msg_t *src);
void MyMemCpy2(char* target, char* src, int size);

#endif
