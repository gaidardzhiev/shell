#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>

#define MAXARGS 128
#define BUFSZ 4096
#define PROMPT_COLOR "\x1b[32m"
#define RESET_COLOR "\x1b[0m"

volatile sig_atomic_t int_flag = 0;

void sigint_hdlr(int sig) {
    (void)sig;
    int_flag = 1;
    write(STDOUT_FILENO, "\n", 1);
}

char **tkz(const char *ln, int *cnt) {
    char **toks = malloc(MAXARGS * sizeof(char *));
    int i = 0, n = (int)strlen(ln);
    int pos = 0;
    while (pos < n) {
        while (pos < n && isspace((unsigned char)ln[pos])) pos++;
        if (pos >= n) break;
        char *tk = malloc(BUFSZ);
        int tl = 0;
        if (ln[pos] == '\'' || ln[pos] == '"') {
            char q = ln[pos++];
            while (pos < n && ln[pos] != q) {
                if (ln[pos] == '\\' && q == '"' && pos + 1 < n) {
                    pos++;
                    tk[tl++] = ln[pos++];
                } else {
                    tk[tl++] = ln[pos++];
                }
            }
            if (pos < n && ln[pos] == q) pos++;
        } else {
            while (pos < n && !isspace((unsigned char)ln[pos]) &&
                   ln[pos] != '>' && ln[pos] != '<' && ln[pos] != ';' &&
                   ln[pos] != '|' && ln[pos] != '&' && ln[pos] != ']') {
                if (ln[pos] == '\\' && pos + 1 < n) {
                    pos++;
                    tk[tl++] = ln[pos++];
                } else {
                    tk[tl++] = ln[pos++];
                }
            }
        }
        tk[tl] = '\0';
        toks[i++] = tk;
        if (pos < n) {
            if (ln[pos] == '>' || ln[pos] == '<' || ln[pos] == ';' ||
                ln[pos] == '|' || ln[pos] == '&' || ln[pos] == ']') {
                if ((ln[pos] == '>' && pos + 1 < n && ln[pos + 1] == '>') ||
                    (ln[pos] == '&' && pos + 1 < n && ln[pos + 1] == '&') ||
                    (ln[pos] == '|' && pos + 1 < n && ln[pos + 1] == '|')) {
                    char *op = malloc(3);
                    op[0] = ln[pos];
                    op[1] = ln[pos + 1];
                    op[2] = '\0';
                    toks[i++] = op;
                    pos += 2;
                } else {
                    char *op = malloc(2);
                    op[0] = ln[pos];
                    op[1] = '\0';
                    toks[i++] = op;
                    pos++;
                }
            }
        }
        if (i >= MAXARGS -1) break;
    }
    toks[i] = NULL;
    *cnt = i;
    return toks;
}

void free_toks(char **toks, int cnt) {
    for (int i = 0; i < cnt; i++) free(toks[i]);
    free(toks);
}

char *exp_vars(const char *tk) {
    size_t len = strlen(tk);
    char *res = malloc(BUFSZ);
    if (!res) return NULL;
    size_t rpos = 0;
    for (size_t i = 0; i < len; i++) {
        if (tk[i] == '$' && (i +1) < len && (isalnum((unsigned char)tk[i +1]) || tk[i +1]== '_')) {
            size_t j = i +1;
            while(j < len && (isalnum((unsigned char)tk[j]) || tk[j]== '_'))
                j++;
            size_t vlen = j - (i +1);
            char var[128] = {0};
            strncpy(var, tk + i +1, vlen);
            char *val = getenv(var);
            if(val) {
                size_t vallen = strlen(val);
                if (rpos + vallen < BUFSZ -1) {
                    strcpy(res + rpos, val);
                    rpos += vallen;
                }
            }
            i = j -1;
        } else {
            if (rpos < BUFSZ -1)
                res[rpos++] = tk[i];
        }
    }
    res[rpos] = '\0';
    return res;
}

int bltin(char **a, int c) {
    if(c == 0) return 0;
    if(strcmp(a[0], "cd") == 0) {
        char *d = (c > 1) ? a[1] : getenv("HOME");
        if(!d) fprintf(stderr, "cd: HOME not set\n");
        else if(chdir(d) != 0) perror("cd");
        return 1;
    }
    if(strcmp(a[0], "exit") == 0) exit(0);
    if(strcmp(a[0], "export") == 0) {
        if (c <2) {
            fprintf(stderr, "export: usage: export VAR=VAL\n");
            return 1;
        }
        char *ar = a[1];
        char *eq = strchr(ar, '=');
        if(!eq) {
            fprintf(stderr, "export: invalid format\n");
            return 1;
        }
        *eq = '\0';
        if(setenv(ar, eq +1, 1) != 0) perror("export");
        return 1;
    }
    return 0;
}

int redir_parse(char **a, int *c, int *infd, int *outfd, int *app) {
    *infd = STDIN_FILENO;
    *outfd = STDOUT_FILENO;
    *app = 0;
    int wpos = 0;
    for(int i = 0; i < *c; i++) {
        if(strcmp(a[i], "<") == 0) {
            if(i +1 >= *c) {fprintf(stderr, "syntax error near unexpected token `newline'\n"); return -1;}
            int fd = open(a[i +1], O_RDONLY);
            if(fd < 0) {perror(a[i +1]); return -1;}
            if(*infd != STDIN_FILENO) close(*infd);
            *infd = fd;
            i++;
        } else if(strcmp(a[i], ">") == 0 || strcmp(a[i], ">>")==0) {
            if(i +1 >= *c) {fprintf(stderr, "syntax error near unexpected token `newline'\n"); return -1;}
            int flags = O_WRONLY | O_CREAT;
            if(strcmp(a[i], ">>") == 0) {
                flags |= O_APPEND;
                *app = 1;
            } else {
                flags |= O_TRUNC;
                *app = 0;
            }
            int fd = open(a[i +1], flags, 0644);
            if(fd < 0) {perror(a[i +1]); return -1;}
            if(*outfd != STDOUT_FILENO) close(*outfd);
            *outfd = fd;
            i++;
        } else {
            a[wpos++] = a[i];
        }
    }
    a[wpos] = NULL;
    *c = wpos;
    return 0;
}

