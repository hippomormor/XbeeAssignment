#include "XBeeHandler.h"
#include "colors.h"
#include <unistd.h>
#include <iostream>
#include <cmath>       /* log */
#include <socket.h>


using namespace std;


// Debug macro to print out a variables name and value.
#define P_V(variable)\
     cout << "(" <<__FUNCTION__ <<":"<<__LINE__<<") {"#variable"}---> [" << std::dec << (variable) << "]" <<endl; \

std::vector<uint8_t> NO_PARAMS;
const double V_REF = 1.90;


Socket ipcSocket = Socket();


enum io_pin_options_e {
  DISABLE,
  ANALOG_IN_SINGLE_ENDED = 2,
  DIGI_IN,
  DIGI_OUT_LOW,
  DIGI_OUT_HIGH,
  IO_OPTIONS_COUNT
};


uint8_t router_ddress[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0x1A, 0xE0};
uint8_t end_node_address[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0x1A, 0xDD};


// Convert ohms to temperature
inline double to_temp(double ohm) {
//  const double A = 0.001129148;
//  const double B = 0.000234125;
//  const double C = 8.76741E08;

  double A = 3.354016;
  double B = 2.569850;
  double C = 2.62013E-06;
  double D = 6.38309E-08;
  double Rref = 1000;
  double R_over_Rref = ohm/Rref;
  
  const double KELVIN_OFFSET = -273.15;
  
  // This calculation is directly from the data-sheet... it does not give any sane temperature. wtf??????
  double res = ( A + (B * log( R_over_Rref)) + (C * pow( R_over_Rref,2)) + ( D * pow( R_over_Rref,3)));
  
  P_V(ohm)
  P_V(res)
  P_V(1/res)
  
  return 1/res;
  
  // Old calculation
  return (( (A + (B * log(ohm)) + C * (pow(log(ohm), 3))))) + KELVIN_OFFSET;
}

inline double to_v(uint16_t adc) {
  return (1.2 / 1023.0) * adc;
}

inline double r2_to_ohm(uint16_t adc_raw) {
  const double R1 = 28000;
  double v_out = to_v(adc_raw);
  double r2 = R1 * (1 / ((V_REF / v_out) - 1));
  return r2;
}

inline double r1_to_ohm(uint16_t adc_raw) {
  const double R2 = 10000;
  double v_out = to_v(adc_raw);
  return (R2 * ((V_REF / v_out) - 1));
}


inline double ntc_to_ohm__Ole_Schultz(uint16_t adc_raw) {
  double v_out = to_v(adc_raw);
  return ((33000.0 - 28000.0 * v_out) / (v_out));
}


// Use the calculation example handout out by the teacher
inline double to_temp2(uint16_t adc_raw_val) {
  double temp = 0;
  
  temp = log(((10230000 / adc_raw_val) - 10000));
  temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp)) * temp);
  temp = temp - 273.15;
  return temp;
}


// Callback to parse the temperature data received from the end node.
void get_temp_callback(XBeeHandler::xbee_frame rx_frame) {
  
  if (rx_frame.data.size() < sizeof(int8_t) * 8) { //Hot-Fix to ensure no overflow
    std::cout << RED << "Rx frame data is incomplete" << END;
    return;
  }
  
  // This should maybe be changed to be dynamic instead
  enum io_data_fields_e {
    NO_SAMPLES,
    DIG_MASK,
    ANALOG_MASK = DIG_MASK + 2,
    DIGI_SAMPLES,
    ANALOG_SAMPLE = DIGI_SAMPLES + 2,
    DAT_FIELD_COUNT
  };
  double temp1 = 0;
  double temp2 = 0;
  
  // Collapse the to bytes to a 16 bit short for the ADC value.
  auto adc_raw_val = (uint16_t) (0x00FF & (rx_frame.data.at(ANALOG_SAMPLE + 1)) |
                                 (rx_frame.data.at(ANALOG_SAMPLE) << 8));
  
  
  P_V(r1_to_ohm(adc_raw_val))
  P_V(r2_to_ohm(adc_raw_val))
  P_V(ntc_to_ohm__Ole_Schultz(adc_raw_val))
  P_V(r1_to_ohm(adc_raw_val) - 18000)
  
  double ohm = r1_to_ohm(adc_raw_val) - 18000;
  
  
  temp1 = to_temp(ohm); // Get temp from our temp calculation
  temp2 = to_temp2(adc_raw_val);        // Get temp from Oles temp calculation
  
  cout << YELLOW << endl;
  cout << "Digital pin enabled  ~> " << (int) rx_frame.data.at(DIGI_SAMPLES) << endl;
  cout << "Analog pin enabled   ~> " << (int) rx_frame.data.at(ANALOG_MASK) << endl;
  cout << "V out                ~> " << to_v(adc_raw_val) << endl;
  cout << "Analog value raw     ~> " << std::hex << adc_raw_val << endl;
  cout << "R1                   ~> " << std::dec << ohm << endl;
  cout << "Temperature (1)      ~> " << temp1 << endl;
  cout << "Temperature (2)      ~> " << temp2 << endl;
  cout << END << endl;
  
}

