#ifndef TYPES_H
#define TYPES_H

#define CMD_CREATE 1
#define CMD_READ 2
#define CMD_UPDATE 3
#define CMD_DELETE 4
#define CMD_DISCONNECT 5

typedef struct __payload_t {
    int cmd_type;
    int key;
    int value_size;
} payload_t;

#endif
