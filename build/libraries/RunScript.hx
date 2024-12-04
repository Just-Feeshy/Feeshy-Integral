import sys.io.Process;

final LIBRARY_PATH:String = "../third_party/";
final LIBRARY_LINK_PATH:String = "../third_party/libraries/";

function main():Void {
    {
        final compileProcess = new Process("cmake", ["..", "-DBUILD_SHARED_LIBS=OFF", "-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=" + LIBRARY_LINK_PATH]);
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
        final buildProcess = new Process("cmake build", []);
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
