/* LABSO2022-1 */

#define DEBUG          1

#define CODANAMEMAXLEN 32
#define BUFMAXLEN      99
typedef enum { false = 0, true = 1 } bool;
typedef enum { none = 0, new = 1, put = 2, get = 3, del = 4, emp = 5, mov = 6 } action;

#include <stdlib.h>    // exit
#include <stdio.h>     // printf, fprintf
#include <string.h>    // strcpy
#include <sys/msg.h>   // queue
#include <sys/types.h> // fqueue
#include <fcntl.h>     // O_...
#include <errno.h>     // errno
#include <signal.h>    // signal

// ---------------------------------------------------------------------------------------------------- //

char argcodaname[CODANAMEMAXLEN]; // store name of "main" queue
int fd, qkey, qid, qid1, qid2;    // file descriptor, queue's key, current queue, source and target queue
struct msg_buffer {
    long mtype;
    char mtext[BUFMAXLEN];
} message;
int sig;                          // pid of external process to send signal to

// ---------------------------------------------------------------------------------------------------- //

// dump message with int argument to stderr if DEBUG set
void dump(char* msg, int val) {
    if (DEBUG) {
        fprintf(stderr, "%s (%d) - #%d: %s\n", msg, val, errno, strerror(errno)); errno = 0;
    }
}


// signal external process in case of success/error:
void sig1() { if (DEBUG) { fprintf(stderr, "sig1->%d\n", sig); }; kill(sig, SIGUSR1); }                              // send SIGUSR1
void sig2() { if (DEBUG) { fprintf(stderr, "sig2->%d\n", sig); }; kill(sig, SIGUSR2); }                              // send SIGUSR2
void trigger(int st) { if (sig > 0) { if (st < 0) { sig2(); } else { sig1(); }; }; }; // call sig1 or sig2 based on argument

// quit program with custom message and passed code
int quit(int code) { // exit function
    char msg[36] = "?unknown";
    if (code == 2) strcpy(msg, "syntax error");
    if (code == 3) strcpy(msg, "action unknown");
    if (code == 4) strcpy(msg, "missing value");
    if (code == 5) strcpy(msg, "unwanted value found");
    if (code == 6) strcpy(msg, "cannot create file");
    if (code == 7) strcpy(msg, "missing source queue");
    if (code > 0) fprintf(stderr, "?Error #%d: %s.\n", code, msg);
    if (code > 0) trigger(-1);
    exit(code);
}

// create queue with stated "name"
int createQueue(char* name, int force) {
    int res, qnew, tr = 0;
    res = open(name, O_CREAT, 0644); // try to create file
    if (res == -1) quit(6);
    dump("[createQueue] res", res);           // (ignore possible error)
    qkey = ftok(name, 1);                     // compute token
    if (qkey < 0) tr = -1;
    dump("[createQueue] qkey", qkey);
    if (force == 1) {
        qnew = msgget(qkey, 0777 | IPC_CREAT);    // compute queue's id, force creation
    }
    else {
        qnew = msgget(qkey, 0777);                // compute queue's id, retrieve existent (if any)
    }
    if (qnew < 0) if (force > 0) { tr = -1; }
    else { quit(7); };
    dump("[createQueue] qid", qnew);
    trigger(tr);
    return qnew;
}

int doWriteQueue(char* msg) {
    // write argument to current queue
    int res;
    strcpy(message.mtext, msg);
    dump("[doWriteQueue] len", strlen(message.mtext));
    dump("[doWriteQueue] qid", qid);
    res = msgsnd(qid, &message, sizeof(message.mtext), 0);
    trigger(res);
    dump("[doWriteQueue] res", res);
    return res;
}
int doReadQueue(int flag) {
    // non-blocking read from current queue
    int res;
    res = msgrcv(qid, &message, BUFMAXLEN, 1, MSG_NOERROR | IPC_NOWAIT);
    if (flag)trigger(res);
    dump("[doReadQueue] res", res);
    if (res > 0) printf("%s\n", message.mtext);
    return res;
}
int doEmptyQueue() {
    // loop-reading until queue is empty
    int res;
    while ((res = doReadQueue(0)) > 0);
    return res;
}
int doDeleteQueue() {
    // remove queue
    int res;
    res = msgctl(qid, IPC_RMID, NULL);
    trigger(res);
    dump("[doDeleteQueue] res", res);
    return res;
}
int doMoveQueue(char* value) {
    // create target queue and loop into source one
    int res, num = 0;
    char buf[BUFMAXLEN];
    qid2 = createQueue(value, 1);
    if (qid2 < 1) quit(6);
    while ((res = doReadQueue(0)) > 0) {// keep on reading messages from source
        qid = qid2;                 // set current queue to target
        strcpy(buf, message.mtext); // copy message read in temporary buffer
        doWriteQueue(buf);          // write buffer into destination
        qid = qid1;                 // restore current queue to source
        num++;                      // count moved messages
    };
    doDeleteQueue();                // remove source queue
    printf("%d\n", num);
    return num;
}

// ---------------------------------------------------------------------------------------------------- //

int perform(int chosen, char* value, int pid) {
    sig = pid;
    int fd, force;
    message.mtype = 1;
    force = (chosen != mov) ? 1 : 0;
    qid1 = createQueue(argcodaname, force); // always create queue but for "mov" action
    qid = qid1;                             // default queue is main one (source)
    if (chosen == new) printf("%d\n", qid);   // (queue already created) simply dump required info
    if (chosen == put) doWriteQueue(value);   // write value into current (source) queue
    if (chosen == get) doReadQueue(1);         // read from current (source) queue
    if (chosen == del) doDeleteQueue();       // delete current (source) queue
    if (chosen == emp) doEmptyQueue();        // read out all messages from current (source) queue to empty it
    if (chosen == mov) doMoveQueue(value);    // move all messages from source to target
    return fd;
}

// ---------------------------------------------------------------------------------------------------- //

int main(int argc, char** argv) {

    int argn = argc - 1, pid;
    action argaction = none;

    if (argn < 3 || argn>4) quit(2); // check syntax

    strcpy(argcodaname, argv[1]);  // store name of queue
    // manage action:
    if (strcmp(argv[2], "new") == 0) argaction = new;
    if (strcmp(argv[2], "put") == 0) argaction = put;
    if (strcmp(argv[2], "get") == 0) argaction = get;
    if (strcmp(argv[2], "del") == 0) argaction = del;
    if (strcmp(argv[2], "emp") == 0) argaction = emp;
    if (strcmp(argv[2], "mov") == 0) argaction = mov;
    if (argaction == none) quit(3);  // check if action unknown
    if (argn == 3 && (argaction == mov || argaction == put)) quit(4); // check specific case I
    if (argn == 4 && (argaction != mov && argaction != put)) quit(5); // check specific case II

    pid = atoi((argaction == put || argaction == mov) ? argv[4] : argv[3]);
    perform(argaction, argv[3], pid);

    quit(0);

};
