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
constexpr int sendBufSize = 256;
std::unordered_map<std::string*, ClientKey*, strPtrHash, strPtrEqual> clients;

void RemoveClient(ClientKey *key);

unsigned __stdcall WorkerThread(void *param)
{
    printf("Worker thread started\n");

    DWORD trBytes;
    ClientKey *pKey;
    IOOverlapped *pOv;

    char *sendBuf = new char[sendBufSize];

    while (true)
    {
        WINBOOL iResult =
            GetQueuedCompletionStatus(cmpPort, &trBytes, (PULONG_PTR)&pKey,
                                      (OVERLAPPED **)&pOv, INFINITE);

        if (iResult == 0)
        {
            const DWORD errorCode = GetLastError();
            fprintf(stderr, "Client disconnected with error: %lu\n", errorCode);

            // This error code indicates that the socket has been closed
            // and the ClientKey already disposed of
            if (errorCode != WSA_OPERATION_ABORTED)
            {
                RemoveClient(pKey);
            }

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
                RemoveClient(pKey);
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
        int msgLength = (unsigned char)pKey->inBufBase[1];

        // If message type and length have been received
        if (pKey->bytesReceived == 2)
        {
            printf("Receiving message of type %d and length %d\n", msgType, msgLength);

            // Receive the message itself
            pKey->AdvanceInputBuffer(2);
            pKey->ReceiveAsync(msgLength);
            continue;
        }

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
                    printf("Accepted client with name %s\n", username->data());
                }
                else
                {
                    // A user with the same username already exists
                    outMsgType = MessageType::NameTaken;
                    printf("Rejected client with name %s\n", username->data());
                    delete username;
                }

                pKey->outBufBase[0] = outMsgType;
                pKey->lastOutMsgType = outMsgType;

                pKey->SendAsync(1);
            }

            break;

        case MessageType::ClientMessage:
            sendBuf[0] = MessageType::ServerEcho;
            // Account for null-terminating character
            const int usernameLength = pKey->username->size() + 1;
            sendBuf[1] = usernameLength;
            sendBuf[2] = msgLength;

            const char *username = pKey->username->data(),
                *message = pKey->inBufBase + 2;

            printf("%s: %s\n", username, message);

            memcpy(sendBuf + 3, username, usernameLength);
            memcpy(sendBuf + 3 + usernameLength, message, msgLength);

            const int totalLength = 3 + usernameLength + msgLength;

            EnterCriticalSection(&clientsSection);

            for (auto entry : clients)
            {
                ClientKey *key = entry.second;

                memcpy(key->outBufBase, sendBuf, totalLength);
                key->SendAsync(totalLength);
            }

            LeaveCriticalSection(&clientsSection);

            break;
        }

        pKey->ResetInput();
        pKey->bytesExpected = 2;
        pKey->ReceiveAsync(2);
    }

    delete[] sendBuf;
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
