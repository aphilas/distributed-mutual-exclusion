#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
  awaiting,
  in_cs,
  normal,
} ProcState;

typedef struct Proc {
  int pid;
  struct Msg* last_msg;
  struct Msg** queue; // array of msg ptrs
  int queue_len;
  ProcState state;
  int oks;
} Proc;

typedef enum {
  cs,
  ok
} MsgType;

typedef struct Msg {
  MsgType type;
  struct Proc* proc;
  int time; // logical time
} Msg;

int request_cs(Proc* proc);
int enter_cs(Proc* proc); // in cs
int exit_cs(Proc* proc);

int send(Proc* proc, Msg* msg); // send msg to proc
int handle(Proc* proc, Msg* msg); // proc - target
int send_all(Msg* msg, Proc* proc); // msg, sender

int time(int add);

// globals

Proc** procs;
int procs_len;

int main(void) {
  return 0;
}

