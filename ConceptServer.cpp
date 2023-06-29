//Concept of outsorcing implemented

#include <windows.h>
#include <iostream>
#include <cstddef>
#include <vector>

#define WSA_SUC 0
#define BIND_SUC 0
#define LIST_SUC 0
#define MAX_CONNECTIONS 2
#define MSG_LEN 500
struct problem{
    //Format of problem
    double mat[3][3];
    double scalar;
    char command[7];
    problem(double m[3][3], double scalar, char command[7]){
        memcpy(this->mat, m, sizeof(this->mat));
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


struct workerConnection{
    SOCKET* socket;
    char bufferRec[MSG_LEN];
    char bufferSen[MSG_LEN];
    bool active;
    problem prob;
    solution sol;
    workerConnection(SOCKET* socket){
        this->socket = socket;
    }
};


static void reportErr(SOCKET* s = NULL, std::vector<SOCKET*> workers = {NULL}){
    if(s != NULL){
        closesocket(*s);
    }
    if(workers[0] != NULL){
        for(SOCKET* worker : workers){
            closesocket(*worker);
        }
    }
    std::cout << "Error: " << WSAGetLastError() << "\n";
    WSACleanup();
    abort();
}





int main() {
    WORD verReq = MAKEWORD(2, 2);
    //10.0.0.193
    const std::string ip = "10.0.0.193";
    unsigned short const port = 55555;
    WSAData wsaDataHolder;
    

    int wsa = WSAStartup(verReq, &wsaDataHolder);
    if (wsa == WSA_SUC){
        std::cout << "Startup Successful.\n";
        std::cout << "Version Requested: " << wsaDataHolder.wVersion << ".\n";
        std::cout << "Description: " << wsaDataHolder.szDescription << ".\n";
    } else {
        std::cout << "Failed Startup.\n";
        std::cout << "Error Code: " << wsa << "\n";
        WSACleanup();
        abort();
    }



    SOCKET connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(connection  != INVALID_SOCKET){
        std::cout << "Socket Creation Successful.\n";
    } else {
        std::cout << "Failed to create Socket.\n";
        reportErr(&connection);
    }
    sockaddr_in connectionInfo;


    //htons and inet simply convert strings/shorts to the correct format
    connectionInfo.sin_family = AF_INET;
    connectionInfo.sin_addr.S_un.S_addr = INADDR_ANY;
    PVOID refAddr = &connectionInfo.sin_addr.s_addr;
    connectionInfo.sin_port = htons(port);

    int bindRes = bind(connection, (SOCKADDR*)&connectionInfo, sizeof(connectionInfo));


    if (bindRes == BIND_SUC){
        std::cout << "Successfully binded socket.\n";
    } else {
        std::cout << "Failed to bind socket.\n";
        reportErr(&connection);
    }

    


    int listRes = listen(connection, MAX_CONNECTIONS);
    if(listRes != INVALID_SOCKET){
        std::cout << "Successfully started Listening...\n";
    } else {
        std::cout << "Failed to listen.\n";
        reportErr(&connection);
    }


    std::vector<SOCKET*> workers;
    std::vector<sockaddr_in> workerInfo;
    while(workers.size() < MAX_CONNECTIONS){
        SOCKET worker;
        sockaddr_in workerConnectionInfo;
        worker = accept(connection, (SOCKADDR* )&workerConnectionInfo, NULL);
        if (worker != INVALID_SOCKET){
            std::cout << "Connected with a Client on port "<< workerConnectionInfo.sin_port << "\n";
            
        } else {
            std::cout << "Failed to accept.\n";
            reportErr(&connection, workers);
        }
        workerInfo.push_back(workerConnectionInfo);
        workers.push_back(&worker);
    }
    for(int i = 0; i < workers.size(); i++){
        std::cout << "Connection ADDRESS: " << workers[i] << "\n";
    }

    double mat[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    char b[7] = "SCALAR";

    for(int worker = 0; worker < workers.size(); worker++){
        workerConnection work(workers[worker]);
        std::cout << "Connection address: " << work.socket << "\n";

        problem prob(mat, 12, b);

        int s = send(*workers[worker], (char *)&prob, sizeof(problem), 0);
        if(s != sizeof(problem)){
            std::cout << "Error Sending to Client " << worker + 1 << "\n";
        }
        solution sol;

        int f = recv(*workers[worker], (char *)& sol, sizeof(solution), 0);
        if(f != sizeof(solution)){
            std::cout << "Error Recieving to Client " << worker + 1 << "\n";
        }
        std::cout << "Recieved from port " << workerInfo[worker].sin_port << ":\n";
        for(int i = 0; i < 3; i++){
            for(int z = 0; z < 3; z++){
                std::cout << sol.mat[i][z] << "\t";
            }
            std::cout << "\n";
        }
    }
    return 0;
}
