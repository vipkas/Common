#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAXLINE  511
#define MAX_SOCK 1024 // �ֶ󸮽��� ��� 64

char *EXIT_STRING = "exit";     // Ŭ���̾�Ʈ�� �����û ���ڿ�
char *START_STRING = "Connected to chat_server \n";
// Ŭ���̾�Ʈ ȯ�� �޽���
int maxfdp1;                            // �ִ� ���Ϲ�ȣ +1
int num_user = 0;                       // ä�� ������ ��
int num_chat = 0;                       // ���ݱ��� ���� ��ȭ�� ��
int clisock_list[MAX_SOCK];             // ä�ÿ� ������ ���Ϲ�ȣ ���
char ip_list[MAX_SOCK][20];             //������ ip���
int listen_sock;                        // ������ ���� ����

                                                        // ���ο� ä�� ������ ó��
void addClient(int s, struct sockaddr_in *newcliaddr);
int getmax();                           // �ִ� ���� ��ȣ ã��
void removeClient(int s);       // ä�� Ż�� ó�� �Լ�
int tcp_listen(int host, int port, int backlog); // ���� ���� �� listen
void errquit(char *mesg) { perror(mesg); exit(1); }

time_t ct;
struct tm tm;

void *thread_function(void *arg) { //��ɾ ó���� ������
        int i;
        printf("��ɾ� ��� : help, num_user, num_chat, ip_list\n");
        while (1) {
                char bufmsg[MAXLINE + 1];
                fprintf(stderr, "\033[1;32m"); //���ڻ��� ������� ����
                printf("server>"); //Ŀ�� ���
                fgets(bufmsg, MAXLINE, stdin); //��ɾ� �Է�
                if (!strcmp(bufmsg, "\n")) continue;   //���� ����
                else if (!strcmp(bufmsg, "help\n"))    //��ɾ� ó��
                        printf("help, num_user, num_chat, ip_list\n");
                else if (!strcmp(bufmsg, "num_user\n"))//��ɾ� ó��
                        printf("���� ������ �� = %d\n", num_user);
                else if (!strcmp(bufmsg, "num_chat\n"))//��ɾ� ó��
                        printf("���ݱ��� ���� ��ȭ�� �� = %d\n", num_chat);
                else if (!strcmp(bufmsg, "ip_list\n")) //��ɾ� ó��
                        for (i = 0; i < num_user; i++)
                                printf("%s\n", ip_list[i]);
                else //���� ó��
                        printf("�ش� ��ɾ �����ϴ�.help�� �����ϼ���.\n");
        }
}

