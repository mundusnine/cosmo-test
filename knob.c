#define KNOB_IMPLEMENTATION
#include "knob.h"
extern const int __hostos;
#include <cosmo.h>

//@TODO: Add config.h i.e. stage one of build to create an easy to configure config.h file to enable, among other things, mingw build
//@TODO: Add mingw64 build
#define HOTRELOAD
MAIN(test_build){
    char* program = knob_shift_args(&argc,&argv);
    char* arg = argc > 0 ? knob_shift_args(&argc,&argv) : "";
    if(!knob_mkdir_if_not_exists("./Deployment")){
        knob_log(KNOB_ERROR,"Failed creating Deployment folder. Something is really wrong...");
        return 1;
    }
    Knob_Cmd cmd = {0};
    //Build .so+
    //@TODO: Replace by zig
    char* dll_ext = ".so";
    if(IsWindows()){
        dll_ext = ".dll";
        knob_cmd_append(&cmd,"./zig/zig.exe");
    }
    knob_cmd_append(&cmd,"cc","-I./Libraries/fenster","-I./src","-ggdb3","-shared","-fPIC","./src/dll.c");
    if(IsLinux()){
        knob_cmd_append(&cmd,"-lX11","-lasound");
    }
    else if(IsWindows()){
        knob_cmd_append(&cmd,"-std=c11", "-fno-sanitize=undefined","-fno-omit-frame-pointer");
        knob_cmd_append(&cmd, "-target");
        knob_cmd_append(&cmd, "x86_64-windows");
        knob_cmd_append(&cmd,"-lgdi32","-lwinmm");
    }
    else if(IsXnu()){
        knob_cmd_append(&cmd,"-framework","Cocoa","-framework","AudioToolbox");
    }
    #ifdef HOTRELOAD
    knob_cmd_append(&cmd,"calc.c","./src/renderer.c","./src/rencache.c","-o",knob_temp_sprintf("./Deployment/libplug%s", dll_ext));
    #else
    knob_cmd_append(&cmd,"-o",knob_temp_sprintf("./Deployment/fenster%s", dll_ext));
    #endif
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of library.");
        return 1;
    }
    if(knob_cstr_match(arg,"--dll")){
        return 0;
    }
    cmd.count = 0;
    #define NOT_COSMO
    #ifdef NOT_COSMO
    if(IsWindows()){
        knob_cmd_append(&cmd,"./zig/zig.exe");
    }
    knob_cmd_append(&cmd,"cc","-DNOT_COSMO");
    #else
    knob_cmd_append(&cmd,"./cosmocc/bin/cosmocc");
    #endif
    knob_cmd_append(&cmd,"-I./Libraries/fenster","-I./src","-g","main.c","hotreload.c");
    #ifdef NOT_COSMO
    if(IsWindows()){
        knob_cmd_append(&cmd,"-ggdb3", "-std=c11", "-fno-sanitize=undefined","-fno-omit-frame-pointer");
    }
    knob_cmd_append(&cmd,"-o","./Deployment/calc.com.dbg");
    #else
    knob_cmd_append(&cmd,"-o","./Deployment/calc.com");
    #endif
    #ifndef HOTRELOAD
    knob_cmd_append(&cmd,"cacl.c","./src/renderer.c","./src/rencache.c","./src/stb_truetype.c");
    #endif
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of executable.");
        return 1;
    }


}
