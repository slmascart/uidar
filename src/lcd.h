void write_word(int data, int fd);
void send_command(int command, int fd);
void send_data(int data, int fd);
void clear_lcd(int fd);
void init_lcd(int fd);
void write(int x, int y, char* data, int fd);
