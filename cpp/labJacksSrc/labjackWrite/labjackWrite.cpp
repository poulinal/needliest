// labjack_writer.cpp
// Build: g++ -std=c++17 -O2 -I/path/to/ljm/include -L/path/to/ljm/lib -lljm -o labjack_writer labjack_writer.cpp
// g++ -std=c++17 -O2 -Iexternal/ljm/include -Lexternal/ljm/lib -lljm -o labjack_writer cpp/labJacksSrc/labjack_writer.cpp
// Run: ./labjack_writer /dev/cu.usbserial-DN06A5F2

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <chrono>
#include <thread>

// LJM header - adjust the include name to your SDK
extern "C" {
#include "LabJackM.h" // or labjacksM.h depending on your SDK
}

int open_serial(const char* dev, int baud) {
    int fd = ::open(dev, O_WRONLY | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        std::cerr << "open(" << dev << ") failed: " << strerror(errno) << "\n";
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "tcgetattr error: " << strerror(errno) << "\n";
        close(fd);
        return -1;
    }

    cfmakeraw(&tty);
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // 8N1
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;

    // No flow control
    tty.c_cflag &= ~CRTSCTS;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "tcsetattr error: " << strerror(errno) << "\n";
        close(fd);
        return -1;
    }

    return fd;
}

bool write_line(int fd, const std::string& s) {
    size_t to_write = s.size();
    const char* buf = s.c_str();
    while (to_write) {
        ssize_t n = ::write(fd, buf, to_write);
        if (n < 0) {
            if (errno == EINTR) continue;
            std::cerr << "Serial write error: " << strerror(errno) << "\n";
            return false;
        }
        to_write -= (size_t)n;
        buf += n;
    }
    return true;
}

int main(int argc, char** argv) {
    const char* serial_dev = "/dev/cu.usbserial-DN06A5F2";
    int baud = 115200;
    double poll_hz = 5.0;

    if (argc >= 2) serial_dev = argv[1];

    // Open serial
    int sfd = open_serial(serial_dev, baud);
    if (sfd < 0) return 1;

    // Open LabJack T7
    int handle;
    int err = LJM_OpenS("T7", "USB", "ANY", &handle);
    if (err != 0) {
        char errStr[200];
        LJM_ErrorToString(err, errStr);
        std::cerr << "LJM_OpenS error: " << errStr << "\n";
        close(sfd);
        return 2;
    }

    std::cout << "Opened LabJack and serial " << serial_dev << "\n";

    while (true) {
        double v = 0.0;
        err = LJM_eReadName(handle, "AIN0", &v);
        if (err != 0) {
            char errStr[200];
            LJM_ErrorToString(err, errStr);
            std::cerr << "LJM read error: " << errStr << "\n";
            v = 0.0;
        }

        char line[64];
        snprintf(line, sizeof(line), "%.3f\n", v);
        if (!write_line(sfd, line)) break;

        std::cout << "sent: " << line; // see progress on console
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000.0 / poll_hz)));
    }

    LJM_Close(handle);
    close(sfd);
    return 0;
}