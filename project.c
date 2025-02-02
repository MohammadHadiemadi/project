// mohammad hadi emadi
// 403 106 346
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include<limits.h>

WINDOW *score_win;
WINDOW *lives_win;
WINDOW *health_win;
int current_weapon = 0;  
int num_high_scores = 0;
int player_lives = 5;
int player_color = 4; 
int player_health = 100 ;
char selected_music[50] = "none";
#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define MAX_EMAIL 100
#define USER_FILE "users.dat"
#define ROOM_WIDTH 14
#define ROOM_HEIGHT 7
#define MAX_ROOMS 6
#define MIN_DISTANCE 3
#define MAX_PILLARS 3
#define MAX_DOORS 2
#define MAP_WIDTH 80   
#define MAP_HEIGHT 24  
#define MAX_MONSTERS 2
#define MAX_HIGH_SCORES 10
#define HIGH_SCORE_FILE "highscores.dat"
#define MAX_NAME_LEN 20
#define MAX_SNAKES 1
time_t paused_time = 0;  
bool is_paused = false;  
typedef struct {
    int x, y;
    bool picked_up;
} Bow;

Bow bow;
int arrows = 5;  
bool has_bow = false;

typedef struct {
    int x, y;
    int room_x, room_y;
    int room_width, room_height;
    bool active;
    int health;
    int move_counter;
} Snake;

Snake snake;
typedef enum {
    EASY = 1,  
    MEDIUM = 2,  
    HARD = 3     
} Difficulty;
 int timedifficulty = 120 ;
Difficulty current_difficulty = EASY;  

int player_x, player_y ;
int GAME_TIME_LIMIT = 60;  

time_t start_time = 0 ;
WINDOW *time_win ;
typedef struct {
    char name[MAX_NAME_LEN];
    int score;
    time_t date;
} HighScore;
HighScore high_scores[MAX_HIGH_SCORES]; 

typedef struct {
    char username[MAX_USERNAME];
    char password_hash[MAX_PASSWORD];
    char email[MAX_EMAIL];
} User;

typedef struct {
    int x, y;
    int pillar_positions[MAX_PILLARS][2];
    int num_pillars;
    bool is_connected;
} Room;
typedef struct { 
    int x, y;              
    int room_x, room_y;   
    int room_width;       
    int room_height;      
    bool active;           
    int move_counter;      
    int health;            
} Monster;
Monster monsters[MAX_MONSTERS];

typedef struct {
    int x;
    int y;
} Player;
typedef struct{
int x ,y ;
bool picked_up ;
}Sword;

Sword sword ;
bool has_sword = false ;


User users[MAX_USERS];
int user_count = 0;
Player player;
int score = 0;

void load_users();
void save_users();
void get_password() ;
int username_exists(const char *username);
int validate_password(const char *password);
int validate_email(const char *email);
void simple_hash(const char *input, char *hash_output);
void custom_random_password(char *password, int length);
void register_user();
int login_user();
void game_menu();
void main_menu();
void play_game();
void place_sword();
void place_bow(Room *rooms, int num_rooms)  ;
void draw_room(Room *room);
void generate_rooms(Room *rooms, int max_rooms, int term_width, int term_height);
void generate_pillars(Room *room);
void move_player(Room *rooms, int num_rooms);
void generate_star_positions(Room *room, int num_stars);
void generate_black_gold(Room *rooms, int num_rooms) ;
void connect_rooms(Room *rooms, int num_rooms);
bool is_path_clear(int x1, int y1, int x2, int y2);
void generate_monsters(Room *rooms, int num_rooms);
void move_monsters();
void generate_snake(Room *rooms, int num_rooms);
void place_next_floor_marker2(Room *rooms, int num_rooms) ;
void move_snake() ;
void game_settings();
void select_music() ;
void handle_game_over(int final_score);
void generate_food(Room *rooms, int num_rooms) ;
void add_high_score(const char *name ,int score) ;
void select_difficulty() ;
void show_weapon_selection() ;
void simple_hash(const char *input, char *hash_output) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++)) {
        hash = ((hash << 5) + hash) + c;
    }
    sprintf(hash_output, "%lu", hash);
}

void custom_random_password(char *password, int length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
    int charset_length = strlen(charset);
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % charset_length];
    }
    password[length] = '\0';
}

void load_users() {
    FILE *file = fopen(USER_FILE, "rb");
    if (file) {
        user_count = fread(users, sizeof(User), MAX_USERS, file);
        fclose(file);
    }
}

void save_users() {
    FILE *file = fopen(USER_FILE, "wb");
    if (file) {
        fwrite(users, sizeof(User), user_count, file);
        fclose(file);
    }
}
int compare_scores(const void *a, const void *b) {
    return ((HighScore *)b)->score - ((HighScore *)a)->score;
}

