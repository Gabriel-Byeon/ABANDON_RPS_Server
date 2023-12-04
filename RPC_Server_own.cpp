#define _CRT_SECURE_NO_WARNINGS //strcpy �Լ��� ũ�� ���� ���� ����� ���ֱ� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#include <iostream> // ������� ����ϱ� ����
#include <winsock2.h>   // ������ ����ϱ� ���� ��� ����, API���� ����ϴ� ���, ������ ���� �� �Լ� ������Ÿ�� ����
#include <WS2tcpip.h>
#include <ctime>
#include "protocol.h"   // �������� ��� ����

#define ROCK 0              
#define SCISSORS 1
#define PAPER 2
#define WIN_REQUEST 3
#define END_REQUEST 4   // ���α׷����� ���� ���ǰų� Ư���� Ʈ���Ÿ� �䱸�ϴ� ���� ���� ����

#pragma comment(lib, "ws2_32.lib")  // ������ ������ ������ϵ��� ������ �������� ��ũ, ws2_32.lib ���̺귯�� ������ ���α׷��� ��ũ�ϵ��� �����Ϸ��� �˸��� pragma ���þ�

#define SERVERPORT 9000 // ��ſ� ���Ǵ� ��Ʈ�� ����
#define BUFSIZE sizeof(Packet)  // ��ſ� ���Ǵ� ������ ����� Packet�� �����ŭ���� ���Ǥ�

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;

    // WSAGetLastError �Լ��� ����Ͽ� �ֱٿ� �߻��� ���� �Լ��� ���� �ڵ带 ����
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    // MessageBox�� ����Ͽ� ���� �޼����� �˾� â�� ǥ��
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    // �Ҵ�� �޸𸮸� ����
    LocalFree(lpMsgBuf);
    // ���α׷��� ������������ ����
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    // err_quit �Լ��� �����ϰ� ����������, MessageBox ��� printf�� ����Ͽ� �ֿܼ� ���� �޼��� ���
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    // ���� �޼��� ���
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    // �Ҵ�� �޸𸮸� ����
    LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[]) {

    int retval;
    WSADATA wsaData;        // Winsock �ʱ�ȭ

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  // Winsock ���� 2.2�� ȣ���ϰ� �̸� �����Ѵ�, �� ���� �ùٸ��� �ʴٸ�
        return -1;  // ������ �ִٴ� ���� ��ȯ

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // ���� serverSocket�� IPv4 �ּ� ü��, TCP ��������, SOCK_STREAM ���� Ÿ���� ����Ͽ� ����
    if (serverSocket == INVALID_SOCKET) {   // ���� ������ �߸��� �����̶��
        err_quit("socket()");   // "socket()"�̶�� �޼����� ��� �� ���α׷� ����
        WSACleanup();   // Winsock ����
        return -1;  // ������ �߻��ߴٴ� ���� ��ȯ
    }

    SOCKADDR_IN serverAddress;  // ���� �ּ� serverAddress ����
    ZeroMemory(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; // IPv4 �ּ� ü��
    serverAddress.sin_addr.s_addr = INADDR_ANY; // �ּҴ� 
    serverAddress.sin_port = htons(SERVERPORT); // 9000�� ��Ʈ
    retval = bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));  // �������� ���Ͽ� IP�� ��Ʈ�� �Ҵ��Ͽ� ��Ʈ��ũ ��Ʈ���̽��� ���� �� �ְ� ��.
    if (retval == SOCKET_ERROR) {   // 
        err_quit("bind()"); // "bind()"�̶�� �޼����� ��� �� ���α׷� ����
        WSACleanup();   // Winsock ����
        return -1;  // ������ �߻��ߴٴ� ���� ��ȯ
    }

    retval = listen(serverSocket, 5);   // Ŭ���̾�Ʈ�κ��� ������ ��ٸ���, ���ÿ� �����ϴ� �ִ� Ŭ���̾�Ʈ ���� 5���� �����Ѵ�
    if (retval == SOCKET_ERROR) {   // ���� ������ �߻��ϸ�
        err_quit("listen()");   // "listen()"�̶�� �޼����� ��� �� ���α׷� ����
        WSACleanup();   // Winsock ����
        return -1;  // ������ �߻��ߴٴ� ���� ��ȯ
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;    // ������ Ŭ���̾�Ʈ���� ������ ��ٸ��� �ִٴ� �ش� ���� ���

    SOCKET clientSocket;        // clientSocket�̶�� ������ ����
    SOCKADDR_IN clientaddr;     // ���� �ּ� ����ü ������ clientaddr ����
    int addrlen;    // �ּ� ���̸� ��Ÿ���� addrlen ���� ����
    Packet packet;  // Packet ����ü ������ packet ����

    addrlen = sizeof(clientaddr);   // clientaddr ����� addrlen�� ����
    clientSocket = accept(serverSocket, (SOCKADDR*)&clientaddr, &addrlen);  // ���������� Ŭ���̾�Ʈ�� ���� ��û�� �㰡�Ѵ�.
    if (clientSocket == INVALID_SOCKET) {   // clientSocket�� �ùٸ� ������ �ƴ϶��
        err_display("accept()");    // "accept()"�̶�� �޼����� ��� �� ���α׷� ����
        WSACleanup();   // Winsock ����
        return -1;  // ������ �߻��ߴٴ� ���� ��ȯ
    }

    printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));    // ��� ���۽� ������ ����� Ŭ���̾�Ʈ�� IP �ּҿ� ��Ʈ �ּҸ� ���

    int retry = 0;      // ���������� ���Ӱ� ����� ���� �� ���� ������ �������� �����ϴ� ����
    int Att = 0, count = 0; // Att�� ���� ��ȸ�� �������� �ִ��� �����ϴ� ����(1�̸� ���� ����, -1�̸� Ŭ���̾�Ʈ ����), count ������ ������ �� ������ ��Ÿ���� ���� 
    int howmanywin = 0; // Ŭ���̾�Ʈ�� �� �� �� �¸��Ͽ����� ��Ÿ���� �Լ�

    std::cout << "����� ������ �����Ͻðڽ��ϱ�? " << "( ��(1) / �ƴϿ�(0) ) : ";
    recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // Ŭ���̾�Ʈ���� ������ ��Ŷ�� ����
    
    std::cout << packet.start_game_request << std::endl;    // ��Ŷ�� start_game_request �ʵ��� ���� ȭ�鿡 ���

    while (packet.start_game_request) {     // ��Ŷ�� start_game_request �ʵ尪�� 1�̸� �ݺ�
        srand(time(NULL));      // ������ ���� ������ �������� �ϱ����� �ð��� �õ尪���� ����

        while (packet.Game_Choose == 0) {   // ��Ŷ�� Game_Choose �ʵ尪�� 0�̸� ���������� ���� ����
            
            std::cout << std::endl << "������ ���� ���� ���� �� ������ �����մϴ�!!" << std::endl;
            std::cout << "����(0), ����(1), ��(2) �� �ϳ��� �����ϼ���(�·�(3), ����(4)): ";

            int serverHand = rand() % 3;    // ������ 0,1,2 3�� �� �������� ����
            packet.choice_S = serverHand;   // ������ ���ð��� ��Ŷ�� choice_S �ʵ忡 ����

            send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // ��Ŷ�� Ŭ���̾�Ʈ�� ����
            recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // Ŭ���̾�Ʈ�� ������ ��Ŷ�� ����

            std::cout << serverHand << std::endl;   // ������ �������� ������ ���� ȭ�鿡 ���

            if (packet.choice_C == END_REQUEST) {   // ��Ŷ�� choice_C �ʵ��� ���� 4���
                packet.end = 1;     // ��Ŷ�� end �ʵ� ���� 1�� ����
                break;      // �ݺ����� ���� ����
            }
            else if (packet.choice_C == WIN_REQUEST) {  // ��Ŷ�� choice_C �ʵ尪�� 3�̶��
                if (count > 0) {        // ������ �� �� �̻� �������� ��
                    packet.winrate = (double)howmanywin / count;    // ��Ŷ�� winrate �ʵ忡 Ŭ���̾�Ʈ�� �·��� ����
                    packet.count = count;   // ��Ŷ�� count �ʵ忡 ������ ���� ���� ����
                    packet.win = howmanywin;    // ��Ŷ�� win �ʵ忡 Ŭ���̾�Ʈ�� �¸��� ���� ���� �Է�

                    int serverwin = count - howmanywin; // ���ºΰ� �����Ƿ� ������ �̱� Ƚ���� �� ���� ������ Ŭ���̾�Ʈ�� �¸��� Ƚ���� �� ��

                    send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // ��Ŷ�� Ŭ���̾�Ʈ�� ����
                    std::cout << std::endl << "<<<<�·� ���� ���>>>>" << std::endl;  
                    std::cout << std::endl << "��� �� : " << count << " ��" << std::endl;  // ��� �� ���
                    std::cout << std::endl << "������ �̱� ��� �� : " << serverwin << " ��" << std::endl;   // ������ �¸��� Ƚ�� ���
                    std::cout << std::endl << "������ �·� : " << (1.0 - packet.winrate) * 100 << "%" << std::endl;  // ������ �·� ���
                    std::cout << std::endl << "Ŭ���̾�Ʈ�� �̱� ��� �� : " << howmanywin << " ��" << std::endl;   // Ŭ���̾�Ʈ�� �¸��� Ƚ�� ���
                    std::cout << std::endl << "Ŭ���̾�Ʈ�� �·� : " << packet.winrate * 100 << "%" << std::endl;   // Ŭ���̾�Ʈ�� �·� ���
                    continue; // �ݺ����� ó������ ���ư��� ������ �ٽ� ����
                }
                std::cout << std::endl << "���� �� �ǵ� ���ϼ̽��ϴ�." << std::endl;   // count�� 0�̸� �ش� ���� ���
                continue;   // �ݺ����� ó������ ���ư��� ������ �ٽ� ����
            }

            std::cout << "������ ������ ��: ";
            if (serverHand == ROCK) {   // ������ 0�� �����ߴٸ�
                std::cout << "����      ";
            }
            else if (serverHand == SCISSORS) {  // ������ 1�� �����ߴٸ�
                std::cout << "����      ";
            }
            else if (serverHand == PAPER) { // ������ 2�� �����ߴٸ�
                std::cout << "��      ";
            }
            std::cout << "Ŭ���̾�Ʈ�� ������ ��: ";
            if (packet.choice_C == ROCK) {  // Ŭ���̾�Ʈ�� 0�� �����ߴٸ�(choice_C �ʵ��� ���� 0�̶��)
                std::cout << "����";
            }
            else if (packet.choice_C == SCISSORS) { // Ŭ���̾�Ʈ�� 1�� �����ߴٸ�(choice_C �ʵ尪�� 1�̶��)
                std::cout << "����";
            }
            else if (packet.choice_C == PAPER) {    // Ŭ���̾�Ʈ�� 2�� �����ߴٸ�(choice_C �ʵ��� ���� 2���
                std::cout << "��";
            }
            std::cout << std::endl;

            if (serverHand == packet.choice_C) {    // ������ ������ ���� Ŭ���̾�Ʈ�� ������ ��(choice_C �ʵ��� ��)�� ���ٸ�
                Att = 0;    // ���� ��ȸ�� �ƹ����Ե� ����
                std::cout << "���º��Դϴ�!! ���⸦ �����ϼ���!" << std::endl;
                retry = 0;  // ���������� ������ �ٽ� ����
            }
            else if ((serverHand + 1) % 3 == packet.choice_C) { // ������ Ŭ���̾�Ʈ�� ���������� ���ӿ��� �¸��Ͽ��ٸ�
                Att = 1;    // ���� ��ȸ�� �������� �ְ�
                std::cout << "������ �����Դϴ�! ��ȸ�� ��ġ�� ������!!" << std::endl;
                retry = 1;  // ����� ������ ����
            }
            else if ((serverHand + 2) % 3 == packet.choice_C) { // ������ Ŭ���̾�Ʈ�� ���������� ���ӿ��� �й��Ͽ��ٸ�
                Att = -1;   // ���� ��ȸ�� Ŭ���̾�Ʈ���� �ְ�
                std::cout << "�ƽ�����! Ŭ���̾�Ʈ�� �����Դϴ�! " << std::endl;
                retry = 1;  // ����� ������ ����
            }

            packet.Att = Att;   // ���� ��ȸ�� �������� �ִ��� ���� ������ �ִ� Att ������ ���� ��Ŷ�� Att �ʵ忡 �Է�
            packet.Game_Choose = retry; // ���������� ���Ӱ� ����� ���� �� ������ ������ ������ ���� ������ �ִ� retry ������ ���� ��Ŷ�� Game_Choose �ʵ忡 ����
            send(clientSocket, (char*)&packet, sizeof(Packet), 0); // ��Ŷ�� Ŭ���̾�Ʈ�� ����
        }

        while (packet.Game_Choose) {    // ��Ŷ�� Game_Choose ���� 1�̸� ����� ������ ����

            std::cout << std::endl << "����� ������ �����մϴ�!!" << std::endl;
            std::cout << "����(0), ����(1), ��(2) �� �ϳ��� �����ϼ���(�·�(3), ����(4)): ";

            recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // Ŭ���̾�Ʈ�� ������ ��Ŷ�� ����
            
            int serverChoice = rand() % 3;  // ������ 0,1,2 �� ������ �� ����
            packet.choice_S = serverChoice; // ������ ������ choice_S �ʵ忡 �Է�
            
            std::cout << serverChoice << std::endl; // ������ ������ ȭ�鿡 ���

            if (packet.choice_C == END_REQUEST) {   // Ŭ���̾�Ʈ�� ������ 4���
                packet.end = 1;     // end �ʵ忡 1�� ����
                break;  // �ݺ����� Ż��
            }
            else if (packet.choice_C == WIN_REQUEST) {  // Ŭ���̾�Ʈ�� ������ 3�̶��
                if (count > 0) {    // ������ �� ���̻� �����ߴٸ�
                    packet.winrate = (double)howmanywin / count;    // ��Ŷ�� winrate �ʵ忡 Ŭ���̾�Ʈ�� �·��� �Է�
                    packet.count = count;   // count �ʵ忡 ������ ��� ���� �Է�
                    packet.win = howmanywin;    // win �ʵ忡 Ŭ���̾�Ʈ�� �¸��� ��� ���� �Է�

                    int serverwin = count - howmanywin; // ���ºδ� �����Ƿ� �� ��� ������ Ŭ���̾�Ʈ�� ������ ��� ���� ���� ������ �¸��� Ƚ��

                    send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // Ŭ���̾�Ʈ�� ��Ŷ ����
                    std::cout << std::endl << "<<<<�·� ���� ���>>>>" << std::endl;  
                    std::cout << std::endl << "��� �� : " << count << " ��" << std::endl;  // �� ��� �� ���
                    std::cout << std::endl << "������ �̱� ��� �� : " << serverwin << " ��" << std::endl;   // ������ �¸��� Ƚ�� ���
                    std::cout << std::endl << "������ �·� : " << (1.0 - packet.winrate) * 100 << "%" << std::endl;  // ������ �·� ���
                    std::cout << std::endl << "Ŭ���̾�Ʈ�� �̱� ��� �� : " << howmanywin << " ��" << std::endl;   // Ŭ���̾�Ʈ�� �¸��� Ƚ�� ���
                    std::cout << std::endl << "Ŭ���̾�Ʈ�� �·� : " << packet.winrate * 100 << "%" << std::endl;   // Ŭ���̾�Ʈ�� �·� ���
                    continue;   // �ݺ����� ó������ ���ư� �ٽ� ����� ���� ����
                }
                std::cout << std::endl << "�� ���� �����Ͻð� �·��� ��û���ּ���." << std::endl;
                continue;   // �ݺ����� ó������ ���ư� �ٽ� ����� ���� ����
            }
            else {         // ����� �� ����� ���� ����   

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // Ŭ���̾�Ʈ�� ��Ŷ ����
                recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // Ŭ���̾�Ʈ�� ������ ��Ŷ ����

                std::cout << "������ ������ ��: ";
                if (serverChoice == ROCK) { // ������ 0�� �����ߴٸ�
                    std::cout << "����      ";
                }
                else if (serverChoice == SCISSORS) {    // ������ 1�� �����ߴٸ�
                    std::cout << "����      ";
                }
                else if (serverChoice == PAPER) {   // ������ 2�� �����ߴٸ�
                    std::cout << "��      ";
                }
                std::cout << "Ŭ���̾�Ʈ�� ������ ��: ";
                if (packet.choice_C == ROCK) {  // Ŭ���̾�Ʈ�� 0�� �����ߴٸ�
                    std::cout << "����";
                }
                else if (packet.choice_C == SCISSORS) { // Ŭ���̾�Ʈ�� 1�� �����ߴٸ�
                    std::cout << "����";
                }
                else if (packet.choice_C == PAPER) {    // Ŭ���̾�Ʈ�� 2�� �����ߴٸ�
                    std::cout << "��";
                }
                std::cout << std::endl;

                char result[50];    // ��� �޼����� �����ϱ� ���� ���ڿ� result ����
                if (packet.Att == 1) {  // ���� ��ȸ�� �������� �־����ٸ�
                    if (serverChoice == packet.choice_C) {  // ������ Ŭ���̾�Ʈ�� ���� ���� �������� ��
                        strcpy(result, "���� �¸�! Ŭ���̾�Ʈ �й�!"); // result ������ �ش� ���� ����
                        packet.Game_Choose = 0; // �ٽ� ���������� ������ ���ư��� ���� Game_Choose �ʵ尪�� 0���� ����
                        count++;    // ������ ��� �� 1 ����
                    }
                    else if ((serverChoice + 1) % 3 == packet.choice_C) {   // ������ ������ ���� ��ȸ�� ������ �ִٸ�
                        strcpy_s(result, "���� ����, ���� ������");  // result ������ �ش� ���� ����

                    }
                    else if ((serverChoice + 2) % 3 == packet.choice_C) {   // Ŭ���̾�Ʈ�� ���� ��ȸ�� �����Դٸ�
                        strcpy(result, "���� ����, Ŭ���̾�Ʈ ����");  // result ������ �ش� ���� ����
                        packet.Att = -1;    // ���� ��ȸ�� Ŭ���̾�Ʈ���� �Ѿ�ٴ� ���� �˸��� ���� ��Ŷ�� Att �ʵ忡 -1�� �Է�
                    }
                }
                else if (packet.Att == -1) {    // ���� ��ȸ�� Ŭ���̾�Ʈ���� �ִٸ�
                    if (serverChoice == packet.choice_C) {  // ������ Ŭ���̾�Ʈ�� ���� ������ �ߴٸ�
                        strcpy(result, "���� �й�! Ŭ���̾�Ʈ �¸�!");  // result ������ �ش� ���� ����
                        packet.Game_Choose = 0; // ���������� ������ �ٽ� �����ϱ� ���� ��Ŷ�� Game_Choose �ʵ� ���� 0���� ����
                        howmanywin++;   // Ŭ���̾�Ʈ�� �¸��� Ƚ�� 1 ����
                        count++;    // �� ��� �� 1 ����
                    }
                    else if ((serverChoice + 1) % 3 == packet.choice_C) {   // ������ ���� ��ȸ�� �����Դٸ�
                        strcpy(result, "���� ����, ���� ����");  // result ������ �ش� ���� ����
                        packet.Att = 1; // ���� ��ȸ�� �������� �Ѿ�ٴ� ���� �˸��� ���� ��Ŷ�� Att �ʵ忡 1�� �Է�
                    }
                    else if ((serverChoice + 2) % 3 == packet.choice_C) {   // Ŭ���̾�Ʈ�� ������ ���� ��ȸ�� �������ִٸ�
                        strcpy(result, "���� ����, Ŭ���̾�Ʈ ������");  // result ������ �ش� ���� ����
                    }
                }
                std::cout << std::endl << "���: " << result << std::endl;    // result ������ �Էµ� ���� ���
                strcpy((char*)& packet.result_str, result); // ��Ŷ�� result_str �ʵ忡 result ���� ���� �Է�

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // ��Ŷ�� Ŭ���̾�Ʈ�� ����
            }
        }
        if (packet.end == 1) {  // ��Ŷ�� end �ʵ� ���� 1�̸�
            if (count > 0) {    // ������ �� �� �̻� �����ߴٸ�
                packet.winrate = (double)howmanywin / count;    // ��Ŷ�� winrate �ʵ忡 Ŭ���̾�Ʈ�� �·� �Է�
                packet.count = count;   // ��Ŷ�� count �ʵ忡 �� ��� �� �Է�
                packet.win = howmanywin;    // ��Ŷ�� win �ʵ忡 Ŭ���̾�Ʈ�� �¸��� Ƚ�� �Է�

                int serverwin = count - howmanywin; // ���ºΰ� �����Ƿ� �� ��� ������ Ŭ���̾�Ʈ�� �¸��� Ƚ���� ���� ������ �¸��� Ƚ��

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // Ŭ���̾�Ʈ�� ��Ŷ ����
                std::cout << std::endl << "<<<<<<<<���� �·� ���� ���>>>>>>>>" << std::endl;   
                std::cout << std::endl << "�� ��� �� : " << count << " ��" << std::endl;    // �� ��� �� ���
                std::cout << std::endl << "������ �̱� �� ��� �� : " << serverwin << " ��" << std::endl; // ������ �¸��� Ƚ�� ���
                std::cout << std::endl << "������ ���� �·� : " << (1.0 - packet.winrate) * 100 << "%" << std::endl;   // ���� ���� �·� ���
                std::cout << std::endl << "Ŭ���̾�Ʈ�� �̱� �� ��� �� : " << howmanywin << " ��" << std::endl; // Ŭ���̾�Ʈ�� �¸��� Ƚ�� ���
                std::cout << std::endl << "Ŭ���̾�Ʈ�� ���� �·� : " << packet.winrate * 100 << "%" << std::endl;    // Ŭ���̾�Ʈ ���� �·� ���
                break;  // �ݺ��� Ż��
            }
            std::cout << std::endl << "�� ���� �����Ͻð� �·��� ��û���ּ���." << std::endl;
            break;  // �ݺ��� Ż��
        }
        else {
            continue;   // �ݺ����� ó������ ���ư� �ٽ� ���������� ���� ����
        }

        closesocket(clientSocket);  // ���� clientSocket �ݱ�
        closesocket(serverSocket);  // ���� serverSocket �ݱ�
        WSACleanup();   // Winsock ����

        return 0;   // ���� ���� ���
    }
    std::cout << std::endl << "���α׷��� �����ϰڽ��ϴ�. ������ �� �˰ڽ��ϴ�~" << std::endl;
    return 0;   // ���� ���� ���
}
