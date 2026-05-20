#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

int setup_serial(const char* port) {

    int serial_port = open(port, O_RDWR);

    if (serial_port < 0) {
        std::cerr << "Failed to open " << port << std::endl;
        return -1;
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

    return serial_port;
}


void send_and_receive(int serial, const char* name, char cmd) {

    tcflush(serial, TCIFLUSH);

    write(serial, &cmd, 1);

    std::cout << "Sent to " << name << ": " << cmd << std::endl;

    usleep(50000); // optional 50ms delay

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    int total = 0;

    while (true) {

        int n = read(serial,
                     buffer + total,
                     sizeof(buffer) - total - 1);

        if (n <= 0)
            break;

        total += n;

        // Stop on newline
        if (buffer[total - 1] == '\n')
            break;
    }

    if (total > 0) {
        buffer[total] = '\0';
        std::cout << name << " says: " << buffer << std::endl;
    }
}


int main() {

    // Pico dispenser
    const char* pico_port = "/dev/ttyACM0";

    // Second serial device
    const char* second_port = "/dev/ttyACM1";

    int pico_serial = setup_serial(pico_port);
    int second_serial = setup_serial(second_port);

    if (pico_serial < 0 || second_serial < 0) {
        return 1;
    }

    std::cout << "Press:\n";
    std::cout << "1 = Send to Pico\n";
    std::cout << "2 = Send to ttyACM1\n";
    std::cout << "q = Quit\n";


	while (true) {

	    char cmd;
	    std::cin >> cmd;

	    if (cmd == 'q')
		break;

	    if (cmd == '1' || cmd == '0') {
		send_and_receive(pico_serial, "Pico", cmd);
	    }

	    if (cmd == '2') {
		send_and_receive(second_serial, "ttyACM1", cmd);
	    }
	}
      
    

    close(pico_serial);
    close(second_serial);

    return 0;
}