int username_exists(const char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int validate_password(const char *password) {
    int len = strlen(password);
    int has_digit = 0, has_upper = 0, has_lower = 0;

    if (len < 7) return 0;

    for (int i = 0; password[i]; i++) {
        if (isdigit(password[i])) has_digit = 1;
        if (isupper(password[i])) has_upper = 1;
        if (islower(password[i])) has_lower = 1;
    }

    return has_digit && has_upper && has_lower;
}

int validate_email(const char *email) {
    int at_sign = 0, dot_after_at = 0;
    int len = strlen(email);
    
    if (len < 5) return 0;  
    
    for (int i = 0; email[i]; i++) {
        if (email[i] == '@') at_sign++;
        if (at_sign && email[i] == '.') dot_after_at = 1;
    }
    return at_sign == 1 && dot_after_at;
}

bool is_path_clear(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int x = x1;
    int y = y1;
    int n = 1 + dx + dy;
    int x_inc = (x2 > x1) ? 1 : -1;
    int y_inc = (y2 > y1) ? 1 : -1;
    int error = dx - dy;
    dx *= 2;
    dy *= 2;

    for (; n > 0; --n) {
        char current = mvinch(y, x) & A_CHARTEXT;
        if (current != ' ' && current != '.' && current != '#') {
            return false;
        }
        if (error > 0) {
            x += x_inc;
            error -= dy;
        } else {
            y += y_inc;
            error += dx;
        }
    }
    return true;
}
void get_password(char *password, int max_length) {
    int i = 0;
    char ch;
    while (i < max_length - 1) {
        ch = getch();  
        if (ch == '\n' || ch == '\r') break;  
        if (ch == 127 || ch == 8) { 
            if (i > 0) {
                i--;
                mvprintw(3, 15 + i, " ");  
                move(3, 1 + i);
                refresh();
            }
        } else {
            password[i++] = ch;
            mvprintw(3, 15 + i, "*");  
            refresh();
        }
    }
    password[i] = '\0';
}
void generate_star_positions(Room *room, int num_stars) {
    for (int i = 0; i < num_stars; i++) {
        int x, y;
        do {
            x = (rand() % (ROOM_WIDTH - 4)) + 2;
            y = (rand() % (ROOM_HEIGHT - 4)) + 2;
        } while (mvinch(room->y + y, room->x + x) != '.');
        attron(COLOR_PAIR(2));
        mvaddch(room->y + y, room->x + x, '*');
        attroff(COLOR_PAIR(2));
    }
}
void select_music() { 
    int choice; 
    while (1) { 
        clear(); 
        mvprintw(1, 1, "Select Background Music:"); 
        mvprintw(3, 1, "1. GOD FATHER "); 
        mvprintw(4, 1, "2. REZA GOLZAR "); 
        mvprintw(5, 1, "3. SASY "); 
        mvprintw(6, 1, "4. No Music"); 
        mvprintw(8, 1, "Choose (1-4): "); 
        refresh(); 
 
        choice = getch() - '0'; 
 
        switch (choice) { 
            case 1: 
                strcpy(selected_music, "song_a.mp3"); 
                break; 
            case 2: 
                strcpy(selected_music, "song_b.mp3"); 
                break; 
            case 3: 
                strcpy(selected_music, "song_c.mp3"); 
                break; 
            case 4: 
                strcpy(selected_music, "none"); 
                break; 
            default: 
                continue; 
        } 
        break; 
    } 
}
void generate_black_gold(Room *rooms, int num_rooms) {
    for (int i = 0; i < num_rooms; i++) {
        for (int j = 0; j < 2; j++) { 
            int x, y;
            do {
                x = (rand() % (ROOM_WIDTH - 4)) + 2;
                y = (rand() % (ROOM_HEIGHT - 4)) + 2;
            } while (mvinch(rooms[i].y + y, rooms[i].x + x) != '.');

            attron(COLOR_PAIR(8)); 
            mvaddch(rooms[i].y + y, rooms[i].x + x, '*');
            attroff(COLOR_PAIR(8));
        }
    }
    refresh();
}
void generate_pillars(Room *room) {
    room->num_pillars = (rand() % MAX_PILLARS) + 1;
    
    for (int i = 0; i < room->num_pillars; i++) {
        room->pillar_positions[i][0] = (rand() % (ROOM_WIDTH - 4)) + 2;
        room->pillar_positions[i][1] = (rand() % (ROOM_HEIGHT - 4)) + 2;
        
        for (int j = 0; j < i; j++) {
            if (room->pillar_positions[i][0] == room->pillar_positions[j][0] &&
                room->pillar_positions[i][1] == room->pillar_positions[j][1]) {
                i--;
                break;
            }
        }
    }
}

void draw_room(Room *room) {
    int start_x = room->x;
    int start_y = room->y;
    
    for (int x = 0; x < ROOM_WIDTH; x++) {
        mvaddch(start_y, start_x + x, '-');
        mvaddch(start_y + ROOM_HEIGHT - 1, start_x + x, '-');
    }
    
    for (int y = 0; y < ROOM_HEIGHT; y++) {
        mvaddch(start_y + y, start_x, '|');
        mvaddch(start_y + y, start_x + ROOM_WIDTH - 1, '|');
    }
    
    for (int y = 1; y < ROOM_HEIGHT - 1; y++) {
        for (int x = 1; x < ROOM_WIDTH - 1; x++) {
            mvaddch(start_y + y, start_x + x, '.');
        }
    }
    
    for (int i = 0; i < room->num_pillars; i++) {
        mvaddch(start_y + room->pillar_positions[i][1],
                start_x + room->pillar_positions[i][0], 'o');
    }
}
bool is_inside_room(int x, int y, Room *rooms, int num_rooms) {
    for (int i = 0; i < num_rooms; i++) {
        if (x > rooms[i].x && x < rooms[i].x + ROOM_WIDTH - 1 &&
            y > rooms[i].y && y < rooms[i].y + ROOM_HEIGHT - 1) {
            return true;
        }
    }
    return false;
} 
bool is_on_wall(int x, int y, Room *rooms, int num_rooms) {
    for (int i = 0; i < num_rooms; i++) {
        
        if (y == rooms[i].y || y == rooms[i].y + ROOM_HEIGHT - 1) {
            if (x >= rooms[i].x && x <= rooms[i].x + ROOM_WIDTH - 1) {
                return true;
            }
        }
    
        if (x == rooms[i].x || x == rooms[i].x + ROOM_WIDTH - 1) {
            if (y >= rooms[i].y && y <= rooms[i].y + ROOM_HEIGHT - 1) {
                return true;
            }
        }
    }
    return false;
}

void connect_rooms(Room *rooms, int num_rooms) {
    for (int i = 0; i < num_rooms - 1; i++) {
        
        int x1 = rooms[i].x + (ROOM_WIDTH / 2);
        int y1 = rooms[i].y + (ROOM_HEIGHT / 2);
        int x2 = rooms[i + 1].x + (ROOM_WIDTH / 2);
        int y2 = rooms[i + 1].y + (ROOM_HEIGHT / 2);
        
        
        int wall_crossings = 0;
        bool path_valid = true;
        
        
        int x = x1;
        int x_inc = (x2 > x1) ? 1 : -1;
        
        
        int temp_x = x1;
        while (temp_x != x2 && path_valid) {
            char current = mvinch(y1, temp_x) & A_CHARTEXT;
            if (current == '|' || current == '-') {
                wall_crossings++;
                if (wall_crossings > 2) { 
                    path_valid = false;
                }
            }
            temp_x += x_inc;
        }
        
        int temp_y = y1;
        int y_inc = (y2 > y1) ? 1 : -1;
        while (temp_y != y2 && path_valid) {
            char current = mvinch(temp_y, x2) & A_CHARTEXT;
            if (current == '|' || current == '-') {
                wall_crossings++;
                if (wall_crossings > 2) {
                    path_valid = false;
                }
            }
            temp_y += y_inc;
        }
        
        if (path_valid) {
            
            x = x1;
            while (x != x2) {
                char current = mvinch(y1, x) & A_CHARTEXT;
                if (current == '|' || current == '-') {
                    attron(COLOR_PAIR(1));
                    mvaddch(y1, x, '+');
                    attroff(COLOR_PAIR(1));
                } else if (current != 'o') {
                    attron(COLOR_PAIR(1));
                    mvaddch(y1, x, '#');
                    attroff(COLOR_PAIR(1));
                }
                x += x_inc;
            }
        
            int y = y1;
            while (y != y2) {
                char current = mvinch(y, x2) & A_CHARTEXT;
                if (current == '|' || current == '-') {
                    attron(COLOR_PAIR(1));
                    mvaddch(y, x2, '+');
                    attroff(COLOR_PAIR(1));
                } else if (current != 'o') {
                    attron(COLOR_PAIR(1));
                    mvaddch(y, x2, '#');
                    attroff(COLOR_PAIR(1));
                }
                y += y_inc;
            }
            
            rooms[i].is_connected = true;
            rooms[i + 1].is_connected = true;
        } else {
            
            wall_crossings = 0;
            path_valid = true;
            
            
            temp_y = y1;
            while (temp_y != y2 && path_valid) {
                char current = mvinch(temp_y, x1) & A_CHARTEXT;
                if (current == '|' || current == '-') {
                    wall_crossings++;
                    if (wall_crossings > 2) {
                        path_valid = false;
                    }
                }
                temp_y += y_inc;
            }
            
            temp_x = x1;
            while (temp_x != x2 && path_valid) {
                char current = mvinch(y2, temp_x) & A_CHARTEXT;
                if (current == '|' || current == '-') {
                    wall_crossings++;
                    if (wall_crossings > 2) {
                        path_valid = false;
                    }
                }
                temp_x += x_inc;
            }
            
            if (path_valid) {
                
                int y = y1;
                while (y != y2) {
                    char current = mvinch(y, x1) & A_CHARTEXT;
                    if (current == '|' || current == '-') {
                        attron(COLOR_PAIR(1));
                        mvaddch(y, x1, '+');
                        attroff(COLOR_PAIR(1));
                    } else if (current != 'o') {
                        attron(COLOR_PAIR(1));
                        mvaddch(y, x1, '#');
                        attroff(COLOR_PAIR(1));
                    }
                    y += y_inc;
                }
                
                x = x1;
                while (x != x2) {
                    char current = mvinch(y2, x) & A_CHARTEXT;
                    if (current == '|' || current == '-') {
                        attron(COLOR_PAIR(1));
                        mvaddch(y2, x, '+');
                        attroff(COLOR_PAIR(1));
                    } else if (current != 'o') {
                        attron(COLOR_PAIR(1));
                        mvaddch(y2, x, '#');
                        attroff(COLOR_PAIR(1));
                    }
                    x += x_inc;
                }
                
                rooms[i].is_connected = true;
                rooms[i + 1].is_connected = true;
            }
        }

    }
    
   
    refresh();
}
void clear_paths(Room *rooms, int num_rooms) {
    
    for (int i = 0; i < num_rooms; i++) {
        
        for (int y = rooms[i].y + 1; y < rooms[i].y + ROOM_HEIGHT - 1; y++) {
            for (int x = rooms[i].x + 1; x < rooms[i].x + ROOM_WIDTH - 1; x++) {
                char current = mvinch(y, x) & A_CHARTEXT;
                
                if (current == '#') {
                    mvaddch(y, x, '.');
                }
            }
        }
    }
    refresh();
}
void generate_rooms(Room *rooms, int max_rooms, int term_width, int term_height) {
    int room_count = 0;
    int max_attempts = 1000;
    int attempts = 0;
    
    clear();
    

    for (int i = 0; i < term_width; i++) {
        mvaddch(0, i, '=');
        mvaddch(term_height - 1, i, '=');
    }
    for (int i = 0; i < term_height; i++) {
        mvaddch(i, 0, '=');
        mvaddch(i, term_width - 1, '=');
    }
    
    while (room_count < max_rooms && attempts < max_attempts) {
        Room new_room;
        new_room.x = (rand() % (term_width - ROOM_WIDTH - 4)) + 2;
        new_room.y = (rand() % (term_height - ROOM_HEIGHT - 4)) + 2;
        new_room.is_connected = false;
        
        bool overlaps = false;
        for (int i = 0; i < room_count; i++) {
            if (abs(new_room.x - rooms[i].x) < ROOM_WIDTH + MIN_DISTANCE &&
                abs(new_room.y - rooms[i].y) < ROOM_HEIGHT + MIN_DISTANCE) {
                overlaps = true;
                break;
            }
        }
        
        if (!overlaps) {
            generate_pillars(&new_room);
            rooms[room_count] = new_room;
            draw_room(&rooms[room_count]);
            generate_star_positions(&rooms[room_count], (rand() % 3) + 1);
            room_count++;
            attempts = 0;
        } else {
            attempts++;
        }
        connect_rooms(rooms, room_count);


        clear_paths(rooms, room_count) ;
        refresh();
    }
    
  

}
void place_next_floor_marker(Room *rooms, int num_rooms) {
    int room_index = rand() % num_rooms;  
    int x = (rand() % (ROOM_WIDTH - 4)) + 2;
    int y = (rand() % (ROOM_HEIGHT - 4)) + 2;

    mvaddch(rooms[room_index].y + y, rooms[room_index].x + x, 'F'); 
    refresh();
}
void generate_life_item(Room *rooms, int num_rooms) {
    int room_index = rand() % num_rooms;  
    int x = (rand() % (ROOM_WIDTH - 4)) + 2;
    int y = (rand() % (ROOM_HEIGHT - 4)) + 2;

    mvaddch(rooms[room_index].y + y, rooms[room_index].x + x, 'L'); 
    refresh();
}
typedef struct {
    int x, y;
} Trap;

#define MAX_TRAPS 2
Trap traps[MAX_TRAPS]; 
void generate_traps(Room *rooms, int num_rooms) {
    for (int i = 0; i < MAX_TRAPS; i++) { 
        int room_index = rand() % num_rooms;  
        int x = (rand() % (ROOM_WIDTH - 4)) + 2;
        int y = (rand() % (ROOM_HEIGHT - 4)) + 2;

        traps[i].x = rooms[room_index].x + x;
        traps[i].y = rooms[room_index].y + y;

        mvaddch(traps[i].y, traps[i].x, '.'); 
    }
    refresh();
}
void place_sword(Room *rooms, int num_rooms) { 
    int room_index = rand() % num_rooms;  
    int x, y; 
    
    do { 
        x = (rand() % (ROOM_WIDTH - 4)) + 2; 
        y = (rand() % (ROOM_HEIGHT - 4)) + 2; 
    } while (mvinch(rooms[room_index].y + y, rooms[room_index].x + x) != '.'); 
 
    sword.x = rooms[room_index].x + x; 
    sword.y = rooms[room_index].y + y; 
    sword.picked_up = false; 
 
    attron(COLOR_PAIR(4)); 
    mvaddch(sword.y, sword.x, 'g'); 
    attroff(COLOR_PAIR(4)); 
 
    refresh(); 
}
void place_bow(Room *rooms, int num_rooms) { 
    int room_index = rand() % num_rooms;  
    int x, y; 
 
    do { 
        x = (rand() % (ROOM_WIDTH - 4)) + 2; 
        y = (rand() % (ROOM_HEIGHT - 4)) + 2; 
    } while (mvinch(rooms[room_index].y + y, rooms[room_index].x + x) != '.'); 
 
    bow.x = rooms[room_index].x + x; 
    bow.y = rooms[room_index].y + y; 
    bow.picked_up = false; 
 
    attron(COLOR_PAIR(4)); 
    mvaddch(bow.y, bow.x, 'A'); 
    attroff(COLOR_PAIR(4)); 
 
    refresh(); 
}


void generate_snake(Room *rooms, int num_rooms) {
    int room_index = rand() % num_rooms;
    Room *current_room = &rooms[room_index];
    
    snake.room_x = current_room->x;
    snake.room_y = current_room->y;
    snake.room_width = ROOM_WIDTH;
    snake.room_height = ROOM_HEIGHT;
    
    do {
        snake.x = snake.room_x + (rand() % (ROOM_WIDTH - 2)) + 1;
        snake.y = snake.room_y + (rand() % (ROOM_HEIGHT - 2)) + 1;
    } while (mvinch(snake.y, snake.x) != '.');
    
    snake.active = true;
    snake.health = 5;
    snake.move_counter = 0;
    
    attron(COLOR_PAIR(5));
    mvaddch(snake.y, snake.x, 's');
    attroff(COLOR_PAIR(5));
    refresh();
}


void move_snake() {
    if (!snake.active) return;
    
    snake.move_counter++;
    if (snake.move_counter % 2 != 0) return; 
    
    int dx = (player.x > snake.x) ? 1 : (player.x < snake.x ? -1 : 0);
    int dy = (player.y > snake.y) ? 1 : (player.y < snake.y ? -1 : 0);
    
    int new_x = snake.x + dx;
    int new_y = snake.y + dy;
    
    char next_char = mvinch(new_y, new_x) & A_CHARTEXT;
    if (next_char == '.' || next_char == '*'|| next_char == '#') {
        mvaddch(snake.y, snake.x, '.');
        snake.x = new_x;
        snake.y = new_y;
        attron(COLOR_PAIR(5));
        mvaddch(snake.y, snake.x, 's');
        attroff(COLOR_PAIR(5));
    }
    
    if (abs(player.x - snake.x) <= 1 && abs(player.y - snake.y) <= 1) {
        player_lives--;
        mvwprintw(lives_win, 1, 2, "Lives: %d", player_lives);

       if (player_lives <= 0) {
       handle_game_over(score);
        score = 0;
       player_lives = 5;
        return;
       }
            
        wrefresh(lives_win);
    }
    refresh();
}

void check_snake_activation(Room *rooms, int num_rooms) {
    for (int i = 0; i < num_rooms; i++) {
        if (player.x > rooms[i].x && player.x < rooms[i].x + ROOM_WIDTH - 1 &&
            player.y > rooms[i].y && player.y < rooms[i].y + ROOM_HEIGHT - 1) {
            snake.active = true;
        }
    }
}
void generate_food(Room *rooms, int num_rooms) { 
    for (int i = 0; i < 3; i++) { 
        int room_index = rand() % num_rooms; 
        int x, y; 
 
        do { 
            x = (rand() % (ROOM_WIDTH - 4)) + 2; 
            y = (rand() % (ROOM_HEIGHT - 4)) + 2; 
        } while (mvinch(rooms[room_index].y + y, rooms[room_index].x + x) != '.');  
 
        attron(COLOR_PAIR(3));
        mvaddch(rooms[room_index].y + y, rooms[room_index].x + x, '$'); 
        attroff(COLOR_PAIR(3)); 
    } 
    refresh(); 
}
void show_weapon_selection() {
    WINDOW *weapon_win = newwin(8, 40, LINES / 2 - 4, COLS / 2 - 20);
    box(weapon_win, 0, 0);
    mvwprintw(weapon_win, 1, 2, "Select your weapon:");

    if (has_sword) {
        mvwprintw(weapon_win, 3, 2, "1. Sword (Available)");
    } else {
        mvwprintw(weapon_win, 3, 2, "1. Sword (Not Owned)");
    }
    
    if (has_bow) {
        mvwprintw(weapon_win, 4, 2, "2. Bow (Available)");
    } else {
        mvwprintw(weapon_win, 4, 2, "2. Bow (Not Owned)");
    }

    mvwprintw(weapon_win, 6, 2, "Press 1 or 2 to select, or q to exit");
    wrefresh(weapon_win);

    int choice;
    while (1) {
        choice = getch();
        
        if (choice == '1' && has_sword) {
            current_weapon = 1;  
            mvprintw(LINES - 2, 2, "You have selected the Sword!");
            break;
        } else if (choice == '2' && has_bow) {
            current_weapon = 2;  
            mvprintw(LINES - 2, 2, "You have selected the Bow!");
            break;
        } else if (choice == '1' && !has_sword) {
            mvprintw(LINES - 2, 2, "Error: You don't have a Sword!");
            refresh();
        } else if (choice == '2' && !has_bow) {
            mvprintw(LINES - 2, 2, "Error: You don't have a Bow!");
            refresh();
        } else if (choice == 'q') {
            break;
        }
    }

    delwin(weapon_win);
    touchwin(stdscr);
    refresh();
}
void place_next_floor_marker2(Room *rooms, int num_rooms) {
    
    int start_room_index = 0;
    int x, y;
    
    do {
        x = (rand() % (ROOM_WIDTH - 4)) + 2;
        y = (rand() % (ROOM_HEIGHT - 4)) + 2;
    } while (mvinch(rooms[start_room_index].y + y, rooms[start_room_index].x + x) != '.');

    mvaddch(rooms[start_room_index].y + y, rooms[start_room_index].x + x, 'F');
    
    for (int i = 1; i < num_rooms; i++) {
        if (!rooms[i].is_connected) {  
            do {
                x = (rand() % (ROOM_WIDTH - 4)) + 2;
                y = (rand() % (ROOM_HEIGHT - 4)) + 2;
            } while (mvinch(rooms[i].y + y, rooms[i].x + x) != '.');

            mvaddch(rooms[i].y + y, rooms[i].x + x, 'F');
        }
    }

    refresh();
}


void move_player(Room *rooms, int num_rooms) {
    int input;
    WINDOW *score_win = newwin(3, 20, 1, COLS - 22);
    WINDOW *lives_win = newwin(3, 20, 4, COLS - 22); 
    WINDOW *pause_info_win = newwin(3, 20, 7, COLS - 22); 
    player.x = rooms[0].x + 2;
    player.y = rooms[0].y + 2;
    attron(COLOR_PAIR(player_color));
    mvaddch(player.y, player.x, '@');
    attroff(COLOR_PAIR(player_color));

    box(score_win, 0, 0);
    box(lives_win, 0, 0);
     box(pause_info_win, 0, 0);
    mvwprintw(score_win, 1, 2, "Score: %d", score);
    mvwprintw(lives_win, 1, 2, "Lives: %d", player_lives);
    mvwprintw(pause_info_win, 1, 2, "Press 's' pause");
    wrefresh(score_win);
    wrefresh(lives_win);
    wrefresh(pause_info_win);
    
    int monster_move_counter = 0;

    while (1) {
             input = getch();

            if (input == ERR) {  
                continue;  
            }
            if (input == 'a' && current_weapon == 1) {  
    for (int i = 0; i < MAX_MONSTERS; i++) { 
        if (monsters[i].active && abs(monsters[i].x - player.x) <= 1 && abs(monsters[i].y - player.y) <= 1) { 
            monsters[i].health--;  
            if (monsters[i].health <= 0) { 
                monsters[i].active = false; 
                mvaddch(monsters[i].y, monsters[i].x, '.');
            } 
        } 
    } 
    if (abs(player.x - snake.x) <= 1 && abs(player.y - snake.y) <= 1) { 
        snake.health--;  
        if (snake.health <= 0) { 
            snake.active = false;
            mvaddch(snake.y, snake.x, '.');
        } 
    } 
}

if (input == 'p' && current_weapon == 2) {  
    if (has_bow && arrows > 0) {
        arrows--;
        int start_x = player.x - 4, end_x = player.x + 4;
        int start_y = player.y - 4, end_y = player.y + 4;

        for (int i = 0; i < MAX_MONSTERS; i++) {
            if (monsters[i].active && monsters[i].x >= start_x && monsters[i].x <= end_x &&
                monsters[i].y >= start_y && monsters[i].y <= end_y) { 
                monsters[i].health--;
                if (monsters[i].health <= 0) {
                    monsters[i].active = false;
                    mvaddch(monsters[i].y, monsters[i].x, '.');
                }
            }
        }

        if (snake.active && snake.x >= start_x && snake.x <= end_x &&
            snake.y >= start_y && snake.y <= end_y) {
            snake.health--;
            if (snake.health <= 0) {
                snake.active = false;
                mvaddch(snake.y, snake.x, '.');
            }
        }

        WINDOW *arrow_win = newwin(3, 16, 21, COLS - 22); 
        box(arrow_win, 0, 0);
        mvwprintw(arrow_win, 1, 2, "Arrows left: %d", arrows);
        wrefresh(arrow_win);
    }
}

            if (input == 'i') {
                if (has_sword || has_bow) {
                    show_weapon_selection();
                } else {
                    mvprintw(LINES - 2, 2, "Error: You don't have enoghf weapon!");
                    refresh();
                }
                continue;
            }
            if (input == 'q') {  
                handle_game_over(score);
                return;
            }
            if (input == 's') {
            
            is_paused = true ;
            paused_time = time(NULL) - start_time ;
            WINDOW *pause_win = newwin(10, 40, LINES/2 - 5, COLS/2 - 20);
            box(pause_win, 0, 0);
            
            mvwprintw(pause_win, 2, 13, "GAME PAUSED");
            mvwprintw(pause_win, 4, 2, "Current Score: %d", score);
            mvwprintw(pause_win, 6, 2, "Press 'c' to continue");
            mvwprintw(pause_win, 7, 2, "Press 'f' to finish game");
            wrefresh(pause_win);

            int pause_input;
            while ((pause_input = getch())) {
                if (pause_input == 'c') {
                       start_time = time(NULL) - paused_time;
                        is_paused = false;
                        delwin(pause_win);
                        touchwin(stdscr);
                        refresh();
                        break;
                } else if (pause_input == 'f') {
                
                    handle_game_over(score);
                    score = 0;
                    player_lives = 5;
                    delwin(score_win);
                    delwin(lives_win);
                    delwin(pause_info_win);
                    delwin(time_win);
                    return;
                }
            }
            continue;
        }
          if (!is_paused) {  
    time_t current_time = time(NULL); 
    int elapsed_time = (int)(current_time - start_time); 
    int remaining_time = GAME_TIME_LIMIT - elapsed_time; 
    
    mvwprintw(time_win, 1, 2, "Time: %d s", remaining_time); 
    wrefresh(time_win); 
    
    if (remaining_time <= 0) { 
        handle_game_over(score); 
        score = 0; 
        player_lives = 5; 
        return; 
    }
}

                    int new_x = player.x;
                    int new_y = player.y;

        switch (input) {
            case '8': new_y--; break; 
            case '2': new_y++; break; 
            case '4': new_x--; break; 
            case '6': new_x++; break;         
    
            default: continue;
        }
                    wrefresh(score_win);
                    wrefresh(lives_win);
                    wrefresh(health_win);
                    wrefresh(time_win);



                    char next_char = mvinch(new_y, new_x) & A_CHARTEXT;
                     if (next_char == '^') {
                    score += 5; 
                    mvwprintw(score_win, 1, 2, "Score: %d", score);
                    wrefresh(score_win);
                 }
                    if (next_char == '$') { 
                        player_health += 15; 
                        if (player_health > 100) player_health = 100; 
                        mvwprintw(health_win, 1, 2, "Health: %d%%", player_health); 
                        wrefresh(health_win); 
                    }

                    if (input == 'a' && has_sword) {  
                for (int i = 0; i < MAX_MONSTERS; i++) { 
                    if (!monsters[i].active) continue; 
            
                    
                    if (abs(player.x - monsters[i].x) <= 1 && abs(player.y - monsters[i].y) <= 1) { 
                        monsters[i].health--;
                        mvprintw(LINES - 2, 2, "Hit monster! Health left: %d", monsters[i].health); 
                        refresh(); 
                        
                        if (monsters[i].health <= 0) { 
                            mvaddch(monsters[i].y, monsters[i].x, '.');  
                            monsters[i].active = false; 
                        } 
                    } 
                } 
            } 
        if (next_char == '.' || next_char == '#' || next_char == '*' || 
            next_char == '+' || next_char == 'F' || next_char == 'L' || next_char == 'T' || next_char == 'g' || next_char == '^'|| next_char == '$'|| next_char == 'A') {
            
            char prev_char = mvinch(player.y, player.x) & A_CHARTEXT;
            mvaddch(player.y, player.x, (prev_char == '#' ? '#' : '.'));

            player.x = new_x;
            player.y = new_y;

            static int move_counter = 0;
            static int zero_health_moves = 0 ; 
            move_counter++; 
            if (move_counter % 13 == 0) { 
                player_health -= 5; 
            if (player_health < 0) {
                 player_health = 0; 
                zero_health_moves++ ;
                if(zero_health_moves % 10 == 0){
                    player_lives-- ;
                    mvwprintw(lives_win,1,2, "Lives: %d",player_lives) ;
                    wrefresh(lives_win) ;
                
                if(player_lives <= 0){
                    handle_game_over(score) ;
                    score = 0 ;
                    player_lives = 5;
                    return ;
                }
            }
         } else {
            zero_health_moves = 0 ;
            }      
                mvwprintw(health_win, 1, 2, "Health: %d%%", player_health); 
                wrefresh(health_win); 
            }
            if (player.x == bow.x && player.y == bow.y && !bow.picked_up) { 
                bow.picked_up = true; 
                has_bow = true; 
                arrows = 5;  

                WINDOW *bow_win = newwin(3, 16, 19, COLS - 22); 
                box(bow_win, 0, 0); 
                wattron(bow_win, COLOR_PAIR(4)); 
                mvwprintw(bow_win, 1, 2, "Bow picked up"); 
                wattroff(bow_win, COLOR_PAIR(4)); 
                wrefresh(bow_win); 
            }

            if (next_char == '*') {
                score += 10;
                mvwprintw(score_win, 1, 2, "Score: %d", score);
                wrefresh(score_win);
            } else if (next_char == 'F') {
                mvprintw(0, 0, "NOW YOU ARE IN NEXT FLOOR");
                refresh();
                napms(1000);
                clear();
                play_game();
                return;
            } else if (next_char == 'L') {
                if(player_lives < 5){
                player_lives++;
                }
                mvwprintw(lives_win, 1, 2, "Lives: %d", player_lives);
                wrefresh(lives_win);
            }
            if (player.x == sword.x && player.y == sword.y && !sword.picked_up) { 
                sword.picked_up = true; 
                has_sword = true; 
            
                WINDOW *sword_win = newwin(3, 20, 16, COLS - 22); 
                box(sword_win, 0, 0); 
                wattron(sword_win, COLOR_PAIR(4));
                mvwprintw(sword_win, 1, 2, "sword pick up"); 
                wattroff(sword_win, COLOR_PAIR(4)); 
                wrefresh(sword_win); 
            }
            check_snake_activation(rooms, num_rooms  ) ;
            
            attron(COLOR_PAIR(player_color));
            mvaddch(player.y, player.x, '@');
            attroff(COLOR_PAIR(player_color));


            move_snake() ;
            refresh() ;
            
            monster_move_counter++;
            if (monster_move_counter % 3 == 0) { 
                for (int i = 0; i < MAX_MONSTERS; i++) {
                    if (!monsters[i].active) continue;

                    int dx = (rand() % 3) - 1;
                    int dy = (rand() % 3) - 1;
                    
                    int new_monster_x = monsters[i].x + dx;
                    int new_monster_y = monsters[i].y + dy;

                    if (new_monster_x > monsters[i].room_x && 
                        new_monster_x < monsters[i].room_x + monsters[i].room_width - 1 &&
                        new_monster_y > monsters[i].room_y && 
                        new_monster_y < monsters[i].room_y + monsters[i].room_height - 1) {
                        
                        char monster_next = mvinch(new_monster_y, new_monster_x) & A_CHARTEXT;
                        if (monster_next == '.') {
                            
                            mvaddch(monsters[i].y, monsters[i].x, '.');
                            
                            monsters[i].x = new_monster_x;
                            monsters[i].y = new_monster_y;
                            
                            attron(COLOR_PAIR(1));
                            mvaddch(monsters[i].y, monsters[i].x, 'D');
                            attroff(COLOR_PAIR(1));
                            for (int i = 0; i < MAX_MONSTERS; i++) {
                                        if (!monsters[i].active) continue;

                                int monster_x = monsters[i].x;
                                int monster_y = monsters[i].y;

                                if ((abs(player.x - monster_x) <= 1) && (abs(player.y - monster_y) <= 1)) {
                                    player_lives--; 
                                    mvwprintw(lives_win, 1, 2, "Lives: %d", player_lives);
                                    wrefresh(lives_win);
                                    mvprintw(LINES - 3, 2, "A monster is near! You lost 1 life!");
                                    refresh();

                                    if (player_lives <= 0) {
                                        handle_game_over(score);
                                        score = 0;
                                        player_lives = 5;
                                        return;
                                    }
                                    break;
                                }
                            }
                            
                            refresh();
                         
                        }
                    }
                }
            }

            refresh();
        }
        for (int i = 0; i < MAX_TRAPS; i++) {
            if (new_x == traps[i].x && new_y == traps[i].y) {
                player_lives--;
                mvwprintw(lives_win, 1, 2, "Lives: %d", player_lives);
                wrefresh(lives_win);
                mvprintw(LINES - 3, 2, "You hit a trap! Be careful!");
                refresh();
                break;
                 if (player_lives <= 0) {
                       handle_game_over(score);
                         score = 0;
                        player_lives = 5;
                   return;
                 } 
            }
        }
    }

    delwin(score_win);
    delwin(lives_win);
    delwin(time_win) ;
    delwin(pause_info_win) ;
    delwin(health_win) ;
} 


void change_player_color() {
    int colors[] = {4, 5, 6, 7, 8, 9}; 
    int num_colors = 6;
    int current_selection = 0;

    while (1) {
        clear();
        mvprintw(1, 1, "Select Player Color:");

        for (int i = 0; i < num_colors; i++) {
            if (i == current_selection) {
                attron(A_REVERSE); 
            }
            mvprintw(3 + i, 5, "Color %d", i + 1);
            attroff(A_REVERSE);
        }

        refresh();
        int ch = getch();
        if (ch == KEY_UP && current_selection > 0) {
            current_selection--;
        } else if (ch == KEY_DOWN && current_selection < num_colors - 1) {
            current_selection++;
        } else if (ch == '\n') { 
            player_color = colors[current_selection]; 
            return;
        }
    }
}

void register_user() {
    User new_user;
    char password[MAX_PASSWORD];
    char hashed_password[MAX_PASSWORD];

    clear();

    while (1) {
        mvprintw(1, 1, "Enter username (max %d chars): ", MAX_USERNAME - 1);
        echo();
        getstr(new_user.username);
        noecho();

        if (strlen(new_user.username) >= MAX_USERNAME) {
          mvprintw(3, 1, "Username too long. Try again.");
            refresh();
            getch();
            move(1, 20);
            clrtoeol();
            move(3, 1);
            clrtoeol();
            continue;
        }

        if (username_exists(new_user.username)) {
            mvprintw(3, 1, "Username already exists. Try again.");
            refresh();
            getch();
            move(1, 20);
            clrtoeol();
            move(3, 1);
            clrtoeol();
        } else {
            break;
        }
    }

    while (1) {
        mvprintw(5, 1, "Enter email: ");
        echo();
        getstr(new_user.email);
        noecho();

        if (!validate_email(new_user.email)) {
            mvprintw(7, 1, "Invalid email format. Try again.");
            refresh();
            getch();
            move(5, 20);
            clrtoeol();
            move(7, 1);
            clrtoeol();
        } else {
            break;
        }
    }

    while (1) {
        custom_random_password(password, 12);
        mvprintw(10, 1, "Generated password suggestion: %s", password);
        mvprintw(3, 1, "Enter password: ");
        noecho();
        get_password(password, MAX_PASSWORD);
        if (!validate_password(password)) {
            mvprintw(12, 1, "Invalid password. Must be 7+ chars, include number, uppercase, lowercase.");
            refresh();
            getch();
            move(11, 20);
            clrtoeol();
            move(12, 1);
            clrtoeol();
        } else {
            simple_hash(password, hashed_password);
            strncpy(new_user.password_hash, hashed_password, MAX_PASSWORD - 1);
            new_user.password_hash[MAX_PASSWORD - 1] = '\0';
            break;
        }
    }

    if (user_count >= MAX_USERS) {
        mvprintw(14, 1, "Maximum user limit reached!");
        refresh();
        getch();
        return;
    }

    users[user_count++] = new_user;
    save_users();
    clear();
    mvprintw(5, 1, "Registration Successful!");
    refresh();
    getch();
}

int login_user() {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char hashed_password[MAX_PASSWORD];

    clear();
    mvprintw(1, 1, "Enter username: ");
    echo();
    getstr(username);
    noecho();

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
           mvprintw(3, 1, "Enter password: ");
           noecho();
           get_password(password, MAX_PASSWORD);
            simple_hash(password, hashed_password);
            if (strcmp(users[i].password_hash, hashed_password) == 0) {
                clear();
                mvprintw(5, 1, "Login successful!");
                refresh();
                getch();
                return 1;
            } else {
                mvprintw(5, 1, "Incorrect password!");
                refresh();
                getch();
                return 0;
            }
        }
    }

    mvprintw(3, 1, "User not found!");
    refresh();
    getch();
    return 0;
}

