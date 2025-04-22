#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

namespace macos_checks {

void check_sip_status() {
    std::cout << "[*] Checking System Integrity Protection (SIP) status..." << std::endl;

    FILE* pipe = popen("csrutil status", "r");
    if (!pipe) {
        std::cerr << "[-] Failed to check SIP status" << std::endl;
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::cout << "    " << buffer;
    }
    pclose(pipe);
}

void check_gatekeeper() {
    std::cout << "\n[*] Checking Gatekeeper status..." << std::endl;

    FILE* pipe = popen("spctl --status", "r");
    if (!pipe) {
        std::cerr << "[-] Failed to check Gatekeeper status" << std::endl;
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::cout << "    " << buffer;
    }
    pclose(pipe);
}

void check_suid_files() {
    std::cout << "\n[*] Checking SUID/SGID files..." << std::endl;

    const std::vector<std::string> search_paths = {
        "/bin", "/sbin", "/usr/bin", "/usr/sbin",
        "/usr/local/bin", "/Applications"
    };

    for (const auto& path : search_paths) {
        std::string command = "find " + path +
                            " -type f \\( -perm -4000 -o -perm -2000 \\) -ls 2>/dev/null";
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) continue;

        char buffer[512];
        bool found = false;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            if (!found) {
                std::cout << "  In " << path << ":" << std::endl;
                found = true;
            }
            std::cout << "    " << buffer;
        }
        pclose(pipe);
    }
}

} // namespace macos_checks

void run_macos_checks() {
    std::vector<void(*)()> checks = {
        macos_checks::check_sip_status,
        macos_checks::check_gatekeeper,
        macos_checks::check_suid_files
        // Add new checks here
    };

    std::cout << "=== macOS Security Scanner ===" << std::endl;
    for (const auto& check : checks) {
        check();
    }
}

int main() {
    run_macos_checks();
    std::cout << "\n[+] Scan completed" << std::endl;
    return 0;
}