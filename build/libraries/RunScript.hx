import sys.io.Process;


var THIRD_PARTY_PATH:String = "../third_party/";
final LIBRARY_PATH:String = "../third_party/";
final LIBRARY_LINK_PATH:String = "../third_party/libraries/";

function main():Void {
    if(!sys.FileSystem.exists(THIRD_PARTY_PATH)) {
        THIRD_PARTY_PATH = "third_party/";
    }

    {
        final compileProcess = new Process("cmake", ["-S", THIRD_PARTY_PATH + "SDL", "-DSDL_STATIC=ON", "-DSDL_SHARED=OFF", "-B", THIRD_PARTY_PATH + "libraries"]);
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

        if(ec != 0) {
            trace("Error: " + stderrContent);
            return;
        }
    }
}