void play_game() {
    if (strcmp(selected_music, "none") != 0) {
        char command[100];
        sprintf(command, "mpg123 -q %s &", selected_music); 
        system(command);
    }
    if(start_time == 0){
        start_time = time(NULL) ;
    }
    if (player_lives <= 0) {
    player_lives = 5; 
    }

    GAME_TIME_LIMIT = 120;
    if(timedifficulty == 60 ){
        GAME_TIME_LIMIT = 60 ;
    } else 
    if(timedifficulty == 90){
        GAME_TIME_LIMIT = 90 ;
    } else
    if (timedifficulty = 120 ){
        GAME_TIME_LIMIT= 120 ;
    } 
    static int saved_score = 0;
    clear();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    int term_height, term_width;
    getmaxyx(stdscr, term_height, term_width);

    if (term_height < ROOM_HEIGHT * 2 || term_width < ROOM_WIDTH * 2) {
        mvprintw(0, 0, "Terminal window too small!");
        refresh();
        getch();
        return;
    }

    int box_height = 3;
    int box_width = 20;
    int start_y = 1;
    int start_x = term_width - box_width - 1;
    WINDOW *weapon_hint = newwin(5, 20, 21, COLS - 22);
    box(weapon_hint, 0, 0);
    mvwprintw(weapon_hint, 1, 2, "Press 'i' to change weapon");
    wrefresh(weapon_hint);

    score_win = newwin(box_height, box_width, start_y, start_x);
    lives_win = newwin(box_height, box_width, start_y + 3, start_x);
    health_win = newwin(3, 20, 13, COLS - 22); 
        box(health_win, 0, 0); 
        mvwprintw(health_win, 1, 2, "Health: %d%%", player_health); 
        wrefresh(health_win);
    if (!score_win || !lives_win) {
        mvprintw(0, 0, "Failed to create score window!");
        refresh();
        getch();
        return;
    }

    box(score_win, 0, 0);
    box(lives_win, 0, 0);
    wattron(score_win, COLOR_PAIR(1));
    wattron(lives_win, COLOR_PAIR(1));
    mvwprintw(score_win, 1, 2, "Score: %d", saved_score);
    mvwprintw(lives_win, 1, 2, "Lives: %d", player_lives);
    wattroff(score_win, COLOR_PAIR(1));
    wattroff(lives_win, COLOR_PAIR(1));
    wrefresh(score_win);
    wrefresh(lives_win);
    time_win = newwin(3,20, 10, COLS -22) ;
    box(time_win, 0,0) ;
    mvwprintw(time_win,1,2,"TIME : %d s",GAME_TIME_LIMIT) ;
    Room rooms[MAX_ROOMS];
    generate_rooms(rooms, MAX_ROOMS, term_width - box_width - 2, term_height - 2);
    place_next_floor_marker(rooms, MAX_ROOMS);
    place_next_floor_marker2(rooms,MAX_ROOMS);
    generate_life_item(rooms, MAX_ROOMS);
    generate_traps(rooms, MAX_ROOMS);
    generate_monsters(rooms, MAX_ROOMS);
    generate_snake(rooms, MAX_ROOMS);
    place_sword(rooms, MAX_ROOMS) ;
    place_bow(rooms, MAX_ROOMS);
    generate_black_gold(rooms,MAX_ROOMS ) ;
    generate_food(rooms , MAX_ROOMS) ;
    mvprintw(term_height - 2, 2, "Use arrow keys to move. Press 'q' to quit.");
    refresh();

    
    move_player(rooms, MAX_ROOMS);
    move_monsters();
    move_snake() ;
    saved_score = score;

    delwin(score_win);
    delwin(lives_win);
}


