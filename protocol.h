// protocol.h

#ifndef PROTOCOL_H
#define PROTOCOL_H

struct Packet {
    int start_game_request = 0;      // ���� ���� ��û (0 : ���� ����
    int Game_Choose = 0;       // ���� ���� ����(0: RPS �ؾ���, 1: MJB �ؾ���)
    int choice_S;          // ���� ����(��: '0', ��: '1', ��: '2')
    int choice_C;          // Ŭ���̾�Ʈ ����(��: '0', ��: '1', ��: '2')
    int Att;          // ���� ��� ����(-1: ���� ����, 0: ���º�, 1: Ŭ���̾�Ʈ ����)
    char result_str[50];    // ���� ��� �����(���ڿ�)
    int count;              // ���� ��
    int win;                // Ŭ���̾�Ʈ �¼�
    double winrate;      // �·� ����
    int end = 0;             // ���� ���� ��û
};

#endif // PROTOCOL_H
