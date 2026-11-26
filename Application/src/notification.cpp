#include <HAPI/application/notification.hpp>
#include <HAPI/core/event.hpp>
#include <HAPI/core/permissions.hpp>

#include <cstdlib>
#include <filesystem>
#include <string>

#include <unistd.h>

#include <sys/wait.h>

namespace hyggshios::application {

core::Result<bool> send_notification(const NotificationRequest& request) {
    if (!core::has_permission(core::Permission::DesktopNotify)) {
        return core::Result<bool>::failure({
            core::ErrorCode::PermissionDenied,
            "Permission denied: DesktopNotify is required",
            0,
        });
    }

    if (request.title.empty()) {
        return core::Result<bool>::failure({
            core::ErrorCode::InvalidArgument,
            "Notification title cannot be empty",
            0,
        });
    }

    core::EventBus::global().emit(core::Event{
        "desktop.notification",
        request.app_name,
        request.title + ": " + request.body,
        0,
    });

    // Best-effort delivery via notify-send when running in a graphical session.
    const bool has_display =
        (std::getenv("DISPLAY") != nullptr) || (std::getenv("WAYLAND_DISPLAY") != nullptr);
    if (has_display && std::filesystem::exists("/usr/bin/notify-send")) {
        const pid_t pid = ::fork();
        if (pid == 0) {
            const char* urgency_str = "normal";
            if (request.urgency == NotificationUrgency::Low) {
                urgency_str = "low";
            } else if (request.urgency == NotificationUrgency::Critical) {
                urgency_str = "critical";
            }
            const std::string timeout_str = std::to_string(request.timeout_ms);
            ::execl("/usr/bin/notify-send",
                    "notify-send",
                    "-a",
                    request.app_name.c_str(),
                    "-u",
                    urgency_str,
                    "-t",
                    timeout_str.c_str(),
                    request.title.c_str(),
                    request.body.c_str(),
                    static_cast<char*>(nullptr));
            ::_exit(127);
        }
        if (pid > 0) {
            int status = 0;
            ::waitpid(pid, &status, 0);
        }
    }

    return core::Result<bool>::success(true);
}

}  // namespace hyggshios::application
