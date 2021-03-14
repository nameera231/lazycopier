#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Declares our signal handler.
void message_handler(int signal, siginfo_t* info, void* ctx);

/**
 * This function will be called at startup and sets up a signal handler.
 */
__attribute__((constructor)) void init() {
  struct sigaction sa;
  
  // The signal hander is initialized.
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = message_handler;
  sa.sa_flags = SA_SIGINFO;

  // Set the signal handler, checking for errors
  if(sigaction(SIGSEGV, &sa, NULL) != 0) {
    perror("sigaction failed");
    exit(2);
  }

}

/**
 * This function will print an error message when it receives a signal
 * that a segmentation fault has occured. The message printed is selected 
 * at random from a list of 8.
 */
void message_handler(int signal, siginfo_t* info, void* ctx) {
  time_t t;
  srand(time(&t));
  char* messages[8] = {"It may be a seg fault, but it's not your fault.",
                       "Try strdup!", 
                       "You're trying to rewrite unwritable memory. This is for your own good.", 
                       "At least it compiled",
                       "It doesn't feel like an achievement without one or two seg faults anyway.",
                       "It's fine, I don't know what happened either.",
                       "Yeah, I miss Visual Studio Code too.",
                       "I know you can do it! All you need is one repl.it pro subscription I swear."};
  printf("Segmentation Fault: %s\n", messages[rand()%8]);
  // The program is terminated.
  exit(0);
}