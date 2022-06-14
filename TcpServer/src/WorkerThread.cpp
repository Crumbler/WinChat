#include "WorkerThread.hpp"

#include <cstdio>
#include <winsock2.h>
#include "ClientKey.hpp"
#include "MessageType.hpp"

extern HANDLE cmpPort;

unsigned __stdcall WorkerThread(void *param)
{
    printf("Thread started\n");

    DWORD trBytes;
    ClientKey *pKey;
    IOOverlapped *pOv;

    while (true)
    {
        WINBOOL iResult =
            GetQueuedCompletionStatus(cmpPort, &trBytes, (PULONG_PTR)&pKey,
                                      (OVERLAPPED **)&pOv, INFINITE);

        if (iResult == 0)
        {
            fprintf(stderr, "Receive error: %lu\n", GetLastError());
            continue;
        }
        else if (iResult != 0 && trBytes == 0)
        {
            printf("Client disconnected\n");
            continue;
        }

        IOType ioType = pOv->type;

        if (ioType == IOType::Send)
        {
            continue;
        }

        // Incomplete receival
        pKey->bytesReceived += trBytes;
        const int bytesLeftToReceive = pKey->bytesExpected - pKey->bytesReceived;
        if (bytesLeftToReceive > 0)
        {
            printf("Received incomplete message with %d missing bytes\n", bytesLeftToReceive);
            pKey->AdvanceInputBuffer(trBytes);
            pKey->ReceiveAsync(bytesLeftToReceive);
            continue;
        }

        int msgType = pKey->inBufBase[0];
        int msgLength = pKey->inBufBase[1];

        // If message type and length have been received
        if (pKey->bytesReceived == 2)
        {
            printf("Receiving message of type %d and length %d\n", msgType, msgLength);

            // Receive the message itself
            pKey->AdvanceInputBuffer(2);
            pKey->ReceiveAsync(msgLength);
            continue;
        }

        printf("Received a %d character message:\n%s\n", msgLength, pKey->inBufBase + 2);

        switch (msgType)
        {
        case MessageType::NameOffer:
            pKey->outBufBase[0] = MessageType::NameAccepted;

            pKey->SendAsync(1);

            break;

        case MessageType::ClientMessage:

            break;
        }

        pKey->ResetInput();
        pKey->bytesExpected = 2;
        pKey->ReceiveAsync(2);
    }
}
