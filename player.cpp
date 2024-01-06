#include "player.h"

Player::~Player() {
}

void Player::Brain(map_type map, std::vector<Entity*>& residents) {
    if (!dead_) {
        int ch = GetLastCh();
        if (ch == '8' || ch == KEY_UP) {
            MoveAttack(-1, 0, map, residents);
        } else if (ch == '9') {
            MoveAttack(-1, 1, map, residents);
        } else if (ch == '6' || ch == KEY_RIGHT) {
            MoveAttack(0, 1, map, residents);
        } else if (ch == '3') {
            MoveAttack(1, 1, map, residents);
        } else if (ch == '2' || ch == KEY_DOWN) {
            MoveAttack(1, 0, map, residents);
        } else if (ch == '1') {
            MoveAttack(1, -1, map, residents);
        } else if (ch == '4' || ch == KEY_LEFT) {
            MoveAttack(0, -1, map, residents);
        } else if (ch == '7') {
            MoveAttack(-1,-1, map, residents);
        } else if (ch == '5') {
            MoveAttack(0, 0, map, residents);
        }

        if (ch == 'g') {
            for (Entity* entity : residents) {
                if (entity->GetY() == GetY() && entity->GetX() == GetX()) {
                    TakeItems(entity);
                }
            }
        }
    }
}

void Player::CheckDead(std::vector<Entity*>& residents) {
    if (hp_ <= 0) {
        main_log->AddMessage("You are Dead!");
        color_pair_ = 5; 
        dead_ = true;
    }
}
