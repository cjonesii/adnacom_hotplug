#include "main.h"
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

extern struct adna_options AdnaOptions;

/* Main */
int main(int argc, char **argv)
{
  verbose = 2; // flag used by pci process
  int status = EXIT_SUCCESS;
  uint8_t remaining = 3; // arbitrary delay before first run

  struct itimerval new_timer;
  struct itimerval old_timer;

  new_timer.it_value.tv_sec = 1;
  new_timer.it_value.tv_usec = 0;
  new_timer.it_interval.tv_sec = 0;
  new_timer.it_interval.tv_usec = 100 * 1000;

  if (argc == 2 && !strcmp(argv[1], "--version")) {
    puts("Adnacom Hotplug Tool version " ADNATOOL_VERSION);
    puts("Supports: H1A (PEX8608), H18/H3/H12 (PEX8718)");
    return 0;
  } else if ((argc == 2 && !strcmp(argv[1], "-v"))) {
    AdnaOptions.bVerbose = true;
  }

  status = adna_pci_process();
  if (status != EXIT_SUCCESS)
    exit(1);
  else
    adna_set_init_flag(true);

  setitimer(ITIMER_REAL, &new_timer, &old_timer);
  signal(SIGALRM, adna_timer_callback);

  while (sleep(remaining) != 0) {
    if (errno == EINTR) {
      ;// PRINTF("Timer Interrupt ");
    } else {
      printf("Sleep error %s\n", strerror(errno));
    }
  }

  status = adna_delete_list();
  if (status != EXIT_SUCCESS)
    exit(1);

  return (adna_get_errors() ? 2 : 0);
}