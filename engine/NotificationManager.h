#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include "GameObject.h"

namespace CMPUT350{
    class NotificationManager {
        public:
            ~NotificationManager();
            void Register(std::weak_ptr<GameObject> object, const std::string& key);
            void Unregister(std::weak_ptr<GameObject> object, const std::string& key);
            void Notify(const std::string& message);
        private:
            std::unordered_map<std::string, std::vector<std::weak_ptr<GameObject>>> mListeners;
        };
}

#endif