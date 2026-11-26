#include <HAPI/core/event.hpp>

#include <chrono>
#include <mutex>
#include <vector>

namespace hyggshios::core {

struct EventBus::Impl {
    struct Entry {
        SubscriptionId id{0};
        std::string topic;
        EventHandler handler;
    };

    std::mutex mutex;
    SubscriptionId next_id{1};
    std::vector<Entry> entries;
};

EventBus::EventBus() : impl_(std::make_unique<Impl>()) {}
EventBus::~EventBus() = default;

SubscriptionId EventBus::subscribe(const std::string& topic, EventHandler handler) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    const SubscriptionId id = impl_->next_id++;
    impl_->entries.push_back(Impl::Entry{id, topic, std::move(handler)});
    return id;
}

bool EventBus::unsubscribe(SubscriptionId id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    for (auto it = impl_->entries.begin(); it != impl_->entries.end(); ++it) {
        if (it->id == id) {
            impl_->entries.erase(it);
            return true;
        }
    }
    return false;
}

void EventBus::emit(Event event) {
    if (event.timestamp_ms == 0) {
        const auto now = std::chrono::system_clock::now().time_since_epoch();
        event.timestamp_ms = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
    }

    std::vector<EventHandler> matched;
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        for (const auto& entry : impl_->entries) {
            if (entry.topic == "*" || entry.topic == event.topic) {
                matched.push_back(entry.handler);
            }
        }
    }

    for (const auto& handler : matched) {
        if (handler) {
            handler(event);
        }
    }
}

EventBus& EventBus::global() {
    static EventBus instance;
    return instance;
}

}  // namespace hyggshios::core
