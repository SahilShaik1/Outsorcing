//Concept of outsorcing implemented

#include <windows.h>
#include <iostream>
#include <cstddef>
#include <vector>

#define WSA_SUC 0
#define BIND_SUC 0
#define LIST_SUC 0
#define MAX_CONNECTIONS 1
struct problem{
    //Format of problem
    double mat[3][3];
    double scalar;
    char command[7];
    problem(double m[3][3], double scalar, char command[7]){
        memcpy(mat, m, sizeof(mat));
        this->scalar = scalar;
        memcpy(this->command, command, sizeof(this->command));
    }
    problem(){}
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



struct data {
    int f;
    data(int f = 0){
        this->f = f;
    }; 
};





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
    
    while(workers.size() < MAX_CONNECTIONS){
        SOCKET worker;
        sockaddr_in workerInfo;
        worker = accept(connection, (SOCKADDR* )&workerInfo, NULL);
        if (worker != INVALID_SOCKET){
            std::cout << "Connected with a Client on port "<< workerInfo.sin_port << "\n";
            workers.push_back(&worker);
        } else {
            std::cout << "Failed to accept.\n";
            reportErr(&connection, workers);
        }
    }
    double mat[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    char b[7] = "SCALAR";
    problem prob(mat, 12, b);

    send(*workers[0], (char *)&prob, sizeof(problem), 0);
    /*
    std::vector<std::vector<double>> matrix = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    for(int i = 0; i < workers.size(); i++){
        const char* cmd = "SCALAR";
        problem* prob = new problem(matrix, 3, cmd);
        std::cout << "Command Ready: " << prob->command << "\n";
        
        char* addr = (char*)prob;
        
        int res = send(*(workers[i]), addr, sizeof(addr), 0);
        if(res != sizeof(cmd)){
            std::cout << "Error in sending Command.\n";
            reportErr(&connection, workers);
        } else {
            std::cout << "Sent Command.\n";
        }
    }
    */

    

    return 0;
}
