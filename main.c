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

int request_cs(Proc* proc) { // ptr to arr of processes
  Msg msg;

  msg.type = cs;
  msg.proc = proc;
  msg.time = time(0);

  send_all(&msg, proc); // await ok's?
  proc->state = awaiting;
  
  return 0;
}

int enter_cs(Proc* proc) {
  proc->oks = 0; // reset oks counter
  proc->state = in_cs;
  return 0;
}

int exit_cs(Proc* proc) {
  Msg msg;

  msg.type = ok;
  msg.proc = proc;
  msg.time = time(0);

  proc->state = normal;

  for (int i = 0; i < proc->queue_len; i++) {
    Msg* q_msg = proc->queue[i]; // msg in queue
    send(q_msg->proc, &msg);
    dequeue(proc->queue); // dequeue topmost
  }
}

int send(Proc* proc, Msg* msg) { // dest, msg
  handle(proc, msg);
  return 0;
}

int send_all(Msg* msg, Proc* proc) { // proc is sender
  for (int i = 0; i < procs_len; i++) {
    Proc* curr = procs[i];

    if (curr->pid != proc->pid) { // send to all except sender
      send(curr, msg);
    }
  }

  proc->last_msg = msg; // save last request_cs msg

  return 0;
}

int handle(Proc* proc, Msg* msg) {
  Msg ok_msg;

  ok_msg.type = ok;
  ok_msg.proc = proc;
  ok_msg.time = time(0);

  if (msg->type == cs) {
    switch(proc->state) {
      case normal:
        send(msg->proc, &ok_msg); // reply with ok
        break;
      case in_cs:
        queue(msg);
        break;
      case awaiting:
        if (msg->time < proc->last_msg->time) {
          send(msg->proc, &ok_msg);
        } else {
          queue(msg);
        }
    }
  } else { // ok
    if (proc->state == awaiting) proc->oks += 1;
    if (proc->oks == procs_len) {
      // all procs responded
      enter_cs(proc);
    }
  }
}

int time(int add) { // *
  static int current_time = 0;
  current_time += add;

  return current_time;
}

// queue, dynamic_arr, dequeue





