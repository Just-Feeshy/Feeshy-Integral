import sys.io.Process;


var THIRD_PARTY_PATH:String = "../third_party/";
final LIBRARY_PATH:String = "../third_party/";
final LIBRARY_LINK_PATH:String = "../third_party/libraries/";

final SDL_OPTIONS = [
    "SDL_STATIC=ON",
    "SDL_SHARED=OFF",
    "SDL_AUDIO=OFF",
    "SDL_ATOMIC=OFF",
    "SDL_VIDEO=ON",
    "SDL_RENDER=ON",
    "SDL_OPENGL=ON",
    "SDL_OPENGLES=ON",
    "SDL_HAPTIC=OFF",
    "SDL_JOYSTICK=OFF",
    "SDL_METAL=OFF",
];

function main():Void {
    if(!sys.FileSystem.exists(THIRD_PARTY_PATH)) {
        THIRD_PARTY_PATH = "third_party/";
    }

    {
        final compileProcess:Array<String> = ["-S", THIRD_PARTY_PATH + "SDL", "-B", THIRD_PARTY_PATH + "libraries"];

        if(Sys.systemName() == "Mac") {
            Sys.println("Detected macOS, setting deployment target to 11.0");
            compileProcess.push("-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0");
            compileProcess.push('-DCMAKE_OSX_ARCHITECTURES=arm64');
        }

        for(option in SDL_OPTIONS) {
            compileProcess.push("-D" + option);
        }

        final compileProcess = new Process("cmake", compileProcess);
        final stdoutContent = compileProcess.stdout.readAll().toString();
        final stderrContent = compileProcess.stderr.readAll().toString();
        final ec = compileProcess.exitCode();
        compileProcess.close();

        if(ec != 0) {
            trace(stderrContent);
            return;
        }
    }

    {
        final buildProcess = new Process("make", ["-C", THIRD_PARTY_PATH + "libraries"]);
        final stdoutContent = buildProcess.stdout.readAll().toString();
        final stderrContent = buildProcess.stderr.readAll().toString();
        final ec = buildProcess.exitCode();
        buildProcess.close();

        if(Sys.systemName() == "Mac") {
            final disableMTLProcess = new Process("nm", [THIRD_PARTY_PATH + "libraries/libSDL2.a | grep MTL"]);
            final disableMTLContent = disableMTLProcess.stdout.readAll().toString();
            final disableMTLExitCode = disableMTLProcess.exitCode();
            disableMTLProcess.close();
        }

        if(ec != 0) {
            trace("Error: " + stderrContent);
            return;
        }
    }
}
