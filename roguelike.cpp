#include "roguelike.h"

int main() {
    InitialiseRenderer();

    main_log = new Log("Main");
    debug_log = new Log("Debug");

    std::vector<Level> levels;

    levels.emplace_back();

    main_log->AddMessage("Welcome to Young Scuf Roguelike!");
    main_log->AddMessage("You are the ROBOSCUF.");
    main_log->AddMessage("---INSTRUCTIONS---");
    main_log->AddMessage("Use arrow keys or the numpad to move your character (@)");
    main_log->AddMessage("Run into the oncoming sawbots to destroy them");
    main_log->AddMessage("Use 'g' to pick up items that fall on the ground");
    main_log->AddMessage("Use 'i' to access your inventory, and the letters surrounded by parentheses to use items");
    main_log->AddMessage("Press '>' and '<' while standing on the corresponding tiles to move between levels");
    main_log->AddMessage("Use 'q' to exit menus and the game");

    Player* player = new Player(
            (levels[0].GetRooms()[0].GetY1() + levels[0].GetRooms()[0].GetY2())/2,
            (levels[0].GetRooms()[0].GetX1() + levels[0].GetRooms()[0].GetX2())/2);

    levels[0].entities_.push_back(player);
    player->PickupItem(new HealingItem("Repair kit", 10.0, 50.0, 10));

    FrameInfo frame_info = {kNone, kMain};
    Level* level = &levels[0];
    int current_level = 1;
    do {
        switch (frame_info.input_context) {
            case kMain: {
                if (frame_info.input_type == kFloorDown) {
                    if (level->map_[player->GetY()][player->GetX()].character == '>') {
                        if (current_level == levels.size()) {
                            levels.push_back(level->NextFloor());
                        }

                        Level::TryMoveEntity(&levels[current_level-1], &levels[current_level], player);
                        player->SetPos(levels[current_level].GetUpStairY(), levels[current_level].GetUpStairX());

                        level = &levels[current_level]; // level -1 + 1
                        current_level++;

                        main_log->AddMessage("The player is transported one floor closer to the core...");
                    }
                }

                if (frame_info.input_type == kFloorUp) {
                    if (level->map_[player->GetY()][player->GetX()].character == '<') {
                        if (current_level > 1) {
                            level = &levels[current_level-2];
                            Level::TryMoveEntity(&levels[current_level-1], &levels[current_level-2], player);
                            player->SetPos(levels[current_level-2].GetDownStairY(), levels[current_level-2].GetDownStairX());
                            current_level--;
                            main_log->AddMessage("The player is transported one floor away from the core...");
                        }
                    }
                }

                if (frame_info.input_type == kAction) {
                    for (int e = level->entities_.size() - 1; e >= 0; e--) {
                        if (level->entities_[e]->active_)
                            level->entities_[e]->Brain(level->map_, level->entities_);
                    }
                    for (int e = level->entities_.size() - 1; e >= 0; e--) {
                        if (level->entities_[e]->active_)
                            level->entities_[e]->CheckDead(level->entities_);
                    }
                }

                std::array<std::array<bool, kMapWidth>, kMapHeight> transparentmap = level->GetTransparent();
                for (Entity* entity : level->entities_) {
                    NonBlindEntity* non_blind_entity = dynamic_cast<NonBlindEntity*>(entity);
                    if (non_blind_entity) non_blind_entity->UpdateFOVTransparent(transparentmap);
                }

                std::vector<std::vector<bool>> visible = player->GetFOV();
                for (int y = 0; y < kMapHeight; y++) {
                    for (int x = 0; x < kMapWidth; x++) {
                        if (visible[y][x]) {
                            level->map_[y][x].lit = true;
                            level->map_[y][x].seen = true;
                        } else {
                            level->map_[y][x].lit = false;
                        }
                    }
                }

                if (player->GetDead()) {
                    int high;
                    std::ifstream hiscore_in;
                    std::ofstream hiscore_out;
                    hiscore_in.open("scores.dat");

                    hiscore_in >> high;
                    hiscore_in.close();

                    main_log->AddMessage("!---GAME OVER---!");
                    main_log->AddMessage(std::string("You made it to floor ").append(std::to_string(current_level)));

                    if (!high || current_level > high) {
                        high = current_level;
                        main_log->AddMessage("NEW HIGH SCORE!");
                    }
                    main_log->AddMessage(std::string("The high score is ").append(std::to_string(high)));

                    hiscore_out.open("scores.dat", std::ios::trunc);
                    hiscore_out << high << std::endl;
                    hiscore_out << "Don't edit this file, it's just scummy." << std::endl;
                    hiscore_out.close();
                }

                RenderLevel(level);
                AddLogMessages(main_log);
                AddLogMessages(debug_log);
                RenderHud(player, current_level);

                break;
            }
            case kMenu: {
                ManageMenu(player);
                RenderMenu(player);
                break;
            }
        }

        frame_info = input(frame_info);

    } while (frame_info.input_context != kAbort);
    endwin();

    delete main_log;
    delete debug_log;

    for (Level level : levels) {
        level.FreeEntities();
    }

    return 0;
}
