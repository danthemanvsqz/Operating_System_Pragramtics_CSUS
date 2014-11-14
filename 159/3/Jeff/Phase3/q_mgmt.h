// q_mgmt.h, 159 1

#ifndef _Q_MGMT_H_
#define _Q_MGMT_H_

#include "types.h"

int EmptyQ(q_t *);
int FullQ(q_t *);
void InitQ(q_t *);
void EnQ(int, q_t *);
int DeQ(q_t *);

#endif
