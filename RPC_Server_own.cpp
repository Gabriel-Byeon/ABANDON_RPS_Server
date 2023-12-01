#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <ctime>
#include "protocol.h"

#define ROCK 0
#define SCISSORS 1
#define PAPER 2
#define WIN_REQUEST 3
#define END_REQUEST 4

#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 9000
#define BUFSIZE sizeof(Packet)

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
    Packet packet;

    addrlen = sizeof(clientaddr);
    clientSocket = accept(serverSocket, (SOCKADDR*)&clientaddr, &addrlen);
    if (clientSocket == INVALID_SOCKET) {
        err_display("accept()");
        WSACleanup();
        return -1;
    }

    printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    int retry = 1;
    int Att = 0, count = 0;
    int howmanywin = 0;

    std::cout << "����� ������ �Ͻðڽ��ϱ�?(��(1) / �ƴϿ�(0)) : ";
    recv(clientSocket, (char*)&packet, sizeof(Packet), 0);
    
    std::cout << packet.start_game_request << std::endl;

    while (packet.start_game_request) {
        srand(time(NULL));

        while (packet.Game_Choose == 0) {
         
            std::cout << "���� ���� �� ������ �����մϴ�." << std::endl;
            std::cout << "����(0), ����(1), ��(2) �� �ϳ��� �����ϼ���(�·�(3), ����(4)): ";

            int serverHand = rand() % 3;
            packet.choice_S = serverHand;

            send(clientSocket, (char*)&packet, sizeof(Packet), 0);
            recv(clientSocket, (char*)&packet, sizeof(Packet), 0);

            std::cout << packet.choice_C << std::endl;

            if (packet.choice_C == END_REQUEST) {
                packet.end = 1;
                break;
            }

            std::cout << "������ ������ ��: ";
            if (serverHand == 0) {
                std::cout << "����      ";
            }
            else if (serverHand == 1) {
                std::cout << "����      ";
            }
            else if (serverHand == 2) {
                std::cout << "��      ";
            }
            std::cout << "Ŭ���̾�Ʈ�� ������ ��: ";
            if (packet.choice_C == 0) {
                std::cout << "����";
            }
            else if (packet.choice_C == 1) {
                std::cout << "����";
            }
            else if (packet.choice_C == 2) {
                std::cout << "��";
            }
            std::cout << std::endl;

            if (serverHand == packet.choice_C) {
                Att = 0;
                std::cout << "���º��Դϴ�." << std::endl;
                retry = 0;
            }
            else if ((serverHand + 1) % 3 == packet.choice_C) {
                Att = 1;
                std::cout << "������ �����Դϴ�." << std::endl;
                retry = 1;
            }
            else {
                Att = -1;
                std::cout << "Ŭ���̾�Ʈ�� �����Դϴ�." << std::endl;
                retry = 1;
            }

            packet.Att = Att;
            packet.Game_Choose = retry;
            send(clientSocket, (char*)&packet, sizeof(Packet), 0);
        }

        while (packet.Game_Choose) {

            std::cout << "����� ������ �����մϴ�." << std::endl;
            std::cout << "����(0), ����(1), ��(2) �� �ϳ��� �����ϼ���(�·�(3), ����(4)): ";

            recv(clientSocket, (char*)&packet, sizeof(Packet), 0);

            std::cout << packet.choice_C << std::endl;

            if (packet.choice_C == END_REQUEST) {
                packet.end = 1;
                break;
            }
            else if (packet.choice_C == WIN_REQUEST) {
                packet.winrate = (double)howmanywin / count;
                
                send(clientSocket, (char*)&packet, sizeof(Packet), 0);
                std::cout << "������ �·� : " << 1.0 - packet.winrate << std::endl;
                std::cout << "Ŭ���̾�Ʈ�� �·� : " << packet.winrate << std::endl;
                continue;
            }
            else {

                int serverChoice = rand() % 3;
                send(clientSocket, (char*)&serverChoice, sizeof(serverChoice), 0);

                packet.choice_S = serverChoice;

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);
                recv(clientSocket, (char*)&packet, sizeof(Packet), 0);

                std::cout << "������ ������ ��: ";
                if (serverChoice == 0) {
                    std::cout << "����      ";
                }
                else if (serverChoice == 1) {
                    std::cout << "����      ";
                }
                else if (serverChoice == 2) {
                    std::cout << "��      ";
                }
                std::cout << "Ŭ���̾�Ʈ�� ������ ��: ";
                if (packet.choice_C == 0) {
                    std::cout << "����";
                }
                else if (packet.choice_C == 1) {
                    std::cout << "����";
                }
                else if (packet.choice_C == 2) {
                    std::cout << "��";
                }
                std::cout << std::endl;

                char result[50];

                if (packet.Att == 1) {
                    if (serverChoice == packet.choice_C) {
                        strcpy_s(result, "���� �¸�! Ŭ���̾�Ʈ �й�!");
                        packet.Game_Choose = 0;
                        count++;
                    }
                    else if ((serverChoice + 1) % 3 == packet.choice_C) {
                        strcpy_s(result, "���� ����");
                    }
                    else {
                        strcpy_s(result, "���� ����");
                        Att = -1;
                    }
                }
                else if (packet.Att == -1) {
                    if (serverChoice == packet.choice_C) {
                        strcpy_s(result, "���� �й�! Ŭ���̾�Ʈ �¸�!");
                        packet.Game_Choose = 0;
                        howmanywin++;
                        count++;
                    }
                    else if ((serverChoice + 1) % 3 == packet.choice_C) {
                        strcpy_s(result, "���� ����");
                        Att = 1;
                    }
                    else if ((serverChoice - 1) % 3 == packet.choice_C) {
                        strcpy_s(result, "���� ����");
                    }
                }
                std::cout << "���: " << result << std::endl;
                strcpy((char*)& packet.result_str, result);

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);
            }
        }
        if (packet.end == 1) {
            if (count > 0) {
                packet.winrate = double(howmanywin) / count;
                double loserate = 1.0 - packet.winrate;
                send(clientSocket, (char*)&packet, sizeof(Packet), 0);
                std::cout << "������ �·� : " << loserate << std::endl;
                std::cout << "Ŭ���̾�Ʈ�� �·� : " << packet.winrate << std::endl;
            }
            break;
        }
        else {
            continue;
        }

        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();

        return 0;
    }
    std::cout << std::endl << "���α׷��� �����ϰڽ��ϴ�." << std::endl;
    return 0;
}
