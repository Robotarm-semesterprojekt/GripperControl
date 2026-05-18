  #include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

int main() {
    const char* port = "/dev/ttyACM0";

    int serial_port = open(port, O_RDWR);

    if (serial_port < 0) {
        std::cerr << "Failed to open serial port\n";
        return 1;
    }

    termios tty{};

    tcgetattr(serial_port, &tty);

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    // Raw mode
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tcsetattr(serial_port, TCSANOW, &tty);

    std::cout << "Press 1 to close, 0 to open, q to quit\n";

    while (true) {

        char cmd;
        std::cin >> cmd;

        if (cmd == 'q') {
            break;
        }

        if (cmd == '0' || cmd == '1') {

            // Send command to Pico
            write(serial_port, &cmd, 1);

            std::cout << "Sent: " << cmd << std::endl;

            // Wait for response from Pico
            char buffer[256];

            memset(buffer, 0, sizeof(buffer));

            int n = read(serial_port, buffer, sizeof(buffer) - 1);

            if (n > 0) {
                buffer[n] = '\0';

                std::cout << "Pico says: " << buffer << std::endl;
            }
        }
    }

    close(serial_port);

    return 0;
}
