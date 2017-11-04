#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OSC_TITLE 2

void usage() {
  printf(
      "Usage: ttoyctl <command> <args>\n"
      "\n"
      "Where <command> is one of:\n"
      "\thelp       Show help for a command\n"
      "\ttitle      Set the window title\n"
      "\tbg         Set the background\n"
      );
}

void print_osc(
    unsigned int code,
    const char *string)
{
  printf("\x1b\x5d" "%d;%s\a", code, string);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    usage();
    exit(1);
  }

  char *command = argv[1];

  int i = 0;
  while (command[i] != '\0') {
    command[i] = tolower(command[i]);
    i += 1;
  }

  if (strcmp(command, "help") == 0) {
    usage();
  } else if (strcmp(command, "title") == 0) {
    if (argc != 3) {
      usage();
      exit(1);
    }
    char *title = argv[2];
    print_osc(OSC_TITLE, title);
  } else {
    printf("Unknown command: %s\n\n", command);
    usage();
    exit(1);
  }

  exit(0);
}
