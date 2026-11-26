#include <HAPI/application/app_info.hpp>
#include <HAPI/application/notification.hpp>
#include <HAPI/core/event.hpp>
#include <HAPI/system/file_api.hpp>
#include <HAPI/system/security_api.hpp>

#include <iostream>
#include <string>

int main() {
    // --- Security & User Info ---
    std::cout << "=== Current User ===\n";
    if (auto user = hyggshi::system::security::try_get_current_user(); user.ok()) {
        const auto& u = user.value();
        std::cout << "Username: " << u.username << " (uid=" << u.uid << ")\n";
        std::cout << "Home: " << u.home_dir << " | Shell: " << u.shell << "\n";
        std::cout << "Root: " << (u.is_root ? "yes" : "no") << "\n";
    }

    if (auto groups = hyggshi::system::security::try_get_current_groups(); groups.ok()) {
        std::cout << "Groups:";
        for (const auto& g : groups.value()) {
            std::cout << " " << g.name;
        }
        std::cout << "\n";
    }

    if (auto caps = hyggshi::system::security::try_get_capabilities(); caps.ok()) {
        std::cout << "CapEff: " << caps.value().cap_effective << "\n\n";
    }

    // --- File API ---
    std::cout << "=== File API ===\n";

    // Subscribe to file events on the global EventBus
    hyggshios::core::EventBus::global().subscribe("*", [](const hyggshios::core::Event& ev) {
        std::cout << "[Event] topic=" << ev.topic << " payload=" << ev.payload << "\n";
    });

    const std::string test_file = "/tmp/hapi_example_file.txt";
    const std::string content = "Hello from Hyggshi OS File API!\nThis file was written by HAPI.\n";

    if (auto w = hyggshi::system::file::write_text(test_file, content); w.ok()) {
        std::cout << "Written: " << test_file << "\n";
    }

    if (auto r = hyggshi::system::file::read_text(test_file); r.ok()) {
        std::cout << "Read back:\n" << r.value();
    }

    if (auto st = hyggshi::system::file::stat(test_file); st.ok()) {
        std::cout << "Size: " << st.value().size_bytes << " bytes\n";
    }

    hyggshi::system::file::remove(test_file);

    // --- Application ---
    std::cout << "\n=== Application ===\n";
    if (auto app = hyggshi::application::try_get_current_app(); app.ok()) {
        std::cout << "Name: " << app.value().name << " | PID: " << app.value().pid << "\n";
        std::cout << "Executable: " << app.value().executable_path << "\n";
        std::cout << "Working dir: " << app.value().working_directory << "\n";
    }

    // Installed .desktop apps (up to 5)
    if (auto apps = hyggshi::application::try_list_installed_apps(5); apps.ok()) {
        std::cout << "\nInstalled apps (first 5):\n";
        for (const auto& a : apps.value()) {
            std::cout << "  " << a.name << " (" << a.id << ")\n";
        }
    }

    // --- Desktop Notification (fires notify-send if graphical session) ---
    std::cout << "\n=== Notification ===\n";
    hyggshi::application::NotificationRequest notif;
    notif.app_name = "HAPI Example";
    notif.title = "HAPI is running";
    notif.body = "File API, Security, and Notification example completed.";
    if (auto n = hyggshi::application::send_notification(notif); n.ok()) {
        std::cout << "Notification dispatched.\n";
    }

    return 0;
}
