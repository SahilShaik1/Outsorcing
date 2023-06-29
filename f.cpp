#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstddef>


#define WSA_SUC 0
#define MSG_LEN 500

struct problem{
    //Format of problem
    double mat[3][3];
    double scalar;
    char command[7];
    problem(double mat[3][3], double scalar, char command[7]){
        memcpy(this->mat, mat, sizeof(this->mat));
        this->scalar = scalar;
        memcpy(this->command, command, sizeof(this->command));
    }
    problem(){}
};

struct solution{
    double mat[3][3];
    double res;
    solution(double m[3][3], double res){
        memcpy(this->mat, m, sizeof(this->mat));
        this->res = res;
    }
    solution(){}
};


struct workerInfo{
    SOCKET* socket;
    char bufferRec[MSG_LEN];
    char bufferSen[MSG_LEN];
    bool active;
    problem prob;
    solution sol;
    workerInfo(SOCKET* socket){
        this->socket = socket;
    }
};






static void reportErr(SOCKET* s = NULL){
    if(s != NULL){
        closesocket(*s);
    }
    std::cout << "Error: " << WSAGetLastError() << "\n";
    WSACleanup();
    abort();
}


DWORD WINAPI recieveMsgs(LPVOID param){
    workerInfo* connection = (workerInfo*)param;
    if(recv(*(connection->socket), connection->bufferRec, sizeof(connection->bufferRec), 0) == SOCKET_ERROR){
        std::cout << "Error in recieving Message.\n";
    }
    std::cout << "Recieved: " << connection->bufferRec << "\n";
    return 0;
}

DWORD WINAPI sendMsgs(LPVOID param){
    workerInfo* connection = (workerInfo*)param;
    send(*(connection->socket), connection->bufferSen, sizeof(connection->bufferSen), 0);
    return 0;
}

DWORD WINAPI solveMsg(LPVOID param){

    problem* prob = (problem *) param;
    if(prob->command == "SCALAR"){
        std::cout << "Scalar transform a matrix";
    } else {
        char* buffer = (char*)param;
        char* token = strtok(buffer, " ");
        int f = *token - '0';
        char* op = strtok(NULL, " ");
        token = strtok(NULL, " ");
        int s = *token - '0';
        if(*op == 'x'){
            std::cout << "\nThe answer is " << f * s << "\n";
        }
    }
    return 0;
}

int main(){
    WORD verReq = MAKEWORD(2, 2);
    const char * ip = "10.0.0.193";
    unsigned short const port = 55555;
    WSADATA holder;
    

    int wsa = WSAStartup(verReq, &holder);
    
    if(wsa == WSA_SUC){
        std::cout << "Startup Finished.\n";
    } else {
        std::cout << "Error Code:" << wsa << "\n";
        abort();
    }


    
    SOCKET connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in connectionInfo;
    if(connection  != INVALID_SOCKET){
        std::cout << "Socket Creation Successful.\n";
    } else {
        std::cout << "Failed to create Socket.\n";
        reportErr();
    }


    connectionInfo.sin_family = AF_INET;
    PVOID cRefAddr = &connectionInfo.sin_addr.S_un.S_addr;
    inet_pton(AF_INET, ip, cRefAddr);
    connectionInfo.sin_port = htons(port);


    if(connect(connection, (SOCKADDR *)&connectionInfo, sizeof(connectionInfo)) != INVALID_SOCKET){
        std::cout << "Successfully connected to server.\n";
    } else {
        std::cout << "Failed to connect to server\n";
        reportErr();
    }
    char buf[MSG_LEN];
    //Replace garbage values with 0
    ZeroMemory(buf, MSG_LEN);
    recv(connection, buf, sizeof(buf), 0);
    std::cout << "Recieved: " << buf;
    return 0;
}
