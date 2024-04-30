#include <stdio.h>    /* Standard input/output definitions */
#include <stdlib.h> 
#include <string.h>   /* String function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */

#ifdef _WIN32
# include <windows.h>
#else
# include <termios.h>  /* POSIX terminal control definitions */
# include <sys/ioctl.h>
#endif

#include "SerialPort.h"

int serialport_writebyte(SERIALPORT_HANDLE fd, uint8_t b){
#ifdef _WIN32
    DWORD errors;
    COMSTAT status;
    DWORD n;
    if(!WriteFile((HANDLE)fd, (void *)(&b), 1, &n, NULL)){
        ClearCommError((HANDLE)fd, &errors, &status);
        perror("serialport_write: Error writing to COM port");
        return -1;
    }
#else /* Linux, Mac */
    int n = write(fd,&b,1);
    if( n!=1)
        return -1;
#endif
    return 0;
}

int serialport_write(SERIALPORT_HANDLE fd, const char* str){
    int len = strlen(str);
#ifdef _WIN32
    DWORD n;
    DWORD errors;
    COMSTAT status;
    if(!WriteFile((HANDLE)fd, (void *)str, len, &n, 0)){
        ClearCommError((HANDLE)fd, &errors, &status);
        perror("serialport_write: Error writing to COM port");
        rieturn -1;
    }
#else /* Linux, Mac */
    int n = write(fd, str, len);
    if( n!=len ) 
        return -1;
#endif
    return 0;
}

int serialport_read_until(SERIALPORT_HANDLE fd, char* buf, char until){
    char b[1] {'\0'};
    int i=0;
    do { 
#ifdef _WIN32
        DWORD n;
        if(!ReadFile(fd, b, 1, &n, NULL)){ /* READFILE returns TRUE on success */
            return -1;
        }
#else /* Linux, Mac */
        int n = read(fd, b, 1);  // read a char at a time
#endif
        if( n==-1){
#if _WIN32
            fprintf(stderr,"COULDN'T READ\n");
            return -1;    // couldn't read
        }else if( n==0 ) {
            //fprintf(stderr,"READ 0\n");
            usleep( 10 ); // wait 10 msec try again
            continue;
#else
            continue;
#endif
        }else{
            //fprintf(stderr,"%d=[%c]\n",i,b[0]);
            buf[i] = b[0];
            i = i + 1;
        }
    } while( b[0] != until );

    buf[i] = '\0';  // null terminate the string
    return 0;
}

int serialport_readbyte(SERIALPORT_HANDLE fd, char *b){
#ifdef _WIN32
    DWORD n;
    if(ReadFile(fd, b, 1, &n, NULL)){
        return -1;
    }
#else /* Linux, Mac */
    int n = read(fd, b, 1);  // read a char at a time
    if(n==1)return 0;
    return -1;
#endif
}


// Serial Port (e.g. "/dev/tty.usbserial","COM1")



SERIALPORT_HANDLE serialport_init(const char *serialport, int baud, SERIALPORT_TERMIOS *oldsettings){
    SERIALPORT_HANDLE fd;
    
#ifdef _WIN32
   {fd = CreateFile(serialport, GENERIC_READ|GENERIC_WRITE, 0, NULL
            , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );
    if(fd==INVALID_HANDLE_VALUE){
        DWORD error = GetLastError();
        switch(error){  
            case ERROR_ACCESS_DENIED:
                fprintf(stderr,"serislport_init: Unable to open port: Access denied (port in use?)\n");
                return SERIALPORT_FILE_ERROR;
            case ERROR_FILE_NOT_FOUND:
                fprintf(stderr,"serialport_init: Unable to open port '%s'. File not found.\n",serialport);
                return SERIALPORT_FILE_ERROR;
            default:
                perror("serialport_init: Unknown error in CreateFile call.");
                return SERIALPORT_FILE_ERROR;
        }
    }

    DCB toptions;
    if(!GetCommState(fd, &toptions)){
        perror("serialport_init: Couldn't get COM state");
        return SERIALPORT_FILE_ERROR;
    }

    // save old settings if requested
    if(oldsettings){
           *oldsettings = toptions;
    }

    DWORD brate;
    switch(baud) {
        case 4800:   brate=CBR_4800;   break;
        case 9600:   brate=CBR_9600;   break;
        case 14400:  brate=CBR_14400;  break;
        case 19200:  brate=CBR_19200;  break;
        case 38400:  brate=CBR_38400;  break;
        case 57600:  brate=CBR_57600;  break;
        case 115200: brate=CBR_115200; break;  
        // some other speeds are possible, see DCB structure details (this
        // 115200 is the max speed of arduino.
    }

    toptions.BaudRate = brate;
    toptions.ByteSize = 8;
    toptions.StopBits = ONESTOPBIT;
    toptions.Parity = NOPARITY; 
    toptions.fOutxCtsFlow = FALSE;
    toptions.fOutxDsrFlow = FALSE;
    toptions.fOutX = FALSE;
    toptions.fInX = FALSE;
    toptions.fRtsControl = RTS_CONTROL_DISABLE;
    toptions.fTXContinueOnXoff = TRUE;
    toptions.fNull = FALSE;
    
    
    if(!SetCommState(fd, &toptions)) {
        perror("serialport_init: Couldn't set COM state");
        return SERIALPORT_FILE_ERROR;
    }
    }

#else /* Linux, Mac */
   {struct termios toptions;
    
    //fprintf(stderr,"init_serialport: opening port %s @ %d bps\n",
    //        serialport,baud);

    fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)  {
        perror("serialport_init: Unable to open port ");
        return SERIALPORT_FILE_ERROR;
    }
    
    if (tcgetattr(fd, &toptions) < 0) {
        perror("serialport_init: Couldn't get term attributes");
        return SERIALPORT_FILE_ERROR;
    }

    // save old settings if requested
    if(oldsettings){
           *oldsettings = toptions;
    }

    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
#ifdef B14400
    case 14400:  brate=B14400;  break;
#endif
    case 19200:  brate=B19200;  break;
#ifdef B28800
    case 28800:  brate=B28800;  break;
#endif
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;
    
    if( tcsetattr(fd, TCSANOW, &toptions) < 0) {
        perror("serialport_init: Couldn't set term attributes");
        return SERIALPORT_FILE_ERROR;
    }
    }
#endif

    return fd;
}

int serialport_restoresettings(SERIALPORT_HANDLE fd, SERIALPORT_TERMIOS *oldsettings){

#ifdef _WIN32
    if(!SetCommState(fd, oldsettings)) {
        perror("serialport_restoresettings: Couldn't set COM state");
        return -1;
    }
#else
    if(tcsetattr(fd, TCSANOW, oldsettings) < 0) {
        perror("serialport_restoresettings: Couldn't set term attributes");
        return -1;
    }
#endif
    return 0;
}


int serialport_close(SERIALPORT_HANDLE fd){
    if(fd<0){
        perror("serialport_close: COM was not open");
        return -1;
    }
#ifdef _WIN32
    CloseHandle(fd);
#else /* Linux, Mac */
    close(fd);
#endif
    return 0;
}
