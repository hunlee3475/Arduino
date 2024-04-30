#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <stdint.h>   /* Standard types */
#include <unistd.h>   /* UNIX standard function definitions */

#ifdef _WIN32
# include <windows.h>
# define SERIALPORT_TERMIOS DCB
# define SERIALPORT_HANDLE HANDLE
# define SERIALPORT_FILE_ERROR INVALID_HANDLE_VALUE
# define SERIALPORT_SLEEP(N) Sleep(N)
#else
# include <termios.h>
# define SERIALPORT_TERMIOS struct termios
# define SERIALPORT_HANDLE int
# define SERIALPORT_FILE_ERROR -1
# define SERIALPORT_SLEEP(N) usleep(N*1e3)
#endif

SERIALPORT_HANDLE serialport_init(const char* serialport, int baud, SERIALPORT_TERMIOS *oldsettings);

int serialport_writebyte(SERIALPORT_HANDLE fd, uint8_t b);

int serialport_readbyte(SERIALPORT_HANDLE fd, char *byte);

int serialport_write(SERIALPORT_HANDLE fd, const char* str);

int serialport_read_until(SERIALPORT_HANDLE fd, char* buf, char until);

int serialport_restoresettings(SERIALPORT_HANDLE fd, SERIALPORT_TERMIOS *oldsettings);


int serialport_close(SERIALPORT_HANDLE fd);

#endif