#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 9000
#define BUFSIZE 512

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
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return -1;

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        err_quit("socket()");
        WSACleanup();
        return -1;
    }

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

    retval = listen(serverSocket, 5);
    if (retval == SOCKET_ERROR) {
        err_quit("listen()");
        WSACleanup();
        return -1;
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;

    SOCKET clientSocket;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    addrlen = sizeof(clientaddr);
    clientSocket = accept(serverSocket, (SOCKADDR*)&clientaddr, &addrlen);
    if (clientSocket == INVALID_SOCKET) {
        err_display("accept()");
        WSACleanup();
        return -1;
    }

    printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    int retry = 1;
    int Att = 0, count = 0, game1end = 0, endrequest = 0;
    int howmanywin = 0, howmanylose = 0;

    while (true) {
        while (retry) {

            std::cout << "가위 바위 보 게임을 시작합니다." << std::endl;
            std::cout << "가위(0), 바위(1), 보(2) 중 하나를 선택하세요: " << std::endl;

            int serverHand = rand() % 3;
            int clientHand;
            send(clientSocket, (char*)&serverHand, sizeof(serverHand), 0);
            recv(clientSocket, (char*)&clientHand, sizeof(clientHand), 0);

            std::cout << "서버가 선택한 것: ";
            if (serverHand == 0) {
                std::cout << "가위      ";
            }
            else if (serverHand == 1) {
                std::cout << "바위      ";
            }
            else if (serverHand == 2) {
                std::cout << "보      ";
            }
            std::cout << "클라이언트가 선택한 것: ";
            if (clientHand == 0) {
                std::cout << "가위";
            }
            else if (clientHand == 1) {
                std::cout << "바위";
            }
            else if (clientHand == 2) {
                std::cout << "보";
            }
            std::cout << std::endl;

            if (clientHand == '4') {
                endrequest = 1;
                break;
            }
            else if (serverHand == clientHand) {
                Att = 0;
                std::cout << "무승부입니다." << std::endl;
                retry = 1;
            }
            else if ((serverHand - 1) % 3 == clientHand) {
                Att = 1;
                std::cout << "서버가 이겼습니다." << std::endl;
                retry = 0;
            }
            else {
                Att = -1;
                std::cout << "클라이언트가 이겼습니다." << std::endl;
                retry = 0;
            }
            send(clientSocket, (char*)&Att, sizeof(Att), 0);
        }

        game1end = 0;
        while (!game1end) {
            int serverChoice = rand() % 3;
            send(clientSocket, (char*)&serverChoice, sizeof(serverChoice), 0);

            char clientChoice;
            recv(clientSocket, &clientChoice, sizeof(clientChoice), 0);

            if (clientChoice == '4') {
                endrequest = 1;
                break;
            }
            else if (clientChoice == '3') {
                double winrate = static_cast<double>(howmanywin) / count;
                double loserate = 1.0 - winrate;
                send(clientSocket, (char*)&winrate, sizeof(winrate), 0);
                std::cout << "서버의 승률 : " << loserate << std::endl;
                std::cout << "클라이언트의 승률 : " << winrate << std::endl;
                continue;
            }
            else {
                std::cout << "서버가 선택한 것: ";
                if (serverChoice == 0) {
                    std::cout << "묵      ";
                }
                else if (serverChoice == 1) {
                    std::cout << "찌      ";
                }
                else if (serverChoice == 2) {
                    std::cout << "빠      ";
                }
                std::cout << "클라이언트가 선택한 것: ";
                if (clientChoice == '0') {
                    std::cout << "묵";
                }
                else if (clientChoice == '1') {
                    std::cout << "찌";
                }
                else if (clientChoice == '2') {
                    std::cout << "빠";
                }
                std::cout << std::endl;

                char result[50];

                if (Att == 1) {
                    if (serverChoice == (clientChoice - '0')) {
                        strcpy_s(result, "서버 승리! 클라이언트 패배!");
                        game1end = 1;
                        retry = 1;
                        count += 1;
                    }
                    else if ((serverChoice + 1) % 3 == (clientChoice - '0')) {
                        strcpy_s(result, "공수 유지");
                    }
                    else {
                        strcpy_s(result, "공수 교대");
                        Att = -1;
                    }
                }
                else if (Att == -1) {
                    if (serverChoice == (clientChoice - '0')) {
                        strcpy_s(result, "서버 패배! 클라이언트 승리!");
                        game1end = 1;
                        retry = 1;
                        howmanywin++;
                        count += 1;
                    }
                    else if ((serverChoice + 1) % 3 == (clientChoice - '0')) {
                        strcpy_s(result, "공수 교대");
                        Att = 1;
                    }
                    else {
                        strcpy_s(result, "공수 유지");
                    }
                }
                std::cout << "결과: " << result << std::endl;

                send(clientSocket, result, sizeof(result), 0);
            }
        }
        if (endrequest == 1) {
            if (count > 0) {
                double winrate = static_cast<double>(howmanywin) / count;
                double loserate = 1.0 - winrate;
                send(clientSocket, (char*)&winrate, sizeof(winrate), 0);
                std::cout << "서버의 승률 : " << loserate << std::endl;
                std::cout << "클라이언트의 승률 : " << winrate << std::endl;
            }
            std::cout << "프로그램을 종료합니다" << std::endl;
            break;
        }
        else {
            continue;
        }
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
