#ifndef RRAM_Controller_H
#define RRAM_Controller_H

#include <systemc>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
using namespace sc_core;
using namespace sc_dt;

SC_MODULE(RRAM_Controller)
{
	sc_in<bool> clk_in_p;

	sc_in<sc_lv<8> > index_p;
	sc_in<sc_bit> ren_p;
	sc_in<sc_bit> wen_p;

	sc_out<sc_logic> bit_out_p;
	sc_in<sc_logic> bit_in_p;
	sc_out<sc_bit> cs_p;

	sc_out<sc_lv<8> > data_read_p;
	sc_out<bool> data_valid_p;
	sc_out<bool> done_p;
	
	sc_event begin_read_index;
	sc_event begin_RRAM_read;
	sc_event begin_RRAM_write;

	#define RRAM_instruction_read "11000000"
	#define num_read_bytes 10;

	ifstream fileReader;	
	bool ren_low;
	bool ren;
	bool wen_low;
	bool wen;
	sc_lv<8> index;
	sc_lv<16> address;
	sc_lv<8> data;

	sc_lv<8> para_add;

	sc_time time_RRAM_cs_hold;

	SC_CTOR(RRAM_Controller):
		time_RRAM_cs_hold(5,SC_NS),
		para_add("10000000")
	{
		SC_METHOD(enable_read);
			sensitive << ren_p.value_changed();
		SC_METHOD(enable_write);
			sensitive << wen_p.value_changed();
		SC_THREAD(read_index);
			sensitive << begin_read_index;
			dont_initialize();
		SC_THREAD(RRAM_read);
		SC_THREAD(RRAM_write);
			sensitive << clk_in_p.pos();
		cs_p.initialize(sc_bit('1'));
		bit_out_p.initialize(sc_logic('Z'));
		data_valid_p.initialize(sc_bit('0'));
		cout << "Initialized RRAM_Controller" << endl;
	}

	void enable_read(void);
	void enable_write(void);
	void read_index(void);
	void RRAM_read(void);
	void RRAM_write(void);
};
#endif
