#define _CRT_SECURE_NO_WARNINGS //strcpy 함수의 크기 설정 오류 출력을 없애기 위해
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#include <iostream> // 입출력을 사용하기 위함
#include <winsock2.h>   // 소켓을 사용하기 위한 헤더 파일, API에서 사용하는 상수, 데이터 유형 및 함수 프로토타입 정의
#include <WS2tcpip.h>
#include <ctime>
#include "protocol.h"   // 프로토콜 헤더 파일

#define ROCK 0              
#define SCISSORS 1
#define PAPER 2
#define WIN_REQUEST 3
#define END_REQUEST 4   // 프로그램에서 자주 사용되거나 특별한 트리거를 요구하는 것을 먼저 정의

#pragma comment(lib, "ws2_32.lib")  // 위에서 선언한 헤더파일들을 가져다 쓰기위한 링크, ws2_32.lib 라이브러리 파일을 프로그램에 링크하도록 컴파일러에 알리는 pragma 지시어

#define SERVERPORT 9000 // 통신에 사용되는 포트를 정의
#define BUFSIZE sizeof(Packet)  // 통신에 사용되는 버퍼의 사이즈를 Packet의 사이즈만큼으로 정의ㅣ

// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;

    // WSAGetLastError 함수를 사용하여 최근에 발생한 소켓 함수의 오류 코드를 얻음
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    // MessageBox를 사용하여 오류 메세지를 팝업 창에 표시
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    // 할당된 메모리를 해제
    LocalFree(lpMsgBuf);
    // 프로그램을 비정상적으로 종료
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    // err_quit 함수와 유사하게 동작하지만, MessageBox 대신 printf를 사용하여 콘솔에 오류 메세지 출력
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    // 오류 메세지 출력
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    // 할당된 메모리를 해제
    LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[]) {

    int retval;
    WSADATA wsaData;        // Winsock 초기화

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  // Winsock 버전 2.2를 호출하고 이를 설정한다, 이 값이 올바르지 않다면
        return -1;  // 오류가 있다는 것을 반환

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // 소켓 serverSocket을 IPv4 주소 체계, TCP 프로토콜, SOCK_STREAM 소켓 타입을 사용하여 생성
    if (serverSocket == INVALID_SOCKET) {   // 서버 소켓이 잘못된 소켓이라면
        err_quit("socket()");   // "socket()"이라는 메세지를 출력 후 프로그램 종료
        WSACleanup();   // Winsock 종료
        return -1;  // 오류가 발생했다는 것을 반환
    }

    SOCKADDR_IN serverAddress;  // 소켓 주소 serverAddress 선언
    ZeroMemory(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; // IPv4 주소 체계
    serverAddress.sin_addr.s_addr = INADDR_ANY; // 주소는 
    serverAddress.sin_port = htons(SERVERPORT); // 9000번 포트
    retval = bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));  // 서버측의 소켓에 IP와 포트를 할당하여 네트워크 인트페이스와 묶일 수 있게 함.
    if (retval == SOCKET_ERROR) {   // 
        err_quit("bind()"); // "bind()"이라는 메세지를 출력 후 프로그램 종료
        WSACleanup();   // Winsock 종료
        return -1;  // 오류가 발생했다는 것을 반환
    }

    retval = listen(serverSocket, 5);   // 클라이언트로부터 연결을 기다리고, 동시에 연결하는 최대 클라이언트 수를 5으로 설정한다
    if (retval == SOCKET_ERROR) {   // 소켓 에러가 발생하면
        err_quit("listen()");   // "listen()"이라는 메세지를 출력 후 프로그램 종료
        WSACleanup();   // Winsock 종류
        return -1;  // 오류가 발생했다는 것을 반환
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;    // 서버가 클라이언트와의 연결을 기다리고 있다는 해당 문장 출력

    SOCKET clientSocket;        // clientSocket이라는 소켓을 생성
    SOCKADDR_IN clientaddr;     // 소켓 주소 구조체 형식의 clientaddr 생성
    int addrlen;    // 주소 길이를 나타내는 addrlen 변수 선언
    Packet packet;  // Packet 구조체 형식의 packet 선언

    addrlen = sizeof(clientaddr);   // clientaddr 사이즈를 addrlen에 대입
    clientSocket = accept(serverSocket, (SOCKADDR*)&clientaddr, &addrlen);  // 서버측에서 클라이언트의 연결 요청을 허가한다.
    if (clientSocket == INVALID_SOCKET) {   // clientSocket이 올바른 소켓이 아니라면
        err_display("accept()");    // "accept()"이라는 메세지를 출력 후 프로그램 종료
        WSACleanup();   // Winsock 종료
        return -1;  // 오류가 발생했다는 것을 반환
    }

    printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));    // 통신 시작시 서버에 연결된 클라이언트의 IP 주소와 포트 주소를 출력

    int retry = 0;      // 가위바위보 게임과 묵찌빠 게임 중 무슨 게임을 진행할지 결정하는 변수
    int Att = 0, count = 0; // Att는 공격 기회가 누구에게 있는지 결정하는 변수(1이면 서버 공격, -1이면 클라이언트 공격), count 변수는 게임의 총 경기수를 나타내는 변수 
    int howmanywin = 0; // 클라이언트가 총 몇 판 승리하였는지 나타내는 함수

    std::cout << "묵찌빠 게임을 진행하시겠습니까? " << "( 네(1) / 아니오(0) ) : ";
    recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 클라이언트에서 전송한 패킷을 수신
    
    std::cout << packet.start_game_request << std::endl;    // 패킷의 start_game_request 필드의 값을 화면에 출력

    while (packet.start_game_request) {     // 패킷의 start_game_request 필드값이 1이면 반복
        srand(time(NULL));      // 서버의 랜덤 선택을 무작위로 하기위한 시간을 시드값으로 설정

        while (packet.Game_Choose == 0) {   // 패킷의 Game_Choose 필드값이 0이면 가위바위보 게임 진행
            
            std::cout << std::endl << "선공을 위한 가위 바위 보 게임을 시작합니다!!" << std::endl;
            std::cout << "바위(0), 가위(1), 보(2) 중 하나를 선택하세요(승률(3), 종료(4)): ";

            int serverHand = rand() % 3;    // 서버는 0,1,2 3개 중 무작위로 선택
            packet.choice_S = serverHand;   // 서버의 선택값을 패킷의 choice_S 필드에 대입

            send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 패킷을 클라이언트로 전송
            recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 클라이언트가 전송한 패킷을 수신

            std::cout << serverHand << std::endl;   // 서버가 랜덤으로 선택한 값을 화면에 출력

            if (packet.choice_C == END_REQUEST) {   // 패킷의 choice_C 필드의 값이 4라면
                packet.end = 1;     // 패킷의 end 필드 값을 1로 변경
                break;      // 반복문을 빠져 나옴
            }
            else if (packet.choice_C == WIN_REQUEST) {  // 패킷의 choice_C 필드값이 3이라면
                if (count > 0) {        // 게임을 한 판 이상 진행했을 시
                    packet.winrate = (double)howmanywin / count;    // 패킷의 winrate 필드에 클라이언트의 승률을 대입
                    packet.count = count;   // 패킷의 count 필드에 진행한 게임 수를 대입
                    packet.win = howmanywin;    // 패킷의 win 필드에 클라이언트가 승리한 게임 수를 입력

                    int serverwin = count - howmanywin; // 무승부가 없으므로 서버가 이긴 횟수는 총 게임 수에서 클라이언트가 승리한 횟수를 뺀 값

                    send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 패킷을 클라이언트로 전송
                    std::cout << std::endl << "<<<<승률 정보 출력>>>>" << std::endl;  
                    std::cout << std::endl << "경기 수 : " << count << " 판" << std::endl;  // 경기 수 출력
                    std::cout << std::endl << "서버가 이긴 경기 수 : " << serverwin << " 판" << std::endl;   // 서버가 승리한 횟수 출력
                    std::cout << std::endl << "서버의 승률 : " << (1.0 - packet.winrate) * 100 << "%" << std::endl;  // 서버의 승률 출력
                    std::cout << std::endl << "클라이언트가 이긴 경기 수 : " << howmanywin << " 판" << std::endl;   // 클라이언트가 승리한 횟수 출력
                    std::cout << std::endl << "클라이언트의 승률 : " << packet.winrate * 100 << "%" << std::endl;   // 클라이언트의 승률 출력
                    continue; // 반복문의 처음으로 돌아가서 게임을 다시 진행
                }
                std::cout << std::endl << "아직 한 판도 안하셨습니다." << std::endl;   // count가 0이면 해당 문장 출력
                continue;   // 반복문의 처음으로 돌아가서 게임을 다시 진행
            }

            std::cout << "서버가 선택한 것: ";
            if (serverHand == ROCK) {   // 서버가 0을 선택했다면
                std::cout << "바위      ";
            }
            else if (serverHand == SCISSORS) {  // 서버가 1을 선택했다면
                std::cout << "가위      ";
            }
            else if (serverHand == PAPER) { // 서버가 2를 선택했다면
                std::cout << "보      ";
            }
            std::cout << "클라이언트가 선택한 것: ";
            if (packet.choice_C == ROCK) {  // 클라이언트가 0을 선택했다면(choice_C 필드의 값이 0이라면)
                std::cout << "바위";
            }
            else if (packet.choice_C == SCISSORS) { // 클라이언트가 1을 선택했다면(choice_C 필드값이 1이라면)
                std::cout << "가위";
            }
            else if (packet.choice_C == PAPER) {    // 클라이언트가 2를 선택했다면(choice_C 필드의 값이 2라면
                std::cout << "보";
            }
            std::cout << std::endl;

            if (serverHand == packet.choice_C) {    // 서버가 선택한 값과 클라이언트가 선택한 값(choice_C 필드의 값)이 같다면
                Att = 0;    // 공격 기회는 아무에게도 없고
                std::cout << "무승부입니다!! 재경기를 진행하세요!" << std::endl;
                retry = 0;  // 가위바위보 게임을 다시 진행
            }
            else if ((serverHand + 1) % 3 == packet.choice_C) { // 서버가 클라이언트와 가위바위보 게임에서 승리하였다면
                Att = 1;    // 공격 기회는 서버에게 있고
                std::cout << "서버가 공격입니다! 기회를 놓치지 마세요!!" << std::endl;
                retry = 1;  // 묵찌빠 게임을 진행
            }
            else if ((serverHand + 2) % 3 == packet.choice_C) { // 서버가 클라이언트와 가위바위보 게임에서 패배하였다면
                Att = -1;   // 공격 기회는 클라이언트에게 있고
                std::cout << "아쉽군요! 클라이언트가 공격입니다! " << std::endl;
                retry = 1;  // 묵찌빠 게임을 진행
            }

            packet.Att = Att;   // 공격 기회가 누구에게 있는지 값을 가지고 있는 Att 변수의 값을 패킷의 Att 필드에 입력
            packet.Game_Choose = retry; // 가위바위보 게임과 묵찌빠 게임 중 무엇을 진행할 것인지 값을 가지고 있는 retry 변수의 값을 패킷의 Game_Choose 필드에 대입
            send(clientSocket, (char*)&packet, sizeof(Packet), 0); // 패킷을 클라이언트로 전송
        }

        while (packet.Game_Choose) {    // 패킷의 Game_Choose 값이 1이면 묵찌빠 게임을 진행

            std::cout << std::endl << "묵찌빠 게임을 시작합니다!!" << std::endl;
            std::cout << "바위(0), 가위(1), 보(2) 중 하나를 선택하세요(승률(3), 종료(4)): ";

            recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 클라이언트가 전송한 패킷을 수신
            
            int serverChoice = rand() % 3;  // 서버는 0,1,2 중 무작위 값 선택
            packet.choice_S = serverChoice; // 서버의 선택을 choice_S 필드에 입력
            
            std::cout << serverChoice << std::endl; // 서버의 선택을 화면에 출력

            if (packet.choice_C == END_REQUEST) {   // 클라이언트의 선택이 4라면
                packet.end = 1;     // end 필드에 1을 대입
                break;  // 반복문을 탈출
            }
            else if (packet.choice_C == WIN_REQUEST) {  // 클라이언트의 선택이 3이라면
                if (count > 0) {    // 게임을 한 판이상 진행했다면
                    packet.winrate = (double)howmanywin / count;    // 패킷의 winrate 필드에 클라이언트의 승률을 입력
                    packet.count = count;   // count 필드에 진행한 경기 수를 입력
                    packet.win = howmanywin;    // win 필드에 클라이언트가 승리한 경기 수를 입력

                    int serverwin = count - howmanywin; // 무승부는 없으므로 총 경기 수에서 클라이언트가 진행한 경기 수를 빼면 서버가 승리한 횟수

                    send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 클라이언트로 패킷 전송
                    std::cout << std::endl << "<<<<승률 정보 출력>>>>" << std::endl;  
                    std::cout << std::endl << "경기 수 : " << count << " 판" << std::endl;  // 총 경기 수 출력
                    std::cout << std::endl << "서버가 이긴 경기 수 : " << serverwin << " 판" << std::endl;   // 서버가 승리한 횟수 출력
                    std::cout << std::endl << "서버의 승률 : " << (1.0 - packet.winrate) * 100 << "%" << std::endl;  // 서버의 승률 출력
                    std::cout << std::endl << "클라이언트가 이긴 경기 수 : " << howmanywin << " 판" << std::endl;   // 클라이언트가 승리한 횟수 출력
                    std::cout << std::endl << "클라이언트의 승률 : " << packet.winrate * 100 << "%" << std::endl;   // 클라이언트의 승률 출력
                    continue;   // 반복문의 처음으로 돌아가 다시 묵찌빠 게임 진행
                }
                std::cout << std::endl << "한 판은 진행하시고 승률을 요청해주세요." << std::endl;
                continue;   // 반복문의 처음으로 돌아가 다시 묵찌빠 게임 진행
            }
            else {         // 제대로 된 묵찌빠 게임 시작   

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 클라이언트로 패킷 전송
                recv(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 클라이언트가 전송한 패킷 수신

                std::cout << "서버가 선택한 것: ";
                if (serverChoice == ROCK) { // 서버가 0을 선택했다면
                    std::cout << "바위      ";
                }
                else if (serverChoice == SCISSORS) {    // 서버가 1을 선택했다면
                    std::cout << "가위      ";
                }
                else if (serverChoice == PAPER) {   // 서버가 2를 선택했다면
                    std::cout << "보      ";
                }
                std::cout << "클라이언트가 선택한 것: ";
                if (packet.choice_C == ROCK) {  // 클라이언트가 0을 선택했다면
                    std::cout << "바위";
                }
                else if (packet.choice_C == SCISSORS) { // 클라이언트가 1을 선택했다면
                    std::cout << "가위";
                }
                else if (packet.choice_C == PAPER) {    // 클라이언트가 2를 선택했다면
                    std::cout << "보";
                }
                std::cout << std::endl;

                char result[50];    // 결과 메세지를 전송하기 위해 문자열 result 선언
                if (packet.Att == 1) {  // 공격 기회가 서버에게 있었었다면
                    if (serverChoice == packet.choice_C) {  // 서버와 클라이언트가 같은 값을 선택했을 때
                        strcpy(result, "서버 승리! 클라이언트 패배!"); // result 변수에 해당 문장 대입
                        packet.Game_Choose = 0; // 다시 가위바위보 게임을 돌아가기 위해 Game_Choose 필드값을 0으로 변경
                        count++;    // 진행한 경기 수 1 증가
                    }
                    else if ((serverChoice + 1) % 3 == packet.choice_C) {   // 서버가 여전히 공격 기회를 가지고 있다면
                        strcpy_s(result, "공수 유지, 서버 공격중");  // result 변수에 해당 문장 대입

                    }
                    else if ((serverChoice + 2) % 3 == packet.choice_C) {   // 클라이언트가 공격 기회를 가져왔다면
                        strcpy(result, "공수 교대, 클라이언트 공격");  // result 변수에 해당 문장 대입
                        packet.Att = -1;    // 공격 기회가 클라이언트에게 넘어갔다는 것을 알리기 위해 패킷의 Att 필드에 -1을 입력
                    }
                }
                else if (packet.Att == -1) {    // 공격 기회가 클라이언트에게 있다면
                    if (serverChoice == packet.choice_C) {  // 서버와 클라이언트가 같은 선택을 했다면
                        strcpy(result, "서버 패배! 클라이언트 승리!");  // result 변수에 해당 문장 대입
                        packet.Game_Choose = 0; // 가위바위보 게임을 다시 진행하기 위해 패킷의 Game_Choose 필드 값을 0으로 변경
                        howmanywin++;   // 클라이언트가 승리한 횟수 1 증가
                        count++;    // 총 경기 수 1 증가
                    }
                    else if ((serverChoice + 1) % 3 == packet.choice_C) {   // 서버가 공격 기회를 가져왔다면
                        strcpy(result, "공수 교대, 서버 공격");  // result 변수에 해당 문장 대입
                        packet.Att = 1; // 공격 기회가 서버에게 넘어갔다는 것을 알리기 위해 패킷의 Att 필드에 1을 입력
                    }
                    else if ((serverChoice + 2) % 3 == packet.choice_C) {   // 클라이언트가 여전히 공격 기회를 가지고있다면
                        strcpy(result, "공수 유지, 클라이언트 공격중");  // result 변수에 해당 문장 대입
                    }
                }
                std::cout << std::endl << "결과: " << result << std::endl;    // result 변수에 입력된 문장 출력
                strcpy((char*)& packet.result_str, result); // 패킷의 result_str 필드에 result 변수 값을 입력

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 패킷을 클라이언트로 전송
            }
        }
        if (packet.end == 1) {  // 패킷의 end 필드 값이 1이면
            if (count > 0) {    // 게임을 한 판 이상 진행했다면
                packet.winrate = (double)howmanywin / count;    // 패킷의 winrate 필드에 클라이언트의 승률 입력
                packet.count = count;   // 패킷의 count 필드에 총 경기 수 입력
                packet.win = howmanywin;    // 패킷의 win 필드에 클라이언트가 승리한 횟수 입력

                int serverwin = count - howmanywin; // 무승부가 없으므로 총 경기 수에서 클라이언트가 승리한 횟수를 빼면 서버가 승리한 횟수

                send(clientSocket, (char*)&packet, sizeof(Packet), 0);  // 클라이언트로 패킷 전송
                std::cout << std::endl << "<<<<<<<<최종 승률 정보 출력>>>>>>>>" << std::endl;   
                std::cout << std::endl << "총 경기 수 : " << count << " 판" << std::endl;    // 총 경기 수 출력
                std::cout << std::endl << "서버가 이긴 총 경기 수 : " << serverwin << " 판" << std::endl; // 서버가 승리한 횟수 출력
                std::cout << std::endl << "서버의 최종 승률 : " << (1.0 - packet.winrate) * 100 << "%" << std::endl;   // 서버 최종 승률 출력
                std::cout << std::endl << "클라이언트가 이긴 총 경기 수 : " << howmanywin << " 판" << std::endl; // 클라이언트가 승리한 횟수 출력
                std::cout << std::endl << "클라이언트의 최종 승률 : " << packet.winrate * 100 << "%" << std::endl;    // 클라이언트 최종 승률 출력
                break;  // 반복문 탈출
            }
            std::cout << std::endl << "한 판은 진행하시고 승률을 요청해주세요." << std::endl;
            break;  // 반복문 탈출
        }
        else {
            continue;   // 반복문의 처음으로 돌아가 다시 가위바위보 게임 진행
        }

        closesocket(clientSocket);  // 소켓 clientSocket 닫기
        closesocket(serverSocket);  // 소켓 serverSocket 닫기
        WSACleanup();   // Winsock 종료

        return 0;   // 정상 종료 출력
    }
    std::cout << std::endl << "프로그램을 종료하겠습니다. 다음에 또 뵙겠습니다~" << std::endl;
    return 0;   // 정상 종룔 출력
}
