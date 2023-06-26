#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstddef>
#include <vector>

#define WSA_SUC 0
#define MSG_LEN 500


struct socket_connection{
    SOCKET* socket;
    char bufferRec[MSG_LEN];
    char bufferSen[MSG_LEN];
    socket_connection(SOCKET* socket){
        this->socket = socket;
    }
};

struct problem{
    //Format of problem
    std::vector<std::vector<double>> mat;
    std::vector<std::vector<double>> ans;
    double scalar;
    std::string command;

    problem(std::vector<std::vector<double>> mat, double scalar, std::string command){
        this->mat = mat;
        this->scalar = scalar;
        this->command = command;
    }
};


struct data {
    int f;
    data(int f = 0){
        this->f = f;
    }; 
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
    socket_connection* connection = (socket_connection*)param;
    if(recv(*(connection->socket), connection->bufferRec, sizeof(connection->bufferRec), 0) == SOCKET_ERROR){
        std::cout << "Error in recieving Message.\n";
    }
    std::cout << "Recieved: " << connection->bufferRec << "\n";
    return 0;
}

DWORD WINAPI sendMsgs(LPVOID param){
    socket_connection* connection = (socket_connection*)param;
    send(*(connection->socket), connection->bufferSen, sizeof(connection->bufferSen), 0);
    return 0;
}

DWORD WINAPI solveMsg(LPVOID param){

    problem* prob = (problem *) param;
    if(prob->command == "SCALAR"){
        std::vector<std::vector<double>> res;
        std::vector<double> temp;
        for(int i = 0; i < prob->mat.size(); i++){
            for(int j = 0; j < prob->mat[i].size(); j++){
                temp.push_back(prob->mat[i][j] * prob->scalar);
            }
            res.push_back(temp);
            temp.clear();
        }
        prob->ans = res;
        for(int i = 0; i < prob->mat.size(); i++){
            for(int j = 0; j < prob->mat[i].size(); j++){
                std::cout << (prob->ans)[i][j] << "\t";
            }
            std::cout << "\n";
        }
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
    const char * ip = "127.0.0.1";
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
    data dat;
    recv(connection, (char *)&dat, sizeof(data), 0);
    std::cout << "Recieved Msg: " << dat.f;
    /*
    socket_connection* con = new socket_connection(&connection);
    DWORD recieveID;
    HANDLE rec = CreateThread(NULL, 0, recieveMsgs, con, false, &recieveID);
    WaitForSingleObject(rec, INFINITE);
    
    
    DWORD solveID;
    
    
    
    std::string msg = con->bufferRec;
    std::cout << "Command is " << con->bufferRec << "\n";
    
    if(msg == "SCALAR"){
        char* addr;
        recv(*(con->socket), addr, sizeof(addr), 0);
        problem* prob = (problem *)addr;
        std::vector<std::vector<double>> mat = prob->mat;
        HANDLE sol = CreateThread(NULL, 0, solveMsg, prob, false, &solveID);
        WaitForSingleObject(sol, INFINITE);
    } else{
        char *prob = &(con->bufferRec)[0];
        HANDLE sol = CreateThread(NULL, 0, solveMsg, prob, false, &solveID);
        std::cout << con->bufferRec << "\n";
        WaitForSingleObject(sol, INFINITE);
    }
    */
    return 0;
}
