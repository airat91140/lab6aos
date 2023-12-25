#include <stdio.h>

#include "daemon.h"
#include "master.h"

int main() {
    int err_code; 
    err_code = launch_daemon();
    return err_code < 0 ? err_code : master();
}