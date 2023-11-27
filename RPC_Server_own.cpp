#define _WINSOCK_DEPRECATED_NO_WARNINGS //  최신 VC++ 컴파일시 경고 방지
#include <iostream> //
#include <winsock2.h>   // 소켓을 사용하기 위한 헤더 파일, API에서 사용하는 상수, 데이터 유형 및 함수 프로토타입 정의
#include <WS2tcpip.h>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")  // 위에서 선언한 헤더파일들을 가져다 쓰기위한 링크, ws2_32.lib 라이브러리 파일을 프로그램에 링크하도록 컴파일러에 알리는 pragma 지시어

#define SERVERPORT 9000
#define BUFSIZE 512

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

    // 윈속 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
        return -1;

    // socket()
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // 소켓을 생성하는 함수 호출
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

    // 데이터 통신에 사용할 변수
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

    // 접속한 클라이언트 정보 출력
    printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    // 가위 바위 보 게임
    int retry = 1;
    int Att = 0, count = 0;
    int howmanywin = 0;

    while (retry == 1) {

        std::cout << "가위 바위 보 게임을 시작합니다." << std::endl;
        std::cout << "가위(0), 바위(1), 보(2) 중 하나를 선택하세요(승률 (3), 종료 (4)): ";

        int serverHand = rand() % 3;  // 0: 가위, 1: 바위, 2: 보
        int clientHand;
        recv(clientSocket, (char*)&clientHand, sizeof(clientHand), 0);

        std::cout << "상대방이 선택한 것: ";
        if (clientHand == 0) {
            std::cout << "가위";
        }
        else if (clientHand == 1) {
            std::cout << "바위";
        }
        else if (clientHand == 2) {
            std::cout << "보";
        }
        else if (clientHand == 3) {
            std::cout << "승률";
        }
        else {
            break;
        }
        std::cout << std::endl;

        // 게임 결과 전송
        if (clientHand == '4') {
            break;
        }
        else if (serverHand == clientHand) {
            Att = 0;  // 무승부
            retry = 1;
        }
        else if ((serverHand + 1) % 3 == clientHand) {
            Att = 1;  // 서버 승리
            retry = 0;
        }
        else {
            Att = -1;  // 클라이언트 승리
            retry = 0;
        }

        send(clientSocket, (char*)&Att, sizeof(Att), 0);
    }
    

    // 묵찌빠 게임
    // (묵: 0, 찌: 1, 빠: 2)
    while (true) {
        int serverChoice = rand() % 3;
        send(clientSocket, (char*)&serverChoice, sizeof(serverChoice), 0);

        char clientChoice;
        recv(clientSocket, &clientChoice, sizeof(clientChoice), 0);

        char result[50];

        if ((clientChoice != '3') && (clientChoice != '4')) {
            if (Att == 1) {
                if (serverChoice == (clientChoice - '0')) {
                    strcpy_s(result, "패배!");
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
                    strcpy_s(result, "승리!");
                    howmanywin++;
                }
                else if ((serverChoice + 1) % 3 == (clientChoice - '0')) {
                    strcpy_s(result, "공수 교대");
                    Att = 1;
                }
                else {
                    strcpy_s(result, "공수 유지");
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

    // 소켓 및 서버 소켓 닫기
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