int main(int argc, char *argv[]) {
        char destFolder[] = "";
        char strFolderPath[] = "./data";

        struct sockaddr_in cliaddr;
        char buf[MAXLINE + 1]; //Ŭ���̾�Ʈ���� ���� �޽���
        int i, j, nbyte, accp_sock, addrlen = sizeof(struct
                sockaddr_in);
        fd_set read_fds;        //�б⸦ ������ fd_set ����ü
        pthread_t a_thread;

        if (argc != 2) {
                printf("How to use this server program :%s port\n", argv[0]);
                exit(0);
        }

        if(mkdir(strFolderPath, 0776) == -1 && errno != EEXIST)
        {
                printf("Failed to make a folder\n");
        }

        // tcp_listen(host, port, backlog) �Լ� ȣ��
        listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);
        //������ ����
        pthread_create(&a_thread, NULL, thread_function, (void *)NULL);
        while (1) {
                FD_ZERO(&read_fds);
                FD_SET(listen_sock, &read_fds);
                for (i = 0; i < num_user; i++)
                        FD_SET(clisock_list[i], &read_fds);

                maxfdp1 = getmax() + 1; // maxfdp1 �� ���
                if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
                        errquit("select fail");

                if (FD_ISSET(listen_sock, &read_fds)) {
                        accp_sock = accept(listen_sock,
                                (struct sockaddr*)&cliaddr, &addrlen);
                        if (accp_sock == -1) errquit("accept fail");
                        addClient(accp_sock, &cliaddr);
                        send(accp_sock, START_STRING, strlen(START_STRING), 0);
                        ct = time(NULL);                        //���� �ð��� �޾ƿ�
                        tm = *localtime(&ct);
                        write(1, "\033[0G", 4);         //Ŀ���� X��ǥ�� 0���� �̵�
                        printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
                        fprintf(stderr, "\033[33m");//���ڻ��� ��������� ����
                        printf("Add user.  Current user number = %d\n", num_user);
                        fprintf(stderr, "\033[32m");//���ڻ��� ������� ����
                        fprintf(stderr, "server>"); //Ŀ�� ���
                }

                // Ŭ���̾�Ʈ�� ���� �޽����� ��� Ŭ���̾�Ʈ���� ���
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                char fileName[20];
                sprintf(fileName, "%d-%d-%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                strcpy(destFolder, "");
                strcat(destFolder, strFolderPath);
                strcat(destFolder, "/");
                strcat(destFolder, fileName);
                printf("%s\n", destFolder);
                if(mkdir(destFolder, 0776) == -1 && errno != EEXIST)
                {
                        printf("Failed to make a folder\n");
                }

                for (i = 0; i < num_user; i++) {
                        if (FD_ISSET(clisock_list[i], &read_fds)) {
                                num_chat++;                             //�� ��ȭ �� ����
                                nbyte = recv(clisock_list[i], buf, MAXLINE, 0);
                                if (nbyte <= 0) {
                                        removeClient(i);        // Ŭ���̾�Ʈ�� ����
                                        continue;
                                }
                                buf[nbyte] = 0;
                                // ���Ṯ�� ó��
                                if (strstr(buf, EXIT_STRING) != NULL) {
                                        removeClient(i);        // Ŭ���̾�Ʈ�� ����
                                        continue;
                                }
                                printf("%s\n", buf);
                                char * ptr = strtok(buf, " ");
                                char txt_path[30];
                                sprintf(txt_path, "%s/%d-%d-%d/%s.txt", strFolderPath, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, ptr);
                                printf("For debugging %s\n", txt_path);
                                FILE *fp = fopen(txt_path, "a");
                                ptr = strtok(NULL, " ");
                                fprintf(fp, "%s\n", ptr);
                                fclose(fp);
                                // ��� ä�� �����ڿ��� �޽��� ���
                                //for (j = 0; j < num_user; j++)
                                //      send(clisock_list[j], buf, nbyte, 0);
                        }
                }

        }  // end of while

        return 0;
}

// ���ο� ä�� ������ ó��
void addClient(int s, struct sockaddr_in *newcliaddr) {
        char buf[20];
        inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
        write(1, "\033[0G", 4);         //Ŀ���� X��ǥ�� 0���� �̵�
        fprintf(stderr, "\033[33m");    //���ڻ��� ��������� ����
        printf("new client: %s\n", buf);//ip���
        // ä�� Ŭ���̾�Ʈ ��Ͽ� �߰�
        clisock_list[num_user] = s;
        strcpy(ip_list[num_user], buf);
        num_user++; //���� �� ����
}

// ä�� Ż�� ó��
void removeClient(int s) {
        close(clisock_list[s]);
        if (s != num_user - 1) { //����� ����Ʈ ��迭
                clisock_list[s] = clisock_list[num_user - 1];
                strcpy(ip_list[s], ip_list[num_user - 1]);
        }
        num_user--; //���� �� ����
        ct = time(NULL);                        //���� �ð��� �޾ƿ�
        tm = *localtime(&ct);
        write(1, "\033[0G", 4);         //Ŀ���� X��ǥ�� 0���� �̵�
        fprintf(stderr, "\033[33m");//���ڻ��� ��������� ����
        printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
        printf("ä�� ������ 1�� Ż��. ���� ������ �� = %d\n", num_user);
        fprintf(stderr, "\033[32m");//���ڻ��� ������� ����
        fprintf(stderr, "server>"); //Ŀ�� ���
}

// �ִ� ���Ϲ�ȣ ã��
int getmax() {
        // Minimum ���Ϲ�ȣ�� ���� ���� ������ listen_sock
        int max = listen_sock;
        int i;
        for (i = 0; i < num_user; i++)
                if (clisock_list[i] > max)
                        max = clisock_list[i];
        return max;
}

// listen ���� ���� �� listen
int  tcp_listen(int host, int port, int backlog) {
        int sd;
        struct sockaddr_in servaddr;

        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd == -1) {
                perror("socket fail");
                exit(1);
        }
        // servaddr ����ü�� ���� ����
        bzero((char *)&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(host);
        servaddr.sin_port = htons(port);
        if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("bind fail");  exit(1);
        }
        // Ŭ���̾�Ʈ�κ��� �����û�� ��ٸ�
        listen(sd, backlog);
        return sd;
}