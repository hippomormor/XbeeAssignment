#include "XBeeHandler.h"
#include "colors.h"

#include <cstring>
#include <termios.h>
#include <fcntl.h>
#include <iostream>
#include <zconf.h>


#include <unistd.h>


XBeeHandler::XBeeHandler(uint32_t set_global_retries) : global_retries(set_global_retries) { };


XBeeHandler::~XBeeHandler() {
  // Close serial file descriptor
  close(serial_fd);
}


bool XBeeHandler::init(std::string tty_str) {
  // Open serial file descriptor
  serial_fd = open(tty_str.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

  // Check if file descriptor is valid
  if (serial_fd < 0) {
    std::cout << RED << "Error " << errno << " opening" << tty_str.c_str() << ": " << strerror(errno) << END << std::endl;
    return false;
  }

  // Get serial attributes
  struct termios tty{};
  memset(&tty, 0, sizeof tty);
  if (tcgetattr(serial_fd, &tty) != 0) {
    std::cout << RED << "Error " << errno << " from tcgetattr" << END << std::endl;
    return false;
  }

  // Set output and input speed
  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);

  // Set Serial flags
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars

  // Disable IGNBRK for mismatched speed tests; otherwise receive break as \000 chars
  tty.c_iflag &= ~IGNBRK;                         // Disable break processing
  tty.c_lflag = 0;                                // No signaling chars, no echo,

  // No canonical processing
  tty.c_oflag = 0;                                // No remapping, no delays
  tty.c_cc[VMIN] = 0;                             // Read doesn't block
  tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // Shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);                // Ignore modem controls,

  // Enable reading
  tty.c_cflag &= ~(PARENB | PARODD);              // Shut off parity
  tty.c_cflag |= 0;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 5;                             // 0.5 seconds read timeout

  // Check if setting of attributes was successful
  if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
    std::cout << RED << "Error " << errno << " setting attributes" << END << std::endl;
    return false;
  }

  return true;
}


// Exchange parser thread
void XBeeHandler::exchange_thread(const uint8_t *address, const std::string &cmd, const std::vector<uint8_t> &cmd_params,
                                  const std::function<void(XBeeHandler::xbee_frame)> &callback) {

  std::cout << "Starting thread" << std::endl;

  // Lock mutex to make exchange thread-safe
  std::lock_guard<std::mutex> guard(recieve_mutex);

  // Wait for serial line to be ready
  sleep(1);

  // Create transmit data frame
  std::vector<uint8_t> tx_frame(TX_FRAME_SIZE - 1);
  tx_frame[START_DELIM] = 0x7E;
  tx_frame[TYPE]        = 0x17;
  tx_frame[ID]          = 0x01;
  tx_frame[ADDR_64 + 0] = address[0];
  tx_frame[ADDR_64 + 1] = address[1];
  tx_frame[ADDR_64 + 2] = address[2];
  tx_frame[ADDR_64 + 3] = address[3];
  tx_frame[ADDR_64 + 4] = address[4];
  tx_frame[ADDR_64 + 5] = address[5];
  tx_frame[ADDR_64 + 6] = address[6];
  tx_frame[ADDR_64 + 7] = address[7];
  tx_frame[ADDR_16 + 0] = 0xFF;
  tx_frame[ADDR_16 + 1] = 0xFE;
  tx_frame[CMD_OPT]     = 0x02;
  tx_frame[AT_CMD + 0]  = (uint8_t) cmd.at(0);
  tx_frame[AT_CMD + 1]  = (uint8_t) cmd.at(1);

  // Insert optional command parameters
  tx_frame.insert(std::end(tx_frame), std::begin(cmd_params), std::end(cmd_params));

  // Save length of data frame
  tx_frame[LENGTH] = (uint8_t) ((tx_frame.size() - 3) >> 8);
  tx_frame[LENGTH + 1] = (uint8_t) (tx_frame.size() - 3);

  // Calculate checksum
  int sum = 0;
  for (int i = 3; i < tx_frame.size(); i++) {
    sum = sum + tx_frame[i];
  }

  // Save checksum
  tx_frame.push_back(static_cast<uint8_t>(0xff - static_cast<uint8_t>(sum)));

  // Check if serial file descriptor is valid
  if (serial_fd == -1) {
    std::cout << RED << "File descriptor invalid " << END << std::endl;
    exit(EXIT_FAILURE);
  }

  // Print the transmit data
  std::cout << "\n" << CYAN_DARK << "Transmitting" << END << std::endl;
  for (auto &a : tx_frame) {
    printf("%02x ", a);
  } printf("\n");

  uint32_t retries = global_retries;

  // Calculate retries
  auto redundancy = [&] (const uint32_t &retries_intl) {
    sleep(3);
    std::cout << RED <<"Retrying.. \nRetry #" << global_retries - retries + 1 << END << "\n" << std::endl;
    return retries_intl - 1;
  };

  do{
    // Send data and wait for line
    write(serial_fd, tx_frame.data(), tx_frame.size());
    sleep(1);

    // Flush serial output line
    tcflush(serial_fd, TCOFLUSH);

    // Set retries
    retries = receive(callback) ? 0 : redundancy(retries);
  }while (retries > 0);

  // Unlock mutex
  recieve_mutex.unlock();
}


