// protocol.h

#ifndef PROTOCOL_H
#define PROTOCOL_H

struct Packet {
    int start_game_request = 0;      // 게임 시작 요청 (0 : 게임 거절
    int Game_Choose = 0;       // 게임 수행 여부(0: RPS 해야함, 1: MJB 해야함)
    int choice_S;          // 서버 선택(묵: '0', 찌: '1', 빠: '2')
    int choice_C;          // 클라이언트 선택(묵: '0', 찌: '1', 빠: '2')
    int Att;          // 게임 결과 선공(-1: 서버 선공, 0: 무승부, 1: 클라이언트 선공)
    char result_str[50];    // 게임 결과 묵찌빠(문자열)
    int count;              // 게임 수
    int win;                // 클라이언트 승수
    double winrate;      // 승률 정보
    int end = 0;             // 게임 종료 요청
};

#endif // PROTOCOL_H
