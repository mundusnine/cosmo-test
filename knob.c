#define KNOB_IMPLEMENTATION
#include "knob.h"
extern const int __hostos;
#include <cosmo.h>

MAIN(test_build){
    
    if(!knob_mkdir_if_not_exists("./Deployment")){
        knob_log(KNOB_ERROR,"Failed creating Deployment folder. Something is really wrong...");
        return 1;
    }
    Knob_Cmd cmd = {0};
    //Build .so
    //For dll do:
    // # macOS
    // -framework Cocoa -framework AudioToolbox
    // # windows
    // -lgdi32 -lwinmm
    knob_cmd_append(&cmd,"cc","-I./Libraries/fenster","-ggdb3","-shared","-o","./Deployment/fenster.so","-fPIC","dll.c");
    if(IsLinux()){
        knob_cmd_append(&cmd,"-lX11","-lasound");
    }
    else if(IsWindows()){
        knob_cmd_append(&cmd,"-lgdi32","-lwinmm");
    }
    else if(IsXnu()){
        knob_cmd_append(&cmd,"-framework","Cocoa","-framework","AudioToolbox");
    }
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of library.");
        return 1;
    }
    cmd.count = 0;
    knob_cmd_append(&cmd,"./cosmocc/bin/cosmocc","-I./Libraries/fenster","-g","-o","./Deployment/calc.com","calc.c");
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of executable.");
        return 1;
    }


}