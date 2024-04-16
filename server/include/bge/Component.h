namespace bge {
    struct ComponentCounter {
        static int familyCounter;
    };

    int ComponentCounter::familyCounter = 0;

    template <typename ComponentType>
    struct Component {
        static inline int family() {
            static int family = ComponentCounter::familyCounter++;
            return family;
        }
    };

    template <typename C>
    static int GetComponentFamily() {
        return Component<typename std::remove_const<C>::type>::family();
    }

    struct HealthComponent : Component<HealthComponent> {
        HealthComponent(int curr, int max): 
            currHealth(curr), 
            maxHealth(max)
            {};
        int currHealth;
        int maxHealth;
    };

    struct PositionComponent: Component<PositionComponent> {
        PositionComponent(float x, float y, float z): 
            x(x),
            y(y),
            z(z)
            {};
        float x;
        float y;
        float z;
    };

}
