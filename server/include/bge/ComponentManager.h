#include <map>
#include "bge/Entity.h"

namespace bge {

    struct ComponentIndex {
        unsigned int index;
    };  

    template <typename ComponentType>
    struct ComponentData {
        unsigned int size = 0;
        std::array<ComponentType, 1024> data;
    };

    template <typename ComponentType>
    class ComponentManager {
        public:
            ComponentManager();
            ComponentIndex add(Entity e, ComponentType& c);
            ComponentType* lookup(Entity e);
            void destroy(Entity e);
        private:
            ComponentData<ComponentType> componentData;
            std::map<Entity, ComponentIndex> entityMap;
            std::map<ComponentIndex, Entity> componentMap;
    };

} 