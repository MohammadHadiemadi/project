#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

// Game constants
#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define MAX_EMAIL 100
#define USER_FILE "users.dat"
#define WIDTH 20    
#define HEIGHT 10    
#define VISION 5   
#define ROOM_WIDTH 14
#define ROOM_HEIGHT 7
#define MAX_ROOMS 6

// Structures
typedef struct {
    char username[MAX_USERNAME];
    char password_hash[MAX_PASSWORD];
    char email[MAX_EMAIL];
} User;

typedef struct {
    int x, y;
} Room;

// Global variables
User users[MAX_USERS];
int user_count = 0;

// Function prototypes
void load_users();
void save_users();
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
void draw_room(int start_x, int start_y, int *door_positions);
void generate_rooms(Room *rooms, int max_rooms, int term_width, int term_height);

// Simple hash function
void simple_hash(const char *input, char *hash_output) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++)) {
        hash = ((hash << 5) + hash) + c;
    }
    sprintf(hash_output, "%lu", hash);
}

// Custom secure random generation
void custom_random_password(char *password, int length) {
    srand(time(NULL));
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
    for(int i = 0; i < length; i++) {
        password[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    password[length] = '\0';
}

// User management functions
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
    for (int i = 0; email[i]; i++) {
        if (email[i] == '@') at_sign++;
        if (at_sign && email[i] == '.') dot_after_at = 1;
    }
    return at_sign == 1 && dot_after_at;
}

void draw_room(int start_x, int start_y, int *door_positions) {
    // Check if door_positions is NULL
    if (!door_positions) {
        static int default_doors[] = {3, 10, 2, 4};
        door_positions = default_doors;
    }
    
    for (int i = 0; i < ROOM_HEIGHT; i++) {
        for (int j = 0; j < ROOM_WIDTH; j++) {
            move(start_y + i, start_x + j);
            if (i == 0 || i == ROOM_HEIGHT - 1) {
                if (j == door_positions[0] || j == door_positions[1]) {
                    addch('+');
                } else {
                    addch('-');
                }
            } else if (j == 0 || j == ROOM_WIDTH - 1) {
                if (i == door_positions[2] || i == door_positions[3]) {
                    addch('+');
                } else {
                    addch('|');
                }
            } else {
                addch('.');
            }
        }
    }
    refresh();
}

void generate_rooms(Room *rooms, int max_rooms, int term_width, int term_height) {
    int room_count = 0;
    int max_attempts = 100;
    int attempts = 0;

    int door_positions[MAX_ROOMS][4];
    
    while (room_count < max_rooms && attempts < max_attempts) {
        int x = (rand() % (term_width - ROOM_WIDTH - 4)) + 2;
        int y = (rand() % (term_height - ROOM_HEIGHT - 4)) + 2;

        bool overlaps = false;
        for (int i = 0; i < room_count; i++) {
            if (abs(x - rooms[i].x) < ROOM_WIDTH + 2 &&
                abs(y - rooms[i].y) < ROOM_HEIGHT + 2) {
                overlaps = true;
                break;
            }
        }

        if (!overlaps) {
            rooms[room_count].x = x;
            rooms[room_count].y = y;

            door_positions[room_count][0] = rand() % (ROOM_WIDTH - 2) + 1;
            door_positions[room_count][1] = rand() % (ROOM_WIDTH - 2) + 1;
            door_positions[room_count][2] = rand() % (ROOM_HEIGHT - 2) + 1;
            door_positions[room_count][3] = rand() % (ROOM_HEIGHT - 2) + 1;
            
            room_count++;
            attempts = 0;
        } else {
            attempts++;
        }
    }

    for (int i = 0; i < room_count; i++) {
        draw_room(rooms[i].x + 1, rooms[i].y + 1, door_positions[i]);

        if (i > 0) {
            int prev_x = rooms[i - 1].x + 1 + door_positions[i - 1][0];
            int prev_y = rooms[i - 1].y + 1 + door_positions[i - 1][3];
            int curr_x = rooms[i].x + 1 + door_positions[i][0];
            int curr_y = rooms[i].y + 1 + door_positions[i][2];

            for (int x = fmin(prev_x, curr_x); x <= fmax(prev_x, curr_x); x++) {
                move(prev_y, x);
                addch('#');
            }

            for (int y = fmin(prev_y, curr_y); y <= fmax(prev_y, curr_y); y++) {
                move(y, curr_x);
                addch('#');
            }
        }
    }
}

void play_game() {
    clear();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    
    int term_height, term_width;
    getmaxyx(stdscr, term_height, term_width);
    
    for(int i = 0; i < term_width; i++) {
        mvaddch(0, i, '='); 
        mvaddch(term_height - 1, i, '='); 
    }
    for(int i = 0; i < term_height; i++) {
        mvaddch(i, 0, '=');  
        mvaddch(i, term_width - 1, '='); 
    }
    
    int box_height = 3;
    int box_width = 20;
    int start_y = 1;  
    int start_x = term_width - box_width - 1; 
    
    WINDOW *score_win = newwin(box_height, box_width, start_y, start_x);
    if (score_win == NULL) {
        mvprintw(0, 0, "Failed to create score window!");
        refresh();
        getch();
        return;
    }
    
    wborder(score_win, '|', '|', '-', '-', '+', '+', '+', '+');
    wattron(score_win, COLOR_PAIR(1));
    mvwprintw(score_win, 1, 2, "YOUR SCORE = 0");
    wattroff(score_win, COLOR_PAIR(1));
    
    Room rooms[MAX_ROOMS];
    generate_rooms(rooms, MAX_ROOMS, term_width - box_width - 2, term_height - 2);
    
    refresh();
    wrefresh(score_win);
    
    getch();
    delwin(score_win);
}

void register_user() {
    User new_user;
    char password[MAX_PASSWORD];
    char hashed_password[MAX_PASSWORD];

    clear();
  
    while (1) {
        mvprintw(1, 1, "Enter username: ");
        echo();
        getstr(new_user.username);
        noecho();

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
        custom_random_password(password, 7);
        mvprintw(10, 1, "Generated password suggestion: %s", password);
        mvprintw(11, 1, "Enter password: ");
     
        echo();
        getstr(password);
        noecho();

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
            strcpy(new_user.password_hash, hashed_password);
            break;
        }
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
           
            echo();
            getstr(password);
            noecho();

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

void game_menu() {
    while (1) {
        clear();
        mvprintw(1, 1, "GAME MENU");
        mvprintw(3, 1, "1. Start New Game");
        mvprintw(4, 1, "2. Load Game");
        mvprintw(5, 1, "3. High Scores");
        mvprintw(6, 1, "4. Exit to Main Menu");
        mvprintw(8, 1, "Select an option: ");
        refresh();
        
        int ch = getch();
        switch (ch) {
            case '1':
                play_game();
                break;
            case '2':
                mvprintw(10, 1, "Loading game...");
                refresh();
                getch();
                break;
            case '3':
                mvprintw(10, 1, "High Scores:");
                refresh();
                getch();
                break;
            case '4':
                return;
        }
    }
}

void main_menu() {
    while (1) {
        clear();
        mvprintw(1, 1, "MAIN MENU");
        mvprintw(3, 1, "1. Login");
        mvprintw(4, 1, "2. Register");
        mvprintw(5, 1, "3. Exit");
        mvprintw(7, 1, "Select an option: ");
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
                endwin();
                exit(0);
        }
    }
}

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    load_users();
    main_menu();
    
    endwin();
    return 0;
}