int select_color() {
    int colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA};
    int num_colors = 6;
    int current_selection = 0;

    while (1) {
        clear();
        mvprintw(1, 1, "Select Player Color:");
        
        for (int i = 0; i < num_colors; i++) {
            if (i == current_selection) {
                attron(A_REVERSE); 
            }
            mvprintw(3 + i, 5, "Color %d", i + 1);
            attroff(A_REVERSE);
        }    
        refresh();
        int ch = getch();
        if (ch == KEY_UP && current_selection > 0) {
            current_selection--;
        } else if (ch == KEY_DOWN && current_selection < num_colors - 1) {
            current_selection++;
        } else if (ch == '\n') {
            init_pair(4, colors[current_selection], COLOR_BLACK); 
            return colors[current_selection];
        }
    }
}
void generate_monsters(Room *rooms, int num_rooms) {
    
    int selected_rooms[MAX_MONSTERS];
    
    selected_rooms[0] = rand() % num_rooms;
    
    
    do {
        selected_rooms[1] = rand() % num_rooms;
    } while (selected_rooms[1] == selected_rooms[0]);


    for (int i = 0; i < MAX_MONSTERS; i++) {
        Room *current_room = &rooms[selected_rooms[i]];
        
        monsters[i].room_x = current_room->x;
        monsters[i].room_y = current_room->y;
        monsters[i].room_width = ROOM_WIDTH;
        monsters[i].room_height = ROOM_HEIGHT;
        
        do {
            monsters[i].x = monsters[i].room_x + (rand() % (ROOM_WIDTH - 2)) + 1;
            monsters[i].y = monsters[i].room_y + (rand() % (ROOM_HEIGHT - 2)) + 1;
        } while (mvinch(monsters[i].y, monsters[i].x) != '.');

        monsters[i].active = true;
        monsters[i].move_counter = 0;
        monsters[i].health = 3 ;
    
        attron(COLOR_PAIR(1));  
        mvaddch(monsters[i].y, monsters[i].x, '.');
        attroff(COLOR_PAIR(1));
    }
    refresh();
}

