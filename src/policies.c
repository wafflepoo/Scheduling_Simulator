#include "scheduler.h"

SchedPolicy FIFO_POLICY  = { "FIFO",  fifo_select,  0 };
SchedPolicy SJF_POLICY   = { "SJF",   sjf_select,   0 };
SchedPolicy SRJF_POLICY  = { "SRJF",  sjrf_select,  0 };
SchedPolicy RR_POLICY    = { "Round Robin", rr_select, 2 };  // quantum = 2