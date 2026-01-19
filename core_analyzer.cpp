#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

string run_and_capture(const string& cmd) {
    string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return result;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

int main() {
    cout << "=== Core dump analyzer ===\n\n";

    cout << "1) Searching for core files (top 10):\n";
    system("find /tmp . ~ -name 'core*' -type f 2>/dev/null | head -10");

    cout << "\n2) Enter path to core file (e.g. /tmp/core.12345)\n";
    cout << "   or press Enter to exit:\n> ";

    string core_file;
    getline(cin, core_file);

    if (core_file.empty()) {
        cout << "No core file provided, exiting.\n";
        return 0;
    }

    cout << "\nChecking that file exists...\n";
    string check_cmd = "test -f '" + core_file + "' && echo 'File exists' || echo 'File NOT found!'";
    system(check_cmd.c_str());

    cout << "\n3) Core file info:\n";
    string info_cmd = "file '" + core_file + "'";
    system(info_cmd.c_str());

    cout << "\n4) Trying to detect executable from core file...\n";
    string prog_cmd =
        "file '" + core_file + "' | grep -o \"from '[^']*'\" | cut -d\\' -f2";
    string program = run_and_capture(prog_cmd);

    if (!program.empty() && program.back() == '\n') {
        program.pop_back();
    }

    if (!program.empty()) {
        cout << "   Detected executable: " << program << "\n";
    } else {
        cout << "   Failed to auto-detect executable.\n";
        cout << "   Please enter path to executable manually (e.g. ./forte-mock):\n> ";
        getline(cin, program);
    }

    if (!program.empty()) {
        cout << "\n5) Running basic GDB analysis...\n";
        cout << "========================================\n\n";

        string gdb_cmd =
            "gdb -q '" + program + "' '" + core_file + "'"
            " -ex \"bt\""
            " -ex \"info registers\""
            " -ex \"quit\" 2>&1";

        system(gdb_cmd.c_str());

        cout << "\n========================================\n";
        cout << "GDB analysis finished.\n";
    } else {
        cout << "Executable not provided, skipping GDB analysis.\n";
    }

    cout << "\n6) Lightweight inspection (strings tail):\n";
    cout << "Last 20 lines of strings(core):\n";

    string tail_cmd = "strings '" + core_file + "' | tail -20";
    system(tail_cmd.c_str());

    cout << "\nAnalysis complete.\n";
    cout << "Press Enter to exit...";
    cin.get();

    return 0;
}
