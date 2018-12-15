#ifndef Workers_h
#define Workers_h

#include <stdio.h>
#include "Commons.h"

void do_reader(int pip[2]);
void do_writer(int pip[2]);

#endif /* Workers_h */
