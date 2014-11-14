#ifndef _SHELL_CMDS_H_
#define _SHELL_CMDS_H_

#include "filesys.h"

void DirLine(stat_t *p, char *line);
void ShellDir(char *str, int stdout, int filesys);
void ShellType(char *str, int stdout, int filesys);
void ShellHelp(int stdout_pid);
void ShellWho(int stdout_pid);
void ShellPrint(char *cmd_str, int print_driver_pid, int fs_pid);
void ShellExec(char *cmd_str, int stdout, int fs_pid);


#endif
