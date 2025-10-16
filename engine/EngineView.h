#ifndef ENGINEVIEW_H
#define ENGINEVIEW_H

#include <memory>
#include <vector>

namespace CMPUT350
{

class GameObject;

    class EngineView
    {
        public:
            virtual ~EngineView() = default;

            // Allow adding new game objects to the engine
            virtual void AddGameObject(std::shared_ptr<GameObject> gameObject) = 0;

            using tGameObjectVec = std::vector<std::shared_ptr<GameObject>>;
            using const_iterator = tGameObjectVec::const_iterator;    
            virtual const_iterator cbegin() const = 0;
            virtual const_iterator cend() const = 0;
            virtual const_iterator begin() const = 0;
            virtual const_iterator end() const = 0;
    };
}

#endif
