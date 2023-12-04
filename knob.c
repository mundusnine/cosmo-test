#define KNOB_IMPLEMENTATION
#include "knob.h"

#include <cosmo.h>

MAIN(test_build){

    if(!knob_mkdir_if_not_exists("./Tests")){
        knob_log(KNOB_ERROR,"Failed creating Test folder. Something is really wrong...");
        return 1;
    }
    Knob_Cmd cmd = {0};
    //Build .so
    knob_cmd_append(&cmd,"cc","-ggdb3","-shared","-o","./Tests/libdll.so","-fPIC","dll.c");
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of library.");
        return 1;
    }
    cmd.count = 0;
    knob_cmd_append(&cmd,"./cosmocc/bin/cosmocc","-g","-o","loader.com","libraryloader.c");
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of executable.");
        return 1;
    }


}