int eval_cond(int c, char **a) {
    if(c < 3 || strcmp(a[0], "[") != 0 || strcmp(a[c-1], "]") != 0) {
        fprintf(stderr, "Invalid condition syntax\n");
        return 1;
    }
    int ac = c - 2;
    char **av = a + 1;
    if(ac == 1) return (strlen(av[0]) > 0) ? 0 : 1;
    if(ac == 2) {
        struct stat st;
        if(strcmp(av[0], "-n") == 0) return (strlen(av[1]) > 0) ? 0 : 1;
        if(strcmp(av[0], "-z") == 0) return (strlen(av[1]) == 0) ? 0 : 1;
        if(stat(av[1], &st) == 0) {
            if(strcmp(av[0], "-e") == 0) return 0;
            if(strcmp(av[0], "-f") == 0) return S_ISREG(st.st_mode) ? 0 : 1;
            if(strcmp(av[0], "-d") == 0) return S_ISDIR(st.st_mode) ? 0 : 1;
        }
        return 1;
    }
    if(ac == 3) {
        int li = atoi(av[0]), ri = atoi(av[2]);
        if(strcmp(av[1], "=") == 0 || strcmp(av[1], "==") == 0) return (strcmp(av[0], av[2]) == 0) ? 0 : 1;
        if(strcmp(av[1], "!=") == 0) return (strcmp(av[0], av[2]) != 0) ? 0 : 1;
        if(strcmp(av[1], "-eq") == 0) return (li == ri) ? 0 : 1;
        if(strcmp(av[1], "-ne") == 0) return (li != ri) ? 0 : 1;
        if(strcmp(av[1], "-gt") == 0) return (li > ri) ? 0 : 1;
        if(strcmp(av[1], "-ge") == 0) return (li >= ri) ? 0 : 1;
        if(strcmp(av[1], "-lt") == 0) return (li < ri) ? 0 : 1;
        if(strcmp(av[1], "-le") == 0) return (li <= ri) ? 0 : 1;
    }
    fprintf(stderr, "Unsupported condition\n");
    return 1;
}

int exec_cmd(char **a, int c);

int exec_line(char **a, int c) {
    int st = 0, i = 0, b = 0;
    for(i = 0; i <= c; i++) {
        if(i == c || strcmp(a[i], ";") == 0 || strcmp(a[i], "&&") == 0 || strcmp(a[i], "||") == 0) {
            int l = i - b;
            if(l > 0) {
                char *cmd[MAXARGS];
                for(int x = 0; x < l; x++) cmd[x] = a[b + x];
                cmd[l] = NULL;
                int exec = 1;
                if(b > 0) {
                    if(strcmp(a[b-1], "&&") == 0 && st != 0) exec = 0;
                    if(strcmp(a[b-1], "||") == 0 && st == 0) exec = 0;
                }
                if(exec == 1) {
                    if(l >= 3 && strcmp(cmd[0], "[") == 0)
                        st = eval_cond(l, cmd);
                    else
                        st = exec_cmd(cmd, l);
                } else {
                    st = (strcmp(a[b-1], "&&") == 0) ? 1 : 0;
                }
            }
            b = i + 1;
        }
    }
    return st;
}

int exec_cmd(char **a, int c) {
    if(c == 0) return 0;
    if(bltin(a, c)) return 0;

    int infd, outfd, app;
    if(redir_parse(a, &c, &infd, &outfd, &app) < 0) return 1;

    pid_t pid = fork();
    if(pid < 0) {
        perror("fork");
        return 1;
    }
    if(pid == 0) {
        if(infd != STDIN_FILENO) {
            dup2(infd, STDIN_FILENO);
            close(infd);
        }
        if(outfd != STDOUT_FILENO) {
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
        }
        execvp(a[0], a);
        perror(a[0]);
        exit(127);
    }
    if(infd != STDIN_FILENO) close(infd);
    if(outfd != STDOUT_FILENO) close(outfd);

    int status = 0;
    while(waitpid(pid, &status, 0) == -1) {
        if(errno != EINTR) break;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int main(void) {
    signal(SIGINT, sigint_hdlr);
    char *ln = NULL;
    size_t cap = 0;
    char cwd[PATH_MAX];

    while(1) {
        if(int_flag) int_flag = 0;

        char *usr = getpwuid(getuid())->pw_name;
        if(!getcwd(cwd, sizeof(cwd))) strcpy(cwd, "~");

        printf("%s%s@%s$ %s", PROMPT_COLOR, usr, cwd, RESET_COLOR);
        fflush(stdout);

        ssize_t len = getline(&ln, &cap, stdin);
        if(len == -1) {
            printf("\n");
            break;
        }

        if(len > 0 && ln[len -1] == '\n')
            ln[len -1] = '\0';

        if(ln[0] == '\0') continue;

        int cnt = 0;
        char **tks = tkz(ln, &cnt);
        if(!tks) {
            fprintf(stderr, "Memory error\n");
            continue;
        }

        for(int i = 0; i < cnt; i++) {
            char *ex = exp_vars(tks[i]);
            free(tks[i]);
            tks[i] = ex;
        }

        exec_line(tks, cnt);
        free_toks(tks, cnt);
    }
    free(ln);
    return 0;
}
