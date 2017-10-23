#include "XBeeHandler.h"
#include "global.h"
#include <unistd.h>
#include <iostream>
#include <math.h>       /* log */


using namespace std;


#define P_V(variable)\
     cout << "(" <<__FUNCTION__ <<":"<<__LINE__<<") {"#variable"}---> [" << std::dec << (variable) << "]" <<endl \


enum io_pin_options_e{
  DISABLE,
  ANALOG_IN_SINGLE_ENDED = 2,
  DIGI_IN,
  DIGI_OUT_LOW,
  DIGI_OUT_HIGH,
  IO_OPTIONS_COUNT
};


uint8_t rutera_ddress[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0x1A, 0xE0};
uint8_t end_node_address[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0x1A, 0xDD};

const double V_REF = 2.2;


inline double to_temp(double ohm) {
  const double A = 0.001129148;
  const double B = 0.000234125;
  const double C = 8.76741E08;

//  double A = 3.354016;
//  double B = 2.569850;
//  double C = 2.62013E-06;

  const double KELVIN_OFFSET = -273.15;

  return( (1 / (A + (B * log(ohm)) + C *(pow( log(ohm), 3)))) ) + KELVIN_OFFSET ;
}

 double to_v(uint16_t adc){ // Should be 1.3 ish
  return (V_REF / 1023.0) * adc;
}

inline double to_ohm(uint16_t val) {

  const double R1 = 18000;
  double v_out = to_v(val);
  double r2 = R1 * (1 / ( (V_REF/v_out) - 1));
  return r2;

}

inline double to_temp2(uint16_t adc_raw_val){
  double temp = 0;

  temp = log(((10230000 / adc_raw_val) - 10000));
  P_V(log(((10230000 / adc_raw_val) - 10000)));
  temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp)) * temp);
  temp = temp - 273.15;
  return temp;
}



void get_temp_callback(XBeeHandler::xbee_frame rx_frame) {

  if (rx_frame.data.size() < sizeof(int8_t) * 8) {
    std::cout << RED << "Rx frame data is incomplete" << END;
    return;
  }

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

  auto adc_raw_val = (uint16_t) (0x00FF & (rx_frame.data.at(ANALOG_SAMPLE + 1)) | (rx_frame.data.at(ANALOG_SAMPLE) << 8));
  temp1 = to_temp(to_ohm(adc_raw_val));
  temp2 = to_temp2(adc_raw_val);

  cout << "Digital pin enabled  ~> " << (int) rx_frame.data.at(DIGI_SAMPLES) << endl;
  cout << "Analog pin enabled   ~> " << (int) rx_frame.data.at(ANALOG_MASK) << endl;
  cout << "V out                ~> " <<  to_v(adc_raw_val) << endl;
  cout << "Analog value raw     ~> " << std::hex << adc_raw_val << endl;
  cout << "R2                   ~> " << std::dec << (int) to_ohm(adc_raw_val) << endl;
  cout << "Temperature (1)      ~> " << temp1 << endl;
  cout << "Temperature (2)      ~> " << temp2 << endl;
}


void enable_temp_sens_callback(XBeeHandler::xbee_frame rx_frame) {
  cout << "Not yet implemented" << endl;
}


void frame_constructer(XBeeHandler *xbee_handler){

  string at_cmd;
  string cmd_params;
  cout << BLUE << "Type AT cmd " << END << endl;
  cin >> at_cmd;
  cout << "got this: \"" << at_cmd << "\" "<< endl;
  cout << BLUE << "Type cmd params " << END << endl;
  cin >> cmd_params;
  std::vector<uint8_t > params;
  params[0] = (uint8_t) (cmd_params[0] - 0x30);
  cout << "got this: \"" << (int) params[0] << "\" "<< endl;

//  xbee_handler->send(end_node_address, "IS", params , std::bind(&get_temp_callback, std::placeholders::_1));

}


char human_cmd = 0;


int main(int argc, char *argv[]) {
  XBeeHandler xbee_handler(20);
  xbee_handler.init("/dev/ttyUSB0");

  std::vector<uint8_t> parameter_DI = {2};

  cout << CYAN_DARK << "\nStarting.." << END << endl;
  cout << "Type next cmd please .. \n" << "~> ";
  while(true) {
    cin >> human_cmd;
    cout << endl;
    switch (human_cmd){
      case 'q' :
        cout << CYAN_DARK <<"Quitting thank you" << END << endl;
        exit(EXIT_SUCCESS);
      case 't' :
        cout << CYAN_DARK << "Sampling temperature" << END << endl;
        xbee_handler.send(end_node_address, "IS", std::vector<uint8_t>(), std::bind(&get_temp_callback, std::placeholders::_1));
        break;
      case '0':
        cout << CYAN_DARK << "Getting digital status" << END << endl;
        xbee_handler.send(end_node_address, "D3", {DIGI_OUT_LOW}, std::bind(&enable_temp_sens_callback, std::placeholders::_1));
        break;
      case '1':
        cout << CYAN_DARK << "Getting digital status" << END << endl;
        xbee_handler.send(end_node_address, "D3", {DIGI_OUT_HIGH}, std::bind(&enable_temp_sens_callback, std::placeholders::_1));
        break;
      case 'a':
        cout << CYAN_DARK << "Getting digital status" << END << endl;
        while(true){
          xbee_handler.send(end_node_address, "IS", std::vector<uint8_t>(), std::bind(&get_temp_callback, std::placeholders::_1));
          sleep(4);
        }
      case 'b':
        frame_constructer(&xbee_handler);
        break;
      case 'c':
        xbee_handler.send(end_node_address, "D1", {2}, std::bind(&enable_temp_sens_callback, std::placeholders::_1));
        break;

      default: break;
    }
    //system("stty -echo && stty raw");
  }
}






