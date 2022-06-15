#include "WorkerThread.hpp"

#include <cstdio>
#include <unordered_map>
#include <winsock2.h>
#include "ClientKey.hpp"
#include "MessageType.hpp"

struct strPtrHash
{
    unsigned operator()(std::string *s) const
    {
        return std::hash<std::string>()(*s);
    }
};

struct strPtrEqual
{
    unsigned operator()(std::string *s1, std::string *s2) const
    {
        return *s1 == *s2;
    }
};

extern HANDLE cmpPort;
extern CRITICAL_SECTION clientsSection;
std::unordered_map<std::string*, ClientKey*, strPtrHash, strPtrEqual> clients;

void RemoveClient(ClientKey *key);

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
            fprintf(stderr, "Client disconnected with error: %lu\n", GetLastError());
            RemoveClient(pKey);
            continue;
        }
        else if (iResult != 0 && trBytes == 0)
        {
            printf("Client disconnected\n");
            RemoveClient(pKey);
            continue;
        }

        IOType ioType = pOv->type;

        if (ioType == IOType::Send)
        {
            // If the name is taken, shut down the connection
            if (pKey->lastOutMsgType == MessageType::NameTaken)
            {
                pKey->socket->Shutdown(SD_BOTH);
            }

            continue;
        }

        // IOType::Receive

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
            {
                std::string *username = new std::string(pKey->inBufBase + 2);

                EnterCriticalSection(&clientsSection);

                auto res = clients.insert({ username, pKey });

                LeaveCriticalSection(&clientsSection);

                MessageType outMsgType;

                // If insertion successful
                if (res.second)
                {
                    outMsgType = MessageType::NameAccepted;
                    pKey->username = username;
                }
                else
                {
                    // A user with the same username already exists
                    outMsgType = MessageType::NameTaken;
                    delete username;
                }

                pKey->outBufBase[0] = outMsgType;
                pKey->lastOutMsgType = outMsgType;

                pKey->SendAsync(1);
            }

            break;

        case MessageType::ClientMessage:

            break;
        }

        pKey->ResetInput();
        pKey->bytesExpected = 2;
        pKey->ReceiveAsync(2);
    }
}

// Remove a client after disconnection
void RemoveClient(ClientKey *key)
{
    // If the user is in the unordered_map
    if (key->username != nullptr)
    {
        EnterCriticalSection(&clientsSection);

        clients.erase(key->username);

        LeaveCriticalSection(&clientsSection);
    }

    delete key;
}
