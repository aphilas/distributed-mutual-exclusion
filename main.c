#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define QUEUE_BASE_SIZE 10
#define NO_OF_PROCS 10

typedef enum ProcState {
  normal,
  awaiting,
  in_cs,
} ProcState;

typedef enum MsgType {
  cs,
  ok
} MsgType;

typedef struct Proc {
  int pid;
  struct Msg* last_msg;
  struct MsgQueue* queue; // array of msg ptrs
  ProcState state;
  int oks;
} Proc;

typedef struct Msg {
  MsgType type;
  struct Proc* proc;
  int time; // logical time
} Msg;

typedef struct MsgQueue {
  struct Msg** list; // list[Msg*]
  int size;
  int used;
} MsgQueue;

int request_cs(Proc*);
int enter_cs(Proc*); // in cs
int exit_cs(Proc*);

int send(Proc*, Msg*); // send msg to proc
int handle(Proc*, Msg*); // proc - target
int send_all(Msg*, Proc*); // msg, sender

int create_queue(MsgQueue*);
int queue(MsgQueue*, Msg*);
Msg* dequeue(MsgQueue*);

int get_time(int);
bool toss(int);
void tick(void); // run the world!
void prt_if(ProcState); // print procs by state

// global
Proc procs[NO_OF_PROCS];

int main(void) {
  Msg m1;
  srand(time(NULL));

  for (int i = 0; i < NO_OF_PROCS; i++) {
    Proc p;
    MsgQueue* q = (MsgQueue*) malloc(sizeof(MsgQueue));

    p.pid = i;
    p.oks = 0;
    p.last_msg = NULL;
    p.queue = q;
    p.state = normal;

    create_queue(p.queue);

    procs[i] = p;
  }

  for (int i = 0; i < 20; i++) {
    tick();
  }

  return 0;
}

int request_cs(Proc* proc) { // ptr to arr of processes
  Msg msg;

  msg.type = cs;
  msg.proc = proc;
  msg.time = get_time(0);

  proc->state = awaiting;
  send_all(&msg, proc); // await ok's
  
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
  msg.time = get_time(0);

  proc->state = normal;

  // printf("top of queue: p%d\n", proc->queue->list[0]->proc->pid);
  proc->queue->list[0]->proc->pid;

  for (int i = 0; i < proc->queue->used; i++) {
    send(dequeue(proc->queue)->proc, &msg);
    // printf("Sent p%d ok\n", q_msg->proc->pid);
  }
}

int send(Proc* proc, Msg* msg) { // dest, msg
  handle(proc, msg);
  return 0;
}

int send_all(Msg* msg, Proc* proc) { // proc is sender
  for (int i = 0; i < NO_OF_PROCS; i++) {
    Proc* curr = &procs[i];

    if (curr->pid != proc->pid) { // send to all except sender
      send(curr, msg);
    }
  }

  proc->last_msg = msg; // save last request_cs msg

  return 0;
}

int handle(Proc* proc, Msg* msg) {
  Msg ok_msg;

  // printf("%d received msg of type %d from %d\n", proc->pid, msg->type, msg->proc->pid);

  ok_msg.type = ok;
  ok_msg.proc = proc;
  ok_msg.time = get_time(0);

  if (msg->type == cs) {
    switch(proc->state) {
      case normal:
        send(msg->proc, &ok_msg); // reply with ok
        break;
      case in_cs:
        printf("p%d attempted to queue p%d\n", proc->pid, msg->proc->pid);
        queue(proc->queue, msg);
        break;
      case awaiting:
        if (msg->time < proc->last_msg->time) {
          send(msg->proc, &ok_msg);
        } else {
          printf("p%d attempted to queue p%d\n", proc->pid, msg->proc->pid);
          queue(proc->queue, msg);
        }
    }
  } else { // ok
    // printf("p%d recieved ok from %d\n", proc->pid, msg->proc->pid);

    if (proc->state == awaiting) proc->oks += 1;
    // printf("p%d, state: %d, oks: %d\n", proc->pid, proc->state, proc->oks);

    // all procs responded
    if (proc->oks == NO_OF_PROCS - 1) enter_cs(proc);
  }
}

int get_time(int add) { // *
  static int current_time = 0;
  current_time += add;

  return current_time;
}

int create_queue(MsgQueue* msg_queue) {
  // list[Msg*]
  msg_queue->list = (Msg**) malloc(sizeof(Msg*) * QUEUE_BASE_SIZE); // arr of messages
  msg_queue->size = QUEUE_BASE_SIZE;
  msg_queue->used = 0;
  return 0;
}

int queue(MsgQueue* queue, Msg* msg) {
  if (queue->used >= queue->size) { // queue full
    queue->list = (Msg**) realloc(queue->list, sizeof(Msg*) * (queue->size * 2)); // double size of list
    queue->size *= 2;
  } 

  queue->list[queue->used] = msg;
  queue->used += 1;

  // printf("Inserted p%d. Used: %d, size: %d.\n", queue->list[queue->used - 1]->proc->pid, queue->used, queue->size);

  return 0;
}

Msg* dequeue(MsgQueue* queue) {
  Msg* top;
  if (queue->used > 0) {
    top = queue->list[0];
    queue->list[0] = NULL;
    queue->used -= 1;

    // printf("dequeued p%d", top->proc)

    return top;
  }

  return NULL;
}

bool toss(int k) {
  return ((double) rand()/(double) RAND_MAX < (double) 1/k) ? true : false;
}

void prt_if(ProcState state) {
  for (int i = 0; i < NO_OF_PROCS; i++) {
    if (procs[i].state == state) printf("p%d ", procs[i].pid); 
  }
  printf("\n");
}

void tick(void) {
  for (int i = 0; i < NO_OF_PROCS; i++) {
    Proc* curr = &procs[i];
    
    switch (curr->state){
    case normal:
      if (toss(10)) request_cs(curr);
      break;

    case awaiting:
      // wait for oks
      break;

    case in_cs:
      if (toss(2)) exit_cs(curr);
      break;
    
    default:
      break;
    }
  }

  printf("time %d:\n", get_time(0));
  printf(" normal: "); prt_if(normal);
  printf(" awaiting: "); prt_if(awaiting);
  printf(" cs: "); prt_if(in_cs);

  get_time(1); // the times / they are a-changing
}