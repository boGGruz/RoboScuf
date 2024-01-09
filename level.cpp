#include "level.h"

Tile wall = Tile{true, false, 1, 2, ' ', false, false};
Tile ground = Tile{false, true, 3, 4, ' ', false, false};
Tile up_stair = Tile{false, true, 6, 7, '<', false, false};
Tile down_stair = Tile{false, true, 6, 7, '>', false, false};

int max_rooms = 15;
int min_rooms = 10;

int min_room_size = 3;
int max_room_size = 10;

std::uniform_int_distribution<> room_y_distribution(1, kMapHeight-max_room_size-1);
std::uniform_int_distribution<> room_x_distribution(1, kMapWidth-max_room_size-1);
std::uniform_int_distribution<> room_size_distribution(min_room_size, max_room_size);
std::uniform_int_distribution<> number_rooms_distribution(min_rooms, max_rooms);

Level::Level() {
    for (int i = 0; i < kMapHeight; i++) {
        map_[i].fill(wall);
    }

    GenerateRooms();
    GenerateTunnels();
    ApplyRooms();
    PopulateRooms();
}

Level::~Level() = default;

Level Level::NextFloor() {
    Level new_floor;

    Room room = new_floor.GetRooms()[0];

    std::uniform_int_distribution<> stair_y(room.GetY1(), room.GetY2());
    std::uniform_int_distribution<> stair_x(room.GetX1(), room.GetX2());

    int y = stair_y(kRng);
    int x = stair_x(kRng);

    new_floor.map_[y][x] = up_stair;
    new_floor.up_stair_y = y;
    new_floor.up_stair_x = x;

    return new_floor;
}

void Level::TryMoveEntity(Level* source, Level* dest, Entity* entity) {
    if (source != nullptr && dest != nullptr && entity != nullptr) {
        dest->entities_.push_back(entity);
        source->entities_.erase(std::find(source->entities_.begin(), source->entities_.end(), entity));
    }
}

void Level::FreeEntities() {
    for (Entity* entity : entities_) {
        delete entity;
    }
}

std::array<std::array<bool, kMapWidth>, kMapHeight> Level::GetTransparent() {
    std::array<std::array<bool, kMapWidth>, kMapHeight> transparentmap{};

    for (int y = 0; y < kMapHeight; y++) {
        for (int x = 0; x < kMapWidth; x++) {
            transparentmap[y][x] = map_[y][x].transparent;
        }
    }
    return transparentmap;
}

std::vector<Room> Level::GetRooms() {
    return rooms_;
}

int Level::GetUpStairY() const {
    return up_stair_y;
}

int Level::GetUpStairX() const {
    return up_stair_x;
}

int Level::GetDownStairY() const {
    return down_stair_y;
}

int Level::GetDownStairX() const {
    return down_stair_x;
}

void Level::GenerateRooms() {
    int y,x,width,height;
    bool room_ok;
    for (int i = 0; i < number_rooms_distribution(kRng); i++) {
        room_ok = false;
        while (!room_ok) {
            y = room_y_distribution(kRng);
            x = room_x_distribution(kRng);
            width = room_size_distribution(kRng);
            height = room_size_distribution(kRng);

            room_ok = true;
            for (Room room : rooms_) {
                if (!(
                        ((room.GetX1() < x && room.GetX2() < x) ||
                            (room.GetX1() > x + width && room.GetX2() > x + width)) ||
                        ((room.GetY1() < y && room.GetY2() < y) ||
                                (room.GetY1() > y + height && room.GetY2() > y + height)))) {
                    room_ok = false;
                    break;
                }
            }

        }
        rooms_.emplace_back(y, x, width, height);
    }
}

