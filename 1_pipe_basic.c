/*
 * 1_pipe_basic.c — 파이프로 부모가 자식에게 문자열을 보낸다
 *
 * [핵심 개념]
 *   pipe(2) 는 파일 디스크립터 두 개를 준다: fd[0] 은 읽기, fd[1] 은 쓰기.
 *   fork 하면 자식이 이 디스크립터를 물려받으므로 부모-자식이 통신할 수 있다.
 *   각자 쓰지 않는 쪽은 반드시 닫아야 한다 — 쓰기 쪽이 모두 닫혀야 읽기 쪽이 EOF(0)를 본다.
 *   출처: man 2 pipe, man 7 pipe
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 1_pipe_basic 1_pipe_basic.c
 *   ./1_pipe_basic
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    int fd1[2], fd2[2];

    if (argc != 2) {
        printf("사용법: %s \"문자열\"\n", argv[0]);
        return 1;
    }

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(fd1[1]);
        close(fd2[0]);

        char buf[256];
        ssize_t n = read(fd1[0], buf, sizeof(buf) - 1);

        if (n > 0) {
            buf[n] = '\0';

            for (int i = 0; buf[i] != '\0'; i++)
                buf[i] = toupper((unsigned char)buf[i]);

            write(fd2[1], buf, strlen(buf));
        }

        close(fd1[0]);
        close(fd2[1]);
        _exit(0);
    }

    close(fd1[0]);
    close(fd2[1]);

    write(fd1[1], argv[1], strlen(argv[1]));
    close(fd1[1]);

    char result[256];
    ssize_t n = read(fd2[0], result, sizeof(result) - 1);

    if (n > 0) {
        result[n] = '\0';
        printf("[부모] 자식이 변환한 문자열: %s\n", result);
    }

    close(fd2[0]);
    wait(NULL);

    return 0;
}