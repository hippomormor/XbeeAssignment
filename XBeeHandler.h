#ifndef PII_XBEEHANDLER_H
#define PII_XBEEHANDLER_H

#include <vector>
#include <string>
#include <thread>
#include <mutex>


class XBeeHandler {
public:
  typedef struct xbee_frame {
    uint16_t length;
    uint64_t address;
    std::vector<int8_t> data;
    uint8_t status;
    std::string command;
  };

  explicit XBeeHandler(uint32_t set_global_retries);

  virtual ~XBeeHandler();

  bool init(std::string tty_str);

  bool send(const uint8_t address[], const std::string &cmd, const std::vector<uint8_t> &cmd_params, const std::function<void(XBeeHandler::xbee_frame)> &callback);

private:
  enum frame_structure{
    START_DELIM,
    LENGTH,
    TYPE = LENGTH + 2,
    ID,
    ADDR_64,
    ADDR_16 = ADDR_64 + 8,
    CMD_OPT = ADDR_16 + 2,
    AT_CMD,
    CHECKSUM = AT_CMD + 2,
    TX_FRAME_SIZE
  };

  int serial_fd;

  std::mutex recieve_mutex;

  bool receive(const std::function<void(XBeeHandler::xbee_frame)> &callback);
  void exchange_thread(const uint8_t *address, const std::string &cmd, const std::vector<uint8_t> &cmd_params,
                         const std::function<void(XBeeHandler::xbee_frame)> &callback);

  uint32_t global_retries;
};

#endif //PII_XBEEHANDLER_H
