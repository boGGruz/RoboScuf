#include "renderer.h"

WINDOW* ncurses_hud_window;
WINDOW* ncurses_menu_window;

int screen_width;
int screen_height;

int hud_width;
int hud_message_number = 0;
const int kMaxHudMessages = 20;
std::string hud_messages[kMaxHudMessages];

void RenderEntities(std::vector<Entity*>, map_type);
void RenderEnvironment(map_type);

void PrintHudMessages();


int GetIntLength(int num) {
    int digits = 0;
    if (num <= 0) {
        num = 0 - num;
        digits++;
    }
    while (num >= 1) {
        num /= 10;
        digits++;
    }
    return digits;
}

void InitialiseRenderer() {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    getmaxyx(stdscr, screen_height, screen_width);

    stdscr = newwin(screen_height, screen_width, 0, 0);

    hud_width = screen_width-kMapWidth;
    ncurses_hud_window = newwin(screen_height, hud_width, 0, kMapWidth);

    ncurses_menu_window = newwin(screen_height, screen_width, 0, 0);

    wrefresh(stdscr);
    wrefresh(ncurses_hud_window);

    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_RED, 1000, 0, 0);
    init_color(COLOR_GREEN, 0, 1000, 0);
    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_BLUE, 1000, 0, 800);
    init_color(COLOR_MAGENTA, 1000, 0, 1000);
    init_color(COLOR_CYAN, 0, 1000, 1000);
    init_color(COLOR_WHITE, 1000, 1000, 1000);
    init_color(8, 0, 0, 500);
    init_color(10, 0, 100, 100);
    init_color(11, 100, 100, 0); // dark yellow

    init_pair(0, COLOR_BLACK, COLOR_BLACK);
    init_pair(1, COLOR_BLACK, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, 8);
    init_pair(3, COLOR_BLACK, COLOR_CYAN);
    init_pair(4, COLOR_BLACK, 10); // dark cyan
    init_pair(5, COLOR_BLACK, COLOR_RED); // blood
    init_pair(6, COLOR_BLACK, COLOR_YELLOW);
    init_pair(7, COLOR_BLACK, 11);

    bkgd(' ' | COLOR_PAIR(0));
}

void RenderHud(Entity* player, int level) {
    wclear(ncurses_hud_window);
    box(ncurses_hud_window, 0, 0);

    mvwprintw(ncurses_hud_window, 1, 1, "%s", player->GetName().c_str());
    mvwprintw(ncurses_hud_window, 2, 1, "HP: %d", player->GetHP());
    mvwprintw(ncurses_hud_window, 3, 1, "Floor: %d", level);

    std::array<int, 4> stats = player->GetStats();
    std::string stat_titles[4] = {"Max HP: ", "DEF: ", "ATK Roll: ", "ATK Bonus: "};

    int statbox_width = 0;
    for (int i = 0; i < stats.size(); i ++) {
        if (stat_titles[i].length() + GetIntLength(stats[i]) > statbox_width) {
            statbox_width = stat_titles[i].length() + GetIntLength(stats[i]);
        }
    }

    for (int i = 0; i < stats.size(); i++) {
        mvwprintw(
                ncurses_hud_window,
                i + 1,
                hud_width - 2 - statbox_width,
                "%s",
                stat_titles[i].c_str());

        mvwprintw(
                ncurses_hud_window,
                i + 1,
                hud_width - 1 - GetIntLength(stats[i]),
                "%d", stats[i]);
    }

    PrintHudMessages();

    wrefresh(ncurses_hud_window);
}

void RenderMenu(Entity* player) {
    wclear(ncurses_menu_window);
    box(ncurses_menu_window, 0, 0);
    mvwprintw(ncurses_menu_window, 1, 1, "Inventory of %s", player->GetName().c_str());

    Inventory* inventory = player->GetInventory();

    
    int i = 0;
    char option_char = 'a';
    for (auto item : inventory->GetItems()) {
        
        if (option_char == 'q') option_char++;

        if (item->count_ > 0)
            mvwprintw(ncurses_menu_window, 2+i++, 1, "%c) %dx %s", option_char++, item->count_, item->GetName().c_str());
    }

    wrefresh(ncurses_menu_window);
}

void PrintHudMessages() {
    int current_term_line = screen_height-1;
    for (int i = hud_message_number - 1; i > hud_message_number - kMaxHudMessages; i--) {
        if (i < 0 || current_term_line <= kMaxHudMessages) {
            break;
        }

        int num_lines = ceil(hud_messages[i%kMaxHudMessages].length() / (float) (hud_width - 2));

        for (int l = 0; l < num_lines; l++) {
            mvwprintw(
                    ncurses_hud_window,
                    current_term_line - num_lines + l,
                    1,
                    "%s",
                    hud_messages[i%kMaxHudMessages]
                        .substr((hud_width - 2) * l,(hud_width - 2) * (l + 1)).c_str());
        }

        current_term_line -= num_lines;
    }
}

void RenderLevel(Level* level) {
    wclear(stdscr);

    RenderEnvironment(level->map_);
    RenderEntities(level->entities_, level->map_);

    refresh();
}

void RenderEnvironment(map_type map) {
    for (int y = 0; y < kMapHeight; y++) {
        for (int x = 0; x < kMapWidth; x++) {
            if (map[y][x].lit) {
                mvaddch(y, x, map[y][x].character | COLOR_PAIR(map[y][x].lit_color_pair));
            } else {
                if (map[y][x].seen) {
                    mvaddch(y, x, map[y][x].character | COLOR_PAIR(map[y][x].unlit_color_pair));
                }
            }
        }
    }
}

void RenderEntities(std::vector<Entity*> entities, map_type map) {
    std::vector<Entity*> to_render_first;
    std::vector<Entity*> to_render_second;

    for (Entity* entity : entities) {
        if (map[entity->GetY()][entity->GetX()].lit && entity->active_) {
            if (entity->GetDoormat()) to_render_first.push_back(entity);
            else to_render_second.push_back(entity);
        }
    }

    for (Entity* entity : to_render_first) {
        mvaddch(
                entity->GetY(),
                entity->GetX(),
                entity->GetAvatar() | COLOR_PAIR(entity->GetColorPair()));

    }

    for (Entity* entity : to_render_second) {
        mvaddch(
                entity->GetY(),
                entity->GetX(),
                entity->GetAvatar() | COLOR_PAIR(entity->GetColorPair()));

    }
}

void AddLogMessages(Log* log) {
    while (log->GetUnreads() > 0) {
        if (log->GetName() != "Main") {
            hud_messages[hud_message_number++ % kMaxHudMessages] = log->GetName().append(": ").append(log->GetMessage());
        } else {
            hud_messages[hud_message_number++ % kMaxHudMessages] = log->GetMessage();
        }
    }
}
