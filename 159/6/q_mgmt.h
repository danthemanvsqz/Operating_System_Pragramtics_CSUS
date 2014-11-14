// q_mgmt.h, 159 1

#ifndef _Q_MGMT_H_
#define _Q_MGMT_H_

#include "types.h"

int EmptyQ(q_t *);
int FullQ(q_t *);
void InitQ(q_t *);
void EnQ(int, q_t *);
int DeQ(q_t *);

int MsgEmptyQ(msg_q_t *);
int MsgFullQ(msg_q_t *);
void MsgEnQ(msg_t *, msg_q_t *);
msg_t *MsgDeQ(msg_q_t *);

void MyBZero(char *, int);
void MyMemCpy(char *, char *, int);
void MyStrCpy(char *, char *);

int StrCmp(char *, char *);

char CharDeQ(char_q_t *);
void CharEnQ(char, char_q_t *);
int CharEmptyQ(char_q_t *);
int CharFullQ(char_q_t *);
void CharInitQ(char_q_t *);

#endif
