#include "NotificationManager.h"

namespace CMPUT350 {
    NotificationManager::~NotificationManager() {
        // Clear all listeners to prevent dangling references
        mListeners.clear();
    }

    void NotificationManager::Register(std::weak_ptr<GameObject> object, const std::string& key) {
        mListeners[key].push_back(object);
    }

    void NotificationManager::Unregister(std::weak_ptr<GameObject> object, const std::string& key) {
        auto it = mListeners.find(key);
        if (it == mListeners.end()) 
            return;

        auto& vec = it->second; //change to std::vector<std::weak_ptr<GameObject>? instead of auto

        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                            [&object](const std::weak_ptr<GameObject>& listener) {
                                return listener.lock() == object.lock(); // match by shared_ptr identity
                            }
                ), 
            vec.end());

        if (vec.empty()) {
            mListeners.erase(it); // Clean up empty keys
        }
    }

    void NotificationManager::Notify(const std::string& message) {
        auto it = mListeners.find(message);
        if (it == mListeners.end()) 
            return;

        auto& vec = it->second;

        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                                        [&message](std::weak_ptr<GameObject>& listener) {
                                            auto sp = listener.lock();
                                            if (!sp) 
                                                return true; // dead object, remove
                                            sp->ReceiveNotification(message); // notify
                                            return false;
                                        }
                                ), 
                    vec.end());

        if (vec.empty()) {
            mListeners.erase(it); //clean-up empty keys
        }
    }
}
