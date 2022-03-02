#ifndef BUILTIN_OP_CODES_H
#define BUILTIN_OP_CODES_H

//#include "common/types.h"

// generic, functional required OP Codes (0-49)
static const uint16_t OP_NONE = 0x00;
static const uint16_t OP_ConnectionRequest = 0x01;
static const uint16_t OP_ConnectionAccepted = 0x02;
static const uint16_t OP_ConnectionDenied = 0x03;
static const uint16_t OP_ConnectionFailed = 0x04;
static const uint16_t OP_ConnectionDisconnect = 0x05;
static const uint16_t OP_ConnectionTimedOut = 0x06;
static const uint16_t OP_ConnectionLost = 0x07;
static const uint16_t OP_NewConnectionData = 0x08;

static const uint16_t OP_Ack = 0x09;
static const uint16_t OP_KeepAlive = 0x0A;
static const uint16_t OP_RetransmissionRequest = 0x0B;
static const uint16_t OP_RetransmissionReply = 0x0C;
static const uint16_t OP_RetransmissionAck = 0x0D;
static const uint16_t OP_RetransmissionImpossible = 0x0E;

static const uint16_t OP_IPCData = 0x30;                     // 48 - Sharing data between IPC peers

static const uint16_t OP_ENDINTERNAL = 0x31;                 // 49 - End of Internal, Builtin OP Codes

#endif // BUILTIN_OP_CODES_H
