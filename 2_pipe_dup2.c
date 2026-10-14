/*
 * 2_pipe_dup2.c — 셸의 "ls | wc -l" 을 직접 만들어 본다
 *
 * [핵심 개념]
 *   dup2(a, b) 는 디스크립터 b 가 a 와 같은 곳을 가리키게 만든다.
 *   자식의 표준출력(1)을 파이프의 쓰기 끝으로 바꿔치기하면
 *   그 프로그램은 아무것도 몰라도 출력이 파이프로 흘러간다. 셸이 하는 일이 바로 이것이다.
 *   출처: man 2 dup2, man 2 pipe
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 2_pipe_dup2 2_pipe_dup2.c
 *   ./2_pipe_dup2      # ls | wc -l 과 같은 결과가 나온다
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int sep = -1;

    /* ':' 위치 찾기 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ":") == 0) {
            sep = i;
            break;
        }
    }

    if (sep <= 1 || sep >= argc - 1) {
        fprintf(stderr, "사용법: %s 명령어 : 명령어\n", argv[0]);
        return 1;
    }

    argv[sep] = NULL;

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) {
        /* 첫 번째 명령어 */
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(127);
    }

    pid_t pid2 = fork();

    if (pid2 == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        return 1;
    }

    if (pid2 == 0) {
        /* 두 번째 명령어 */
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        execvp(argv[sep + 1], &argv[sep + 1]);
        perror("execvp");
        _exit(127);
    }

    /* 부모 프로세스 */
    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}