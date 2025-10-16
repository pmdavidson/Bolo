#include "GameObject.h"

namespace CMPUT350
{
    GameObject::~GameObject()
    {
        // Clear the cached context pointer to prevent dangling references
        mLastContext = nullptr;
    }

    void GameObject::ReceiveNotification(const std::string& message)
    {
        // Default implementation does nothing
        // Subclasses can override this to handle notifications
    }
}