void move_monsters() {
    
    static int global_counter = 0;
    global_counter++;
    
    if (global_counter % 4 != 0) return;

    for (int i = 0; i < MAX_MONSTERS; i++) {
        if (!monsters[i].active) continue;

    
        int dx = (rand() % 3) - 1; 
        int dy = (rand() % 3) - 1;  
        
        int new_x = monsters[i].x + dx;
        int new_y = monsters[i].y + dy;

        if (new_x <= monsters[i].room_x || 
            new_x >= monsters[i].room_x + monsters[i].room_width - 1 ||
            new_y <= monsters[i].room_y || 
            new_y >= monsters[i].room_y + monsters[i].room_height - 1) {
            continue;  
        }

        char next_char = mvinch(new_y, new_x) & A_CHARTEXT;
        if (next_char == '.') {
            
            mvaddch(monsters[i].y, monsters[i].x, '.');
            
            monsters[i].x = new_x;
            monsters[i].y = new_y;
            
            attron(COLOR_PAIR(1));
            mvaddch(monsters[i].y, monsters[i].x, 'D');
            attroff(COLOR_PAIR(1));

            if (monsters[i].x == player.x && monsters[i].y == player.y) {
                player_lives--;
                mvwprintw(lives_win, 1, 2, "Lives: %d", player_lives);
                wrefresh(lives_win);
                mvprintw(LINES - 3, 2, "A monster hit you! Be careful!");
                refresh();
            }
        }
    }
    refresh();
}
void select_difficulty() {
    int choice;
    while (1) {
        clear();
        mvprintw(1, 1, "Select Difficulty:");
        mvprintw(3, 1, "1. Easy (120 sec)");
        mvprintw(4, 1, "2. Medium (90 sec)");
        mvprintw(5, 1, "3. Hard (60 sec)");
        mvprintw(7, 1, "Choose (1-3): ");
        refresh();




        choice = getch() - '0';
        if (choice >= 1 && choice <= 3) {
            switch(choice) {
                case 1: 
                    timedifficulty = 120 ;
                    break;
                case 2: 
                    timedifficulty = 90 ;
                    break;
                case 3: 
                    timedifficulty = 60 ;
                    break;
            }
            break;
        }
    }
}
void game_settings() {
    while (1) {
        clear();
        mvprintw(1, 1, "GAME SETTINGS");
        mvprintw(3, 1, "1. Change Player Color");
        mvprintw(4, 1, "2. Set Game Difficulty");
        mvprintw(5, 1, "3. select back grand music");
        mvprintw(6, 1, "4. Back to Game Menu");
        mvprintw(7, 1, "Select an option: ");
        refresh();

        int ch = getch();
        switch (ch) {
            case '1':
                change_player_color();
                break;
            case '2':
                select_difficulty();
                break;
            case '3':
                select_music() ;
                break;
            case '4':  
                return;
        }
    }
}
void load_high_scores() {
    FILE *file = fopen(HIGH_SCORE_FILE, "rb");
    if (file) {
        num_high_scores = fread(high_scores, sizeof(HighScore), MAX_HIGH_SCORES, file);
        fclose(file);
    }
}
void save_high_scores() {
    FILE *file = fopen(HIGH_SCORE_FILE, "wb");
    if (file) {
        fwrite(high_scores, sizeof(HighScore), num_high_scores, file);
        fclose(file);
    }
}
void add_high_score(const char *name, int score) {
    if (num_high_scores < MAX_HIGH_SCORES) {
        strncpy(high_scores[num_high_scores].name, name, MAX_NAME_LEN - 1);
        high_scores[num_high_scores].score = score;
        high_scores[num_high_scores].date = time(NULL);
        num_high_scores++;
    } else {
        if (score > high_scores[MAX_HIGH_SCORES - 1].score) {
            strncpy(high_scores[MAX_HIGH_SCORES - 1].name, name, MAX_NAME_LEN - 1);
            high_scores[MAX_HIGH_SCORES - 1].score = score;
            high_scores[MAX_HIGH_SCORES - 1].date = time(NULL);
        }
    }
    qsort(high_scores, num_high_scores, sizeof(HighScore), compare_scores);
    save_high_scores();
}
void show_high_scores() {
    clear();
    mvprintw(1, 1, "HIGH SCORES");
    mvprintw(2, 1, "==================");
    
    for (int i = 0; i < num_high_scores; i++) {
        char date_str[26];
        ctime_r(&high_scores[i].date, date_str);
        date_str[24] = '\0';  
         
        if (i == 0) {
            attron(COLOR_PAIR(2)); 
            mvprintw(4 + i, 1," %d. %-20s %5d  %s", 
                    i + 1, high_scores[i].name, high_scores[i].score, date_str);
            attroff(COLOR_PAIR(2));
        } else if (i == 1) {
            attron(COLOR_PAIR(7));
            mvprintw(4 + i, 1, "%d. %-20s %5d  %s", 
                    i + 1, high_scores[i].name, high_scores[i].score, date_str);
            attroff(COLOR_PAIR(7));
        } else if (i == 2) {
            attron(COLOR_PAIR(3)); 
            mvprintw(4 + i, 1, "%d. %-20s %5d  %s", 
                    i + 1, high_scores[i].name, high_scores[i].score, date_str);
            attroff(COLOR_PAIR(3));
        } else {
            mvprintw(4 + i, 1, "%d. %-20s %5d  %s", 
                    i + 1, high_scores[i].name, high_scores[i].score, date_str);
        }      
    }
    mvprintw(15, 1, "Press 'M' to return to Game Menu...");refresh();
        int key;
  while ((key = getch())) {
     if (key == 'M' || key == 'm') {
      game_menu();      
      return;
    }}
    
}
void handle_game_over(int final_score) {
    clear();
    char player_name[MAX_NAME_LEN];
    
    attron(A_BOLD);
    mvprintw(LINES/2 - 5, COLS/2 - 5, "GAME OVER!");
    attroff(A_BOLD);
    mvprintw(LINES/2 - 3, COLS/2 - 15, "Your final score: %d", final_score);
    
    mvprintw(LINES/2 - 1, COLS/2 - 15, "Enter your name: ");
    echo();
    getstr(player_name);
    noecho();
    
    
    add_high_score(player_name, final_score);
    
    show_high_scores();
    system("pkill -f mpg123") ;
}









