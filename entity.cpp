#include "entity.h"

int Entity::next_id_ = 0;

Entity::Entity(std::string name, std::string description, int y, int x, char avatar) {
    name_ = name;
    description_ = description;
    y_ = y;
    x_ = x;
    avatar_ = avatar;

    id_ = next_id_++;

    stats_[0] = 10; // normal entity has 10 hp
    stats_[1] = 0; // 0 defense
    stats_[2] = 0; // 0 maximum attack roll
    stats_[3] = 1; // 1 attack bonus (min damage)

    hp_ = stats_[0];

    equipment_slots_[0] = 1; // normal entity has 1 head
    equipment_slots_[1] = 2; // 2 arms
    equipment_slots_[2] = 1; // 1 torso
    equipment_slots_[3] = 2; // 2 feet

    faction_ = 0;

    doormat_ = true;

    living_ = false; // monsters attack living things, but shouldn't attack every entity
    dead_ = false; // entities start alive

    active_ = true; // whether or not it exists

    color_pair_ = 3;

    max_weight_ = 40.0;
}

Entity::~Entity() {
}

int Entity::GetX() {
    return x_;
}

int Entity::GetY() {
    return y_;
}

char Entity::GetAvatar() {
    return avatar_;
}

std::string Entity::GetName() {
    return name_;
}

int Entity::GetId() {
    return id_;
}

int Entity::GetHP() {
    return hp_;
}

std::array<int, 4> Entity::GetStats() {
    return stats_;
}

int Entity::GetFaction() {
    return faction_;
}

bool Entity::GetDoormat() {
    return doormat_;
}

bool Entity::GetLiving() {
    return living_;
}

bool Entity::GetDead() {
    return dead_;
}

short Entity::GetColorPair() {
    return color_pair_;
}

void Entity::Brain(map_type map, std::vector<Entity*>& entities) {
    return;
}

void Entity::CheckDead(std::vector<Entity*>& residents) {
}

void Entity::MoveAttack(int dy, int dx, map_type map, std::vector<Entity*> residents) {
    int new_y = y_ + dy;
    int new_x = x_ + dx;

    for (Entity* target : residents) {
        if (target->GetY() == new_y && target->GetX() == new_x) {
            if (target->GetFaction() != GetFaction() && !target->GetDead() && target->active_ && !target->GetDoormat()) {
                std::uniform_int_distribution<> damage_range(0, stats_[2]);

                int damage = damage_range(kRng) + stats_[3];

                target->TakeDamage(damage);

                main_log->AddMessage(std::string("").append(name_)
                        .append(std::string(" attacks ")).append(target->GetName())
                        .append(" for ").append(std::to_string(damage)).append(" damage!"));
                return;
            } else if (!target->GetDoormat()){
                return;
            }
        }
    }

    if (!map[new_y][new_x].blocking) {
        Move(dy, dx);
    }
}

void Entity::Move(int dy, int dx) {
    y_ += dy;
    x_ += dx;
}

void Entity::SetPos(int y, int x) {
    y_ = y;
    x_ = x;
}

void Entity::TakeDamage(int damage) {
    hp_ -= (damage - stats_[1]);
}

/* deal damage to another entity from this entity
 * maybe imoplement this later, but it probably isn't necessary
void Entity::Damage() {

} */

void Entity::Heal(int heal) {
    hp_ += heal;
}



Inventory* Entity::GetInventory() {
    return &inventory_;
}

float Entity::GetInvenWeight() {
    float weight = 0;
    for (auto item : inventory_.GetItems()) {
        weight += item->GetWeight()*item->count_;
    }
    return weight;
}

void Entity::PickupItem(Item* item){
    inventory_.AddItem(item);
}

void Entity::TakeItems(Entity* other_entity) {
    Inventory* other_inventory = other_entity->GetInventory();
    if (other_inventory->GetItems().size() > 0 && other_entity != this) {
        for (auto item : other_inventory->GetItems()) {
            Inventory::TryMove(other_inventory, &this->inventory_, item);
        }

         /*
        for (std::pair<Item, int> item : *other_inventory) {
            other_inventory->erase(item.first);
        }*/
    }
}

bool Entity::DropItem(Item* item, std::vector<Entity*>& entities) {
    auto item_it = inventory_.FindItem(item);
    if (item_it != inventory_.GetItems().end()) {
        int count = item->count_;
        if (count > 0) {
            std::string description;

            if (count > 1) {
                description = std::to_string(count).append("x ").append(item->GetName());
            } else {
                description = std::string("A ").append(item->GetName());
            }

            ItemEntity* new_entity = new ItemEntity(
                    item->GetName(),
                    description,
                    GetY(),
                    GetX(),
                    item->GetName()[0]);
            entities.push_back(new_entity);

            Inventory::TryMove(&inventory_, (*std::find(entities.begin(), entities.end(), new_entity))->GetInventory(), item);

            return true;
        }
    }
    return false;
}

void Entity::EquipItem(){
}

void Entity::DequipItem(){
}

void Entity::UseItem(){
}

bool Entity::ConsumeItem(ComestibleItem* item){
    if (item->count_ > 0) {
        item->Consume(this);
        item->count_--;
        return true;
    } else {
        return false;
    }
}


std::vector<std::vector<bool>> NonBlindEntity::GetFOV() {
    return FOV.SpiralPath(y_, x_);
}

void NonBlindEntity::UpdateFOVTransparent(std::array<std::array<bool, kMapWidth>, kMapHeight> transparentmap) {
    for (int y = 0; y < kMapHeight; y++) {
        for (int x = 0; x < kMapWidth; x++) {
            FOV.transparentmap_[y][x] = transparentmap[y][x];
        }
    }
}

void ItemEntity::Brain(map_type map, std::vector<Entity*>& entities) {
    int total_count = 0;
    for (auto item : inventory_.GetItems()) {
        total_count += item->count_;
    }
    if (total_count <= 0) {
        active_ = false;
    }
};
