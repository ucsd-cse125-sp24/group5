#pragma once
typedef unsigned long size_t;

namespace bge {
    struct Entity {
        int id;
        friend bool operator<(const Entity &l, const Entity &r) { return l.id < r.id; }


        // these functions are needed to make unordered_set works well with Entity
        bool operator==(const Entity& otherEntity) const
        {
            return this->id == otherEntity.id;
        }

        struct HashFunction
        {
            // pretty bad hash function, but hey it's unique (as long as the id is generated properly)
            size_t operator()(const Entity& entity) const
            {
                return entity.id;
            }
        };
    };

}