void game_menu() {
    while (1) {
        clear();
        mvprintw(1, 1, "GAME MENU");
        mvprintw(3, 1, "1. Start New Game");
        mvprintw(4, 1, "2. Load Game");
        mvprintw(5, 1, "3. High Scores");
        mvprintw(6, 1, "4. Settings");  
        mvprintw(7, 1,  "5. instructions") ;
        mvprintw(8, 1, "6. Exit to Main Menu");
        mvprintw(9, 1, "Select an option: ");
        refresh();

        int ch = getch();
        switch (ch) {
            case '1':
            score = 0;
            player_lives = 5;
            start_time = 0;
            has_sword = false;
            sword.picked_up = false;

            for (int i = 0; i < MAX_MONSTERS; i++) {
                monsters[i].active = false;
            }
                play_game();
                break;
            case '2':
                mvprintw(10, 1, "Loading game...");
                refresh();
                getch();
                break;
            case '3':
                show_high_scores();
                break;
            case '4':  
                game_settings();
                break;
            case '5' :
            clear() ;
            mvprintw(1,1, "instruction") ;
            mvprintw(3,1, "g :  It means a weapon and by pressing the a key when you are around the monster, it damages it to kill it") ;
             mvprintw(5,1, "* :[blue]   This is in the game as black gold and you get five points for getting it") ;
             mvprintw(7,1, "* : [gold]   This is in the game as gold and you get 10 points by getting it") ;
              mvprintw(9,1, "D :  It is in the game as a monster that you can kill and it can also damage you and it has three lives") ;
               mvprintw(11,1, "F :  By standing on this, you go to the next floor of the game") ;
                mvprintw(13,1, "L :  By taking this you add one life to your life") ;
                 mvprintw(15,1, " The duration of the game is 60 seconds or one minute, you can adjust the game difficulty and reduce the duration The game ends when this time expires") ;
                 mvprintw(17,1, "s : It is another monster of the game that has five souls and follows you even in the corridor ") ;
                mvprintw(19,1, "A : It has the role of a bow in the game, and by picking it up, if you are in the four houses around the monsters, your arrow will affect them and reduce their health. ") ;
                refresh() ;
                getch() ;
                break ;            
            case '6':
                return;
        }
    }
}
void main_menu() {
    while (1) {
        clear();
        mvprintw(1, 1, "================================");
        mvprintw(2, 1, "=     [[ WELCOM TO GAME ]]     =");
        mvprintw(3, 1, "================================");
        
        mvprintw(5, 1, "1. Login");
        mvprintw(6, 1, "2. Register");
        mvprintw(7, 1, "3. Play as Guest");  
        mvprintw(8, 1, "4. Exit");
        mvprintw(10, 1, "Select an option: ");
        refresh();

        int ch = getch();
        switch (ch) {
            case '1':
                if (login_user()) {
                    game_menu();
                }
                break;
            case '2':
                register_user();
                break;
            case '3':  
                game_menu() ;
                break;
            case '4':
                clear();
                mvprintw(10, 1, "Thanks for playing!");
                refresh();
                napms(1000);
                endwin();
                exit(0);
        }
    }
}

int main() {
    
    initscr();
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        return 1;
    }
    
    
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
  

    srand(time(NULL));
    

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(3, COLOR_BLUE, COLOR_BLACK);  
    init_pair(4, COLOR_RED, COLOR_BLACK);     
    init_pair(5, COLOR_GREEN, COLOR_BLACK);   
    init_pair(6, COLOR_BLUE, COLOR_BLACK);   
    init_pair(7, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(8, COLOR_CYAN, COLOR_BLACK);   
    init_pair(9, COLOR_MAGENTA, COLOR_BLACK); 
 
    load_users();
    load_high_scores();  
    main_menu();
    endwin();
    return 0;
}