// (@TODO) Pars the IO status 
void enable_temp_sens_callback(XBeeHandler::xbee_frame rx_frame) {
  cout << "Not yet implemented" << endl;
}


// Manually build a frame and transmit it to the node in question 
void frame_constructer(XBeeHandler *xbee_handler) {
  string at_cmd;
  string cmd_params;
  cout << BLUE << "Type AT cmd " << END << endl;
  cin >> at_cmd;
  cout << "got this: \"" << at_cmd << "\" " << endl;
  cout << BLUE << "Type cmd params " << END << endl;
  cin >> cmd_params;
  std::vector<uint8_t> params;
  params[0] = (uint8_t) (cmd_params[0] - 0x30);
  cout << "got this: \"" << (int) params[0] << "\" " << endl;

//  xbee_handler->send(end_node_address, "IS", params , std::bind(&get_temp_callback, std::placeholders::_1));

}


// m..m..Main 
int main(int argc, char *argv[]) {
  
  
  ipcSocket.InitTx("localhost", 3828);
  
  string first = "hello Java from C IPC test";
  
  ipcSocket.Tx((void*) first.c_str(), first.length() );
  
  char human_cmd = 0;
  XBeeHandler xbee_handler(20);
  xbee_handler.init("/dev/ttyUSB0"); // Connect to to the Xbee
  
  cout << CYAN_DARK << "\nStarting.." << END << endl;
  cout << "Type next cmd please .. \n" << "~> ";
  while (true) {
    cin >> human_cmd;
    cout << endl;
    switch (human_cmd) {
      case 'q' :
        cout << CYAN_DARK << "Quitting thank you for participating" << END << endl;
        exit(EXIT_SUCCESS);
      case 't' : // Sample remote temp sensor
        cout << CYAN_DARK << "Sampling temperature" << END << endl;
        xbee_handler.send(end_node_address, "IS", NO_PARAMS, std::bind(&get_temp_callback, std::placeholders::_1));
        break;
      case '0': // Clear remote control bit
        cout << CYAN_DARK << "Getting digital status" << END << endl;
        xbee_handler.send(end_node_address, "PR", {0x1F, 0xFD},
                          std::bind(&enable_temp_sens_callback, std::placeholders::_1));
        break;
      case '1': // Set remote control bit
        cout << CYAN_DARK << "Getting digital status" << END << endl;
        xbee_handler.send(end_node_address, "PR", {0x1F, 0xFF},
                          std::bind(&enable_temp_sens_callback, std::placeholders::_1));
        break;
      case 'a': // Auto run. Sample remote temp every 4 sec forever
        cout << CYAN_DARK << "Getting digital status" << END << endl;
        while (true) {
          xbee_handler.send(end_node_address, "IS", NO_PARAMS, std::bind(&get_temp_callback, std::placeholders::_1));
          sleep(4);
        }
      case 'b': // Build a frame manually
        frame_constructer(&xbee_handler);
        break;
      case 'c': // Debug options
        xbee_handler.send(end_node_address, "D1", {2}, std::bind(&enable_temp_sens_callback, std::placeholders::_1));
        break;
      
      default:
        break;
    }
  }
}






