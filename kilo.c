#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#define ctrl_key(k) ((k) & 0x1f)

struct 
termios original_term;

void
disableRawMode(){
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term) == -1){
    die('tcsetattr');
  }
}

void
enableRawMode(){
  if(tcgetattr(STDIN_FILENO, &original_term) == -1){
    die("tcgetattr");
  };
  atexit(disableRawMode);
  struct termios raw = original_term;
  
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if(tcsetattr(STDIN_FILENO,  TCSAFLUSH, &raw) == -1){
    die('tcsetattr');
  }
}

void
editorReadKey(){

}

void
editorKeyPress(){

}

void
die(const char *s){
  perror(s);
  exit(1);
}
int
main() {
  enableRawMode();
  while (1) {
    char c = '\0';
    if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == ctrl_key('q')) break;
  }
  return 0;
}