#define KNOB_IMPLEMENTATION
#include "knob.h"
extern const int __hostos;
#include <cosmo.h>
#define CONFIGURED

#ifdef CONFIGURED

#include CONFIG_PATH

char* args[] = {
    "dll",
    "app"
};
const int arg_len = sizeof(args)/sizeof(args[0]);
char* valid_apps[] = {
    "basic",
    "calc",
    "todo"
};

#define INCLUDES "-I./Libraries/fenster","-I.","-I./src","-I./build"
const int valid_apps_len = sizeof(valid_apps)/sizeof(valid_apps[0]);
//@TODO: Add config.h i.e. stage one of build to create an easy to configure config.h file to enable, among other things, mingw build
//@TODO: Add mingw64 build
MAIN(test_build){
    
    knob_log(KNOB_INFO, "--- STAGE 2 ---");

    char* program = knob_shift_args(&argc,&argv);
    int is_dll = 0;// argc > 0 ? knob_shift_args(&argc,&argv) : "";
    char* app_name = "basic";//calc, todo
    
    while(argc > 0){
        char* arg = knob_shift_args(&argc,&argv);
        if(knob_cstr_match(arg,"--dll")){
            is_dll = 1;
        }
        else if(knob_cstr_match(arg,"--app")){
            if(argc > 0){
                arg = knob_shift_args(&argc,&argv);
                int i = 0;
                for(; i < valid_apps_len;++i){
                    if(knob_cstr_match(arg,valid_apps[i])){
                        app_name = arg;
                        break;
                    }
                }
                if(i == valid_apps_len){
                    knob_log(KNOB_ERROR,"Argument after --app wasn't a valid app name, will default to basic app.");
                    char names[256] = {0};
                    char* noms = names;
                    for(int y = 0; y < valid_apps_len;++y){
                        noms = strcat(noms,valid_apps[y]);
                        noms = strcat(noms," ");
                    }
                    knob_log(KNOB_ERROR,"Valid app names are: %s",names);
                    exit(1);
                }
            }
            else {
                knob_log(KNOB_ERROR,"Must pass the app name after the --app command.");
            }
        }
    }

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
    knob_cmd_append(&cmd,"cc",INCLUDES,"-ggdb3","-shared","-fPIC","./src/dll.c");
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
    knob_cmd_append(&cmd,knob_temp_sprintf("./apps/%s/%s.c",app_name,app_name),"./src/renderer.c","./src/rencache.c","-o",knob_temp_sprintf("./Deployment/libplug%s", dll_ext));
    #else
    knob_cmd_append(&cmd,"-o",knob_temp_sprintf("./Deployment/fenster%s", dll_ext));
    #endif
    if(!knob_cmd_run_sync(cmd)){
        knob_log(KNOB_ERROR,"Failed build of library.");
        return 1;
    }
    if(is_dll){
        return 0;
    }
    cmd.count = 0;
    #ifdef NOT_COSMO
    if(IsWindows()){
        knob_cmd_append(&cmd,"./zig/zig.exe");
    }
    knob_cmd_append(&cmd,"cc","-DNOT_COSMO");
    #else
    knob_cmd_append(&cmd,"./cosmocc/bin/cosmocc");
    #endif
    

    knob_cmd_append(&cmd,INCLUDES,"-g","main.c","hotreload.c");

    #ifdef NOT_COSMO
    if(IsWindows()){
        knob_cmd_append(&cmd,"-ggdb3", "-std=c11", "-fno-sanitize=undefined","-fno-omit-frame-pointer");
    }
    knob_cmd_append(&cmd,"-o",knob_temp_sprintf("./Deployment/%s.com.dbg",app_name));
    #else
    knob_cmd_append(&cmd,"-o",knob_temp_sprintf("./Deployment/%s.com"),app_name));
    #endif
    #ifndef KNOB_HOTRELOAD
    knob_cmd_append(&cmd,knob_temp_sprintf("./apps/%s/%s.c",app_name,app_name),"./src/renderer.c","./src/rencache.c","./src/stb_truetype.c");
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

    {
        //@TODO: Should we add this option to knob, or is this too niche ?
        //Gf configuration START
        char curr_path[260] = {0};
        char gf_config[260] = {0};
        char* home = NULL;
        if(IsWindows()){
            home = "USERPROFILE";
        }
        else{
            home = "HOME";
        }
        snprintf(gf_config,260,"%s/%s",getenv(home),".config/gf2_config.ini");
        if(getcwd(curr_path,260) != NULL){
            strncat(curr_path,"/build/gf_pipe.dat",259);
            Knob_String_Builder sb = {0};
            if(knob_read_entire_file(gf_config,&sb)){
                char* pos = strstr(sb.items,"[pipe]");
                if(pos == NULL){
                    knob_sb_append_cstr(&sb,"[pipe]\n");
                    knob_sb_append_cstr(&sb,"control=");
                    knob_sb_append_buf(&sb,curr_path,260);
                    knob_sb_append_cstr(&sb,"\n");
                }
                else {
                    pos+=7;
                    int s = pos - sb.items;
                    while(s != sb.count){
                        sb.items[--sb.count] = '\0';
                    }
                    knob_sb_append_cstr(&sb,"control=");
                    knob_sb_append_buf(&sb,curr_path,260);
                    knob_sb_append_cstr(&sb,"\n");
                }
                if(knob_write_entire_file(gf_config,sb.items,sb.count)){
                    knob_log(KNOB_INFO,"Configured gf to use piped: %s",curr_path);
                }
            }
            if(!knob_file_exists(curr_path)){
                knob_log(KNOB_INFO, "Generating %s for gf pipe system\n",curr_path);
                Knob_Cmd cmd ={0};
                knob_cmd_append(&cmd,"mkfifo",curr_path);
                if(!knob_cmd_run_sync(cmd)){
                    knob_log(KNOB_INFO, "Failed creating %s for gf pipe system\n",curr_path);
                }
                //@TODO: cosmo doesn't seem to have mknod even if it's declared....
                // mknod(curr_path,S_IRWXU | S_IFIFO,0); // mkfifo
            }
        }
        //Gf configuration END
    }

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