bool XBeeHandler::send(const uint8_t address[], const std::string &cmd, const std::vector<uint8_t> &cmd_params, const std::function<void(XBeeHandler::xbee_frame)> &callback) {
  // Start exchange thread and detach
  std::thread exchange_thread(&XBeeHandler::exchange_thread, this, address, cmd, cmd_params, callback);
  exchange_thread.detach();

  return true;
}

// Receive parser
bool XBeeHandler::receive(const std::function<void(XBeeHandler::xbee_frame)> &callback) {
  char buf[128] = {0};
  xbee_frame data_frame{0};

  // Read from serial file descriptor
  ssize_t n = read(serial_fd, buf, sizeof buf);

  // Check if serial file descriptor is valid
  if (n == -1) {
    std::cout << RED << "Read ERROR" << END << std::endl;
    return false;
  }

  // Validate status byte
  data_frame.status = static_cast<uint8_t>(buf[17]);
  printf("Status: %02x\n", data_frame.status);
  if (data_frame.status == 0x01) {
    std::cout << "Status: " << RED << "ERROR" << END << std::endl;
    return false;
  } else if (data_frame.status == 0x00) {
    std::cout << "Status: " << GREEN << "OK" << END << std::endl;
  } else {
    std::cout << "Status: " << YELLOW << "UNKNOWN" << END << std::endl;
  }

  // Validate checksum
  int sum = 0;
  for (int i = 3; i < sizeof(buf); i++) {
    sum = sum + buf[i];
  } printf("Checksum result: %x\n", (uint8_t) sum);
  if ((uint8_t) sum != 0xFF) {
    std::cout << "Checksum: " << RED << "ERROR" << END << std::endl;
    return false;
  } else {
    std::cout << "Checksum: " << GREEN << "OK" << END << std::endl;
  }

  // Get length
  data_frame.length = static_cast<uint16_t>(buf[2] | (buf[1] << 8));

  // Get address
  uint8_t address_64[8];
  int size = sizeof(address_64);
  for (auto &b : address_64) {
    b = static_cast<uint8_t>(buf[4 + size--]);
  } data_frame.address = *reinterpret_cast<uint64_t*>(address_64);

  // Get command
  data_frame.command[0] = static_cast<uint8_t>(buf[15]);
  data_frame.command[1] = static_cast<uint8_t>(buf[16]);

  // Print info to user
  printf("Length: %02x\n", data_frame.length);
  printf("Address: %02llx\n", static_cast<unsigned long long int>(data_frame.address));
  printf("Command: %s\n", data_frame.command.c_str());

  // Get data and print to user
  uint8_t data[data_frame.length - 15];
  size = 0;
  std::cout << "Data: ";
  for (auto &b : data) {
    data_frame.data.push_back(static_cast<uint8_t>(buf[18 + size++]));
    printf("%02x ", (uint8_t) data_frame.data[size - 1]);
  }
  if (data_frame.data.empty()) {
    std::cout << RED << "Empty" << END;
  }

  // Print received data
  size = 0;
  std::cout << CYAN_DARK << "\n\nReceived" << END << std::endl;
  for (auto &b : buf) {
    printf("%02x ", b);
    size++;
    if (size > data_frame.length + 3) {
      break;
    }
  }
  std::cout << std::endl;

  // Flush serial input line
  tcflush(serial_fd, TCIFLUSH);

  // Call callback functions
  callback(data_frame);

  return true;
}