void Level::GenerateTunnels() {
    int num_rooms = rooms_.size();
    for (int i = 0; i < num_rooms - 1; i++) {
        std::uniform_int_distribution<> origin_y(rooms_[i].GetY1(), rooms_[i].GetY2());
        std::uniform_int_distribution<> origin_x(rooms_[i].GetX1(), rooms_[i].GetX2());
        std::uniform_int_distribution<> target_y(rooms_[i+1].GetY1(), rooms_[i+1].GetY2());
        std::uniform_int_distribution<> target_x(rooms_[i+1].GetX1(), rooms_[i+1].GetX2());
        std::bernoulli_distribution y_first(0.5);

        int target[2] = {target_y(kRng), target_x(kRng)};

        if (y_first(kRng)) {
            int corner_x = origin_x(kRng);
            if (!(target[0] >= rooms_[i].GetY1() && target[0] <= rooms_[i].GetY2())) {
                if (target[0] < rooms_[i].GetY1()){ // if the tunnel needs to go up
                    rooms_.emplace_back(
                                target[0],
                                corner_x,
                                1,
                                rooms_[i].GetY1() - target[0]);
                } else { // otherwise, travel down first
                    rooms_.emplace_back(
                                rooms_[i].GetY2() + 1,
                                corner_x,
                                1,
                                target[0] - rooms_[i].GetY2());
                }
            }
            if (!(corner_x >= rooms_[i+1].GetX1() - 1 && corner_x <= rooms_[i+1].GetX2() + 1)) {
                if (target[1] < corner_x) { // if the tunnel needs to go left
                    rooms_.emplace_back(
                                target[0],
                                rooms_[i+1].GetX2() + 1,
                                (corner_x - 1) - rooms_[i+1].GetX2(),
                                1);
                } else { // otherwise travel right
                    rooms_.emplace_back(
                                target[0],
                                corner_x + 1,
                                rooms_[i+1].GetX1() - (corner_x + 1),
                                1);
                }
            }
        } else {
            int corner_y = origin_y(kRng);
            if (!(target[1] >= rooms_[i].GetX1() && target[1] <= rooms_[i].GetX2())) {
                if (target[1] < rooms_[i].GetX1()) { // if the tunnel needs to go left
                    rooms_.emplace_back(
                                corner_y,
                                target[1],
                                rooms_[i].GetX1() - target[1],
                                1);
                } else { // otherwise travel right
                    rooms_.emplace_back(
                                corner_y,
                                rooms_[i].GetX2() + 1,
                                target[1] - rooms_[i].GetX2(),
                                1);
                }
            }
            if (!(corner_y >= rooms_[i+1].GetY1() - 1 && corner_y <= rooms_[i+1].GetY2() + 1)) {
                if (target[0] < corner_y) { // if the tunnel needs to go up
                    rooms_.emplace_back(
                                rooms_[i+1].GetY2() + 1,
                                target[1],
                                1,
                                (corner_y - 1) - rooms_[i+1].GetY2());
                } else { // otherwise travel down
                    rooms_.emplace_back(
                                corner_y + 1,
                                target[1],
                                1,
                                (rooms_[i+1].GetY1() - 1) - corner_y);
                }
            }
        }
    }
}

void Level::ApplyRooms() {
    for (Room room : rooms_) {
        for (int y = 0; y < room.GetHeight(); y++) {
            for (int x = 0; x < room.GetWidth(); x++) {
                map_[room.GetY1() + y][room.GetX1() + x] = ground;
            }
        }
    }
}

void Level::PopulateRooms() {
    for (Room room : rooms_) {
        float area = room.GetArea();
        std::bernoulli_distribution chance_enemy(-(1/((area/100)+1))+1);

        while (chance_enemy(kRng)) {
            std::uniform_int_distribution<> enemy_y(room.GetY1(), room.GetY2());
            std::uniform_int_distribution<> enemy_x(room.GetX1(), room.GetX2());
            entities_.push_back(new Enemy(
                        "Sawbot",
                        "A cheap, unadorned steel cube with a spinning vibroblade that seems to rip even the air around it.",
                        enemy_y(kRng),
                        enemy_x(kRng),
                        's'));
        }
    }

    std::uniform_int_distribution<> room_num (0, rooms_.size() - 1);
    Room room = rooms_[room_num(kRng)];

    std::uniform_int_distribution<> stair_y(room.GetY1(), room.GetY2());
    std::uniform_int_distribution<> stair_x(room.GetX1(), room.GetX2());

    int y = stair_y(kRng);
    int x = stair_x(kRng);
    map_[y][x] = down_stair;
    down_stair_y = y;
    down_stair_x = x;
}
