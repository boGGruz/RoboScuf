#include "menu.h"

MenuContext current_menu_context = kInven;

void ManageMenu(Entity* player) {
    int ch = GetLastCh();
    if (current_menu_context == kInven) {
        char current_char_option = 'a';
        for (auto item : player->GetInventory()->GetItems()) {
            if (item->count_ > 0) {
                if (current_char_option == 'q') current_char_option++;
                if (ch == current_char_option++) {
                    auto* pointer_to_eat = dynamic_cast<ComestibleItem*>(item);
                    if (pointer_to_eat) player->ConsumeItem(pointer_to_eat);
                }
            }
        }
    } else if (current_menu_context == kItemOptions) {
    }
}
