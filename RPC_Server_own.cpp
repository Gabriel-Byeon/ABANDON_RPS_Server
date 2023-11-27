#define _WINSOCK_DEPRECATED_NO_WARNINGS //  �ֽ� VC++ �����Ͻ� ��� ����
#include <iostream> //
#include <winsock2.h>   // ������ ����ϱ� ���� ��� ����, API���� ����ϴ� ���, ������ ���� �� �Լ� ������Ÿ�� ����
#include <WS2tcpip.h>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")  // ������ ������ ������ϵ��� ������ �������� ��ũ, ws2_32.lib ���̺귯�� ������ ���α׷��� ��ũ�ϵ��� �����Ϸ��� �˸��� pragma ���þ�

#define SERVERPORT 9000
#define BUFSIZE 512

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[]) {

    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
        return -1;

    // socket()
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // ������ �����ϴ� �Լ� ȣ��
    if (serverSocket == INVALID_SOCKET) {
        err_quit("socket()");
        WSACleanup();
        return -1;
    }

    // bind()
    SOCKADDR_IN serverAddress; 
    ZeroMemory(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVERPORT);
    retval = bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (retval == SOCKET_ERROR) {
        err_quit("bind()");
        WSACleanup();
        return -1;
    }

    // listen()
    retval = listen(serverSocket, 5);
    if (retval == SOCKET_ERROR) {
        err_quit("listen()");
        WSACleanup();
        return -1;
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;

    // ������ ��ſ� ����� ����
    SOCKET clientSocket;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    // accept()
    addrlen = sizeof(clientaddr);
    clientSocket = accept(serverSocket, (SOCKADDR*)&clientaddr, &addrlen);
    if (clientSocket == INVALID_SOCKET) {
        err_display("accept()");
        WSACleanup();
        return -1;
    }

    // ������ Ŭ���̾�Ʈ ���� ���
    printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    // ���� ���� �� ����
    int retry = 1;
    int Att = 0, count = 0;
    int howmanywin = 0;

    while (retry == 1) {

        std::cout << "���� ���� �� ������ �����մϴ�." << std::endl;
        std::cout << "����(0), ����(1), ��(2) �� �ϳ��� �����ϼ���(�·� (3), ���� (4)): ";

        int serverHand = rand() % 3;  // 0: ����, 1: ����, 2: ��
        int clientHand;
        recv(clientSocket, (char*)&clientHand, sizeof(clientHand), 0);

        std::cout << "������ ������ ��: ";
        if (clientHand == 0) {
            std::cout << "����";
        }
        else if (clientHand == 1) {
            std::cout << "����";
        }
        else if (clientHand == 2) {
            std::cout << "��";
        }
        else if (clientHand == 3) {
            std::cout << "�·�";
        }
        else {
            break;
        }
        std::cout << std::endl;

        // ���� ��� ����
        if (clientHand == '4') {
            break;
        }
        else if (serverHand == clientHand) {
            Att = 0;  // ���º�
            retry = 1;
        }
        else if ((serverHand + 1) % 3 == clientHand) {
            Att = 1;  // ���� �¸�
            retry = 0;
        }
        else {
            Att = -1;  // Ŭ���̾�Ʈ �¸�
            retry = 0;
        }

        send(clientSocket, (char*)&Att, sizeof(Att), 0);
    }
    

    // ����� ����
    // (��: 0, ��: 1, ��: 2)
    while (true) {
        int serverChoice = rand() % 3;
        send(clientSocket, (char*)&serverChoice, sizeof(serverChoice), 0);

        char clientChoice;
        recv(clientSocket, &clientChoice, sizeof(clientChoice), 0);

        char result[50];

        if ((clientChoice != '3') && (clientChoice != '4')) {
            if (Att == 1) {
                if (serverChoice == (clientChoice - '0')) {
                    strcpy_s(result, "�й�!");
                }
                else if ((serverChoice + 1) % 3 == (clientChoice - '0')) {
                    strcpy_s(result, "���� ����");
                }
                else {
                    strcpy_s(result, "���� ����");
                    Att = -1;
                }
            }
            else if (Att == -1) {
                if (serverChoice == (clientChoice - '0')) {
                    strcpy_s(result, "�¸�!");
                    howmanywin++;
                }
                else if ((serverChoice + 1) % 3 == (clientChoice - '0')) {
                    strcpy_s(result, "���� ����");
                    Att = 1;
                }
                else {
                    strcpy_s(result, "���� ����");
                }
            }
        }
        else if (clientChoice == '4')
            break;
        

        count++;
        
        //float winrate_server = howmanylose / count;
        //float winrate_client = howmanywin / count;
        
        send(clientSocket, result, sizeof(result), 0);
        //send(clientSocket, (char*)&winrate_server, sizeof(winrate_server), 0);

        char playAgain;
        recv(clientSocket, &playAgain, sizeof(playAgain), 0);

        if (playAgain != 'y') {
            break;
        }
    }

    // ���� �� ���� ���� �ݱ�
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
