#ifndef ENEMY_H_
#define ENEMY_H_

#include <vector>
#include <string>

#include "logger.h"
#include "constants.h"
#include "entity.h"
#include "spiralpath.h"
#include "rng.h"

class Enemy : public NonBlindEntity {
    public:
        Enemy(std::string name, std::string description, int y, int x, char avatar) :
            NonBlindEntity(name, description, y, x, avatar, 15),
            target_(nullptr) {
                std::bernoulli_distribution item_chance(0.5);
                if (item_chance(kRng)) {
                    PickupItem(new HealingItem("Metal scrap", 5.0, 2.0, 2));
                }
                faction_ = 1;
                doormat_ = false;
            };
        ~Enemy();
        void Brain(map_type, std::vector<Entity*>&);
        void CheckDead(std::vector<Entity*>&);
    private:
        Entity* target_;
};

#endif
