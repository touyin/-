int myserialOpen (const char *device, const int baud);
int serial_Get_char_one(const int fd);

int serialGetString (const int fd,char* s);
int serialDataAvail (const int fd);
