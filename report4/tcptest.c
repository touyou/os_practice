/* $Id: echo-server-select.c,v 1.2 2005/06/11 20:25:10 68user Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_LEN  256  /* バッファのサイズ */

                                /* クライアントの情報を保持する構造体 */
typedef struct CLIENT_INFO {
  char hostname[BUF_LEN];       /* ホスト名 */
  char ipaddr[BUF_LEN];         /* IP アドレス */
  int port;                     /* ポート番号 */
  time_t last_access;           /* 最終アクセス時刻 */
} CLIENT_INFO;

CLIENT_INFO client_info[FD_SETSIZE];

int    listening_socket;
struct sockaddr_in sin;

/*-----------------------------------------------------
  引数でリスニングソケットを受け取り、accept し、
  client_info に新しいクライアントの情報を登録する。
  戻り値は新しいクライアントのソケットディスクリプタ。
  ただしエラー発生時は -1 を返す。
  -----------------------------------------------------*/
int
accept_new_client(int sock){
  int len;
  int new_socket;
  struct hostent *peer_host;
  struct sockaddr_in peer_sin;

  len = sizeof(sin);
  new_socket = accept(listening_socket, (struct sockaddr *)&sin, &len);

  if ( new_socket == -1 ){
    perror("accept");
    exit(1);
  }

  if ( new_socket > FD_SETSIZE-1 ){
    return -1;
  }
                                /* ここから先はデバッグ用の情報取得 */
  len = sizeof(peer_sin);
  getpeername(new_socket,
              (struct sockaddr *)&peer_sin, &len);

  peer_host = gethostbyaddr((char *)&peer_sin.sin_addr.s_addr,
                            sizeof(peer_sin.sin_addr), AF_INET);

                                /* ホスト名 */
  strncpy(client_info[new_socket].hostname, peer_host->h_name,
          sizeof client_info[new_socket].hostname);
                                /* IP アドレス */
  strncpy(client_info[new_socket].ipaddr, inet_ntoa(peer_sin.sin_addr),
          sizeof client_info[new_socket].ipaddr);
                                /* ポート番号 */
  client_info[new_socket].port = ntohs(peer_sin.sin_port);
                                /* 現在時刻を最終アクセス時刻として記録しておく */
  time(&client_info[new_socket].last_access);

  printf("接続: %s (%s) ポート %d  ディスクリプタ %d 番\n",
         client_info[new_socket].hostname,
         client_info[new_socket].ipaddr,
         client_info[new_socket].port,
         new_socket);
  return new_socket;
}


/*-----------------------------------------------------
  引数でソケットディスクリプタを受け取り、そのソケットから
  read(2) で文字列を読み込み、文字列をそのままクライアントに
  送信する。read(2) の戻り値をそのまま返す。
  -----------------------------------------------------*/
int
read_and_reply(int sock){
  int read_size;
  char buf[BUF_LEN];

  read_size = read(sock, buf, sizeof(buf)-1);

  if ( read_size == 0 || read_size == -1 ){
    printf("%s (%s) ポート %d  ディスクリプタ %d 番からの接続が切れました。\n",
           client_info[sock].hostname,
           client_info[sock].ipaddr,
           client_info[sock].port,
           sock);
    close(sock);
    client_info[sock].last_access = 0;
  } else {
                                /* 文字列終端を \0 で terminate */
    buf[read_size] = '\0';
    printf("%s (%s) ポート %d  ディスクリプタ %d 番からのメッセージ: %s",
           client_info[sock].hostname,
           client_info[sock].ipaddr,
           client_info[sock].port,
           sock,
           buf);
    write(sock, buf, strlen(buf));
    time(&client_info[sock].last_access);
  }
  return read_size;
}


int
main(){
  fd_set target_fds;
  fd_set org_target_fds;
  int sock_optval = 1;
  int port = 5000;
                                /* リスニングソケットを作成 */
  listening_socket = socket(AF_INET, SOCK_STREAM, 0);

                                /* ソケットオプション設定 */
  if ( setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
                  &sock_optval, sizeof(sock_optval)) == -1 ){
    perror("setsockopt");
    exit(1);
  }
                                /* アドレスファミリ・ポート番号・IPアドレス設定 */
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);

  if ( bind(listening_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0 ){
    perror("bind");
    exit(1);
  }

  if ( listen(listening_socket, SOMAXCONN) == -1 ){
    perror("listen");
    exit(1);
  }
  printf("ポート %d を見張ります。\n", port);

                                /* 監視対象のディスクリプタ一覧をゼロクリア */
  FD_ZERO(&org_target_fds);
                                /* リスニングソケットを監視対象に追加 */
  FD_SET(listening_socket, &org_target_fds);

  while (1){
    int i;
    time_t now_time;
    struct timeval waitval;     /* select に待ち時間を指定するための構造体 */
    waitval.tv_sec  = 2;        /* 待ち時間に 2.500 秒を指定 */
    waitval.tv_usec = 500;

                                /* org_target_fds を target_fds にコピー */
    memcpy(&target_fds, &org_target_fds, sizeof(org_target_fds));

    select(FD_SETSIZE, &target_fds, NULL, NULL, &waitval);

                                /* ソケットが読み出し可能か順にチェック */
    for ( i=0 ; i<FD_SETSIZE ; i++ ){
      if ( FD_ISSET(i, &target_fds) ){
        printf("ディスクリプタ %d 番が読み込み可能です。\n", i);

        if ( i == listening_socket ){
          int new_sock;
                                /* 新しいクライアントがやってきた */
          new_sock = accept_new_client(i);
          if ( new_sock != -1 ){
                                /* 監視対象に新たなソケットを追加 */
            FD_SET(new_sock, &org_target_fds);
          }
        } else {
          int read_size;
                                /* 接続済みソケットからデータが送信されてきた */
          read_size = read_and_reply(i);

          if ( read_size == -1 || read_size == 0 ){
                                /* 切断したソケットを監視対象から削除 */
            FD_CLR(i, &org_target_fds);
          }
        }
      }
    }

    time(&now_time);          /* 現在時刻を取得 */

    for ( i=0 ; i<FD_SETSIZE ; i++ ){
                                /* 監視対象でないソケットはスキップ */
      if ( ! FD_ISSET(i, &org_target_fds) )  continue;
                                /* リスニングソケットはスキップ */
      if ( i == listening_socket ) continue;

      if ( now_time-10 > client_info[i].last_access ){
        printf("%s (%s) ポート %d  ディスクリプタ %d 番から10秒以上アクセスがありません。切断します。\n",
               client_info[i].hostname,
               client_info[i].ipaddr,
               client_info[i].port,
               i);
        close(i);
                                /* 切断したソケットを監視対象から削除 */
        FD_CLR(i, &org_target_fds);
      }
    }
  }
  close(listening_socket);
  return 0;
}
