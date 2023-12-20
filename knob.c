#define KNOB_IMPLEMENTATION
#include "knob.h"
extern const int __hostos;
#include <cosmo.h>


#ifdef CONFIGURED

#include CONFIG_PATH

//@TODO: Add config.h i.e. stage one of build to create an easy to configure config.h file to enable, among other things, mingw build
//@TODO: Add mingw64 build
MAIN(test_build){
    
    knob_log(KNOB_INFO, "--- STAGE 2 ---");

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
    knob_cmd_append(&cmd,"cc","-I./Libraries/fenster","-I./src","-I./build","-ggdb3","-shared","-fPIC","./src/dll.c");
    if(IsLinux()){
        //@TODO: Add wayland support when wayland is supported for NVIDIA GPU's...
        // knob_cmd_append(&cmd,"-lwayland-client");
        knob_cmd_append(&cmd,"-lX11","-lXinerama");
        knob_cmd_append(&cmd,"-lasound");
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
    #ifdef KNOB_HOTRELOAD
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
    knob_cmd_append(&cmd,"-I./Libraries/fenster","-I./src","-I./build","-g","main.c","hotreload.c");
    #ifdef NOT_COSMO
    if(IsWindows()){
        knob_cmd_append(&cmd,"-ggdb3", "-std=c11", "-fno-sanitize=undefined","-fno-omit-frame-pointer");
    }
    knob_cmd_append(&cmd,"-o","./Deployment/calc.com.dbg");
    #else
    knob_cmd_append(&cmd,"-o","./Deployment/calc.com");
    #endif
    #ifndef KNOB_HOTRELOAD
    knob_cmd_append(&cmd,"cacl.c","./src/renderer.c","./src/rencache.c","./src/stb_truetype.c");
    #endif
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of executable.");
        return 1;
    }
}

#else
#undef CC
#undef CC_PATH
#define CC_PATH "./cosmocc/bin/"
#define CC "cosmocc"
MAIN(STAGE_1){
    //@TODO: Add GO_REBUILD_YOURSELF tech TM
    knob_log(KNOB_INFO, "--- STAGE 1 ---");
    char* program = knob_shift_args(&argc,&argv);
    if (!knob_mkdir_if_not_exists("build")) return 1;

    Knob_Cmd cmd = {0};
    int config_exists = knob_file_exists(CONFIG_PATH);
    int plug_exists = knob_file_exists(PLUG_PATH);
    if(!plug_exists){
        knob_create_plug();
    } else {
        knob_log(KNOB_INFO, "file `%s` already exists", PLUG_PATH);
    }
    if (config_exists < 0) return 1;
    if (config_exists == 0) {
        knob_log(KNOB_INFO, "Generating %s", CONFIG_PATH);
        Knob_String_Builder content = {0};
        knob_create_default_config("CALC",&content,&cmd);
        if (!knob_write_entire_file(CONFIG_PATH, content.items, content.count)) return 1;
    } else {
        knob_log(KNOB_INFO, "file `%s` already exists", CONFIG_PATH);
    }

    cmd.count = 0;
    const char *configured_binary = "./build/knob.configured";
    knob_cmd_append(&cmd, KNOB_REBUILD_URSELF(configured_binary, "knob.c"), "-DCONFIGURED");
    if (!knob_cmd_run_sync(cmd)) return 1;

    cmd.count = 0;
    knob_cmd_append(&cmd, configured_binary);
    knob_da_append_many(&cmd, argv, argc);
    if (!knob_cmd_run_sync(cmd)) return 1;

    return 0;
}
#endif // CONFIGURED
