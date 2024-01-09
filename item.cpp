#include "item.h"
#include "entity.h"

std::string Item::GetName() {
    return name_;
}

float Item::GetWeight() {
    return weight_;
}

float Item::GetValue() {
    return value_;
}

Item* Item::Clone() {
    return new Item(name_, weight_, value_);
}


Entity* EquippableItem::GetUser() {
    return user_;
}

int EquippableItem::GetSlot() {
    return slot_;
}

std::array<int, 4> EquippableItem::GetBonuses() {
    return bonuses_;
}

void EquippableItem::SetUser(Entity* user) {
    user_ = user;
}


void HealingItem::Consume(Entity* user) {
    user->Heal(power_);
}

HealingItem* HealingItem::Clone() {
    return new HealingItem(GetName(), GetWeight(), GetValue(), power_);
}
