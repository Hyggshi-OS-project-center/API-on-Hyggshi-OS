#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace hyggshios::core {

/// Generic event payload dispatched through EventBus.
struct Event {
    std::string topic;   ///< Event topic (e.g., "file.changed", "app.started", "system.notification")
    std::string source;  ///< Emitter identifier
    std::string payload; ///< Optional serialized payload or message
    std::uint64_t timestamp_ms{0};
};

using EventHandler = std::function<void(const Event&)>;
using SubscriptionId = std::uint64_t;

/// Thread-safe publish/subscribe event bus for Core, System, and Application events.
class EventBus {
public:
    EventBus();
    ~EventBus();

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /// Subscribes a handler to a specific topic (or "*" for all topics).
    SubscriptionId subscribe(const std::string& topic, EventHandler handler);

    /// Removes a previously registered subscription.
    bool unsubscribe(SubscriptionId id);

    /// Emits an event to all matching subscribers.
    void emit(Event event);

    /// Returns the shared process-wide EventBus instance.
    static EventBus& global();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hyggshios::core

namespace hyggshi = hyggshios;
