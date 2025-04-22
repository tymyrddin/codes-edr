#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <tlhelp32.h>
#include <psapi.h>
#include <aclapi.h>
#include <sddl.h>

namespace win_checks {

void check_admin_privileges() {
    BOOL is_admin = FALSE;
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
    PSID admin_group;

    if (AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &admin_group)) {
        if (!CheckTokenMembership(NULL, admin_group, &is_admin)) {
            is_admin = FALSE;
        }
        FreeSid(admin_group);
    }

    std::cout << "[*] Running as admin: " << (is_admin ? "YES (Dangerous)" : "NO") << std::endl;
}

void check_service_permissions() {
    std::cout << "\n[*] Checking service permissions..." << std::endl;

    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) {
        std::cerr << "[-] Failed to open service manager: " << GetLastError() << std::endl;
        return;
    }

    std::vector<std::string> critical_services = {"SamSs", "LanmanServer", "Dhcp", "DNS"};

    for (const auto& service_name : critical_services) {
        SC_HANDLE service = OpenService(scm, service_name.c_str(), READ_CONTROL);
        if (service) {
            PSECURITY_DESCRIPTOR sd = NULL;
            if (GetServiceSecurityInfo(service, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL, &sd) == ERROR_SUCCESS) {
                PACL dacl = NULL;
                BOOL dacl_present = FALSE;
                BOOL dacl_defaulted = FALSE;

                if (GetSecurityDescriptorDacl(sd, &dacl_present, &dacl, &dacl_defaulted)) {
                    if (!dacl_present || dacl == NULL) {
                        std::cout << "[!] " << service_name << " service has no DACL (Full access to everyone)" << std::endl;
                    }
                }
                LocalFree(sd);
            }
            CloseServiceHandle(service);
        }
    }
    CloseServiceHandle(scm);
}

void check_unquoted_service_paths() {
    std::cout << "\n[*] Checking for unquoted service paths..." << std::endl;

    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) {
        std::cerr << "[-] Failed to open service manager" << std::endl;
        return;
    }

    DWORD bytes_needed = 0;
    DWORD service_count = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytes_needed, &service_count, NULL);

    if (bytes_needed > 0) {
        std::vector<BYTE> buffer(bytes_needed);
        LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)&buffer[0];

        if (EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytes_needed,
                             &bytes_needed, &service_count, NULL)) {
            for (DWORD i = 0; i < service_count; i++) {
                SC_HANDLE service = OpenService(scm, services[i].lpServiceName, SERVICE_QUERY_CONFIG);
                if (service) {
                    QueryServiceConfig(service, NULL, 0, &bytes_needed);
                    if (bytes_needed > 0) {
                        std::vector<BYTE> service_config_buffer(bytes_needed);
                        LPQUERY_SERVICE_CONFIG service_config = (LPQUERY_SERVICE_CONFIG)&service_config_buffer[0];

                        if (QueryServiceConfig(service, service_config, bytes_needed, &bytes_needed)) {
                            if (service_config->lpBinaryPathName != NULL) {
                                std::string path = service_config->lpBinaryPathName;
                                if (path.find(' ') != std::string::npos && path[0] != '"') {
                                    std::cout << "[!] " << services[i].lpServiceName
                                              << " has unquoted path: " << path << std::endl;
                                }
                            }
                        }
                    }
                    CloseServiceHandle(service);
                }
            }
        }
    }
    CloseServiceHandle(scm);
}

} // namespace win_checks

void run_windows_checks() {
    std::vector<void(*)()> checks = {
        win_checks::check_admin_privileges,
        win_checks::check_service_permissions,
        win_checks::check_unquoted_service_paths
        // Add new checks here
    };

    std::cout << "=== Windows Security Scanner ===" << std::endl;
    for (const auto& check : checks) {
        check();
    }
}

int main() {
    run_windows_checks();
    std::cout << "\n[+] Scan completed" << std::endl;
    return 0;
}
