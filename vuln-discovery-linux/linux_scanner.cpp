#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

namespace linux_checks {

void check_suid_files() {
    std::cout << "\n[*] Checking SUID/SGID files..." << std::endl;

    const std::vector<std::string> search_paths = {
        "/bin", "/sbin", "/usr/bin", "/usr/sbin",
        "/usr/local/bin", "/usr/local/sbin"
    };

    for (const auto& path : search_paths) {
        DIR* dir = opendir(path.c_str());
        if (!dir) continue;

        dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string full_path = path + "/" + entry->d_name;
            struct stat file_stat;

            if (stat(full_path.c_str(), &file_stat) continue;

            if (file_stat.st_mode & S_ISUID) {
                std::cout << "[!] SUID file: " << full_path
                          << " (UID: " << file_stat.st_uid << ")" << std::endl;
            }

            if (file_stat.st_mode & S_ISGID) {
                std::cout << "[!] SGID file: " << full_path
                          << " (GID: " << file_stat.st_gid << ")" << std::endl;
            }
        }
        closedir(dir);
    }
}

void check_world_writable_files() {
    std::cout << "\n[*] Checking world-writable files..." << std::endl;

    const std::vector<std::string> sensitive_paths = {
        "/etc/passwd", "/etc/shadow", "/etc/sudoers",
        "/etc/crontab", "/etc/init.d", "/etc/cron.d",
        "/var/spool/cron"
    };

    for (const auto& path : sensitive_paths) {
        struct stat file_stat;
        if (stat(path.c_str(), &file_stat) == 0) {
            if (file_stat.st_mode & S_IWOTH) {
                std::cout << "[!] World-writable: " << path << std::endl;
            }
        }
    }
}

void check_cron_jobs() {
    std::cout << "\n[*] Checking cron jobs..." << std::endl;

    const std::vector<std::string> cron_paths = {
        "/etc/crontab",
        "/etc/cron.hourly",
        "/etc/cron.daily",
        "/etc/cron.weekly",
        "/etc/cron.monthly",
        "/var/spool/cron/crontabs"
    };

    for (const auto& path : cron_paths) {
        struct stat path_stat;
        if (stat(path.c_str(), &path_stat) != 0) continue;

        if (S_ISDIR(path_stat.st_mode)) {
            DIR* dir = opendir(path.c_str());
            if (!dir) continue;

            dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_name[0] == '.') continue;

                std::string full_path = path + "/" + entry->d_name;
                std::ifstream file(full_path);
                std::string line;

                while (std::getline(file, line)) {
                    if (line.find("* * * * *") != std::string::npos ||
                        line.find("@reboot") != std::string::npos) {
                        std::cout << "[!] Frequent cron job: " << full_path
                                  << "\n    Content: " << line << std::endl;
                    }
                }
            }
            closedir(dir);
        } else {
            std::ifstream file(path);
            std::string line;

            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;

                if (line.find("* * * * *") != std::string::npos ||
                    line.find("@reboot") != std::string::npos) {
                    std::cout << "[!] Frequent cron job in " << path
                              << "\n    Content: " << line << std::endl;
                }
            }
        }
    }
}

} // namespace linux_checks

void run_linux_checks() {
    std::vector<void(*)()> checks = {
        linux_checks::check_suid_files,
        linux_checks::check_world_writable_files,
        linux_checks::check_cron_jobs
        // Add new checks here
    };

    std::cout << "=== Linux Security Scanner ===" << std::endl;
    for (const auto& check : checks) {
        check();
    }
}

int main() {
    run_linux_checks();
    std::cout << "\n[+] Scan completed" << std::endl;
    return 0;
}