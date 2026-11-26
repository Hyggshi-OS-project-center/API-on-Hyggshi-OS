#pragma once

#include <HAPI/core/result.hpp>

#include <string>

namespace hyggshios::application {

/// Urgency level for desktop notifications.
enum class NotificationUrgency {
    Low,
    Normal,
    Critical
};

/// Desktop notification request.
struct NotificationRequest {
    std::string app_name{"Hyggshi OS"};
    std::string title;
    std::string body;
    NotificationUrgency urgency{NotificationUrgency::Normal};
    int timeout_ms{5000};
};

/// Sends a desktop notification (emits "desktop.notification" on EventBus and invokes
/// notify-send if available in a graphical session).
[[nodiscard]] core::Result<bool> send_notification(const NotificationRequest& request);

} // namespace hyggshios::application

namespace hyggshi = hyggshios;
