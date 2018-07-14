#ifndef top_H
#define top_H

#include "RRAM.h"
#include "RRAM_Controller.h"

#include <systemc>
#include <sstream>
#include <string>
#include <iostream>
using namespace std;
using namespace sc_core;

SC_MODULE(top)
{
	RRAM rram;
	RRAM_Controller controller;

	sc_signal<sc_bit, SC_MANY_WRITERS> cs_lc;
	sc_signal<sc_logic, SC_MANY_WRITERS> RRAM_din_lc;
	sc_signal<sc_logic> RRAM_dout_lc;
	
	sc_clock clk; 

	sc_signal<sc_bit> ren_lc;
	sc_signal<sc_bit> wen_lc;
	sc_signal<sc_lv<8> > index_lc;
	sc_signal<sc_lv<8> > data_lc;
	sc_signal<bool> valid_lc;
	sc_signal<bool> done_lc;	

	sc_lv<8> index;
	sc_lv<8> data;

	int RRAM_bytes_read;	
	sc_lv<8> high_impedance;

	sc_event begin_RRAM_read;
	sc_event done_RRAM_read;

	sc_time time_index_hold;	
	
	sc_trace_file* tracefile;	

	SC_CTOR(top):
		clk("clk",20,SC_NS,0.5,10,SC_NS,true),
		rram("rram"),
		controller("controller"),
		RRAM_bytes_read(10),
		high_impedance("ZZZZZZZZ"),
		time_index_hold(5,SC_NS)
	{
/**************************************************************************
		Selecting signals to be traced as waveforms
**************************************************************************/
	
		tracefile = sc_create_vcd_trace_file("waveform");
		sc_trace(tracefile,clk,"clock");
		sc_trace(tracefile,ren_lc,"ren");
		sc_trace(tracefile,wen_lc,"wen");
		sc_trace(tracefile,index_lc,"index");
		sc_trace(tracefile,data_lc,"data");	
		sc_trace(tracefile,valid_lc,"valid");
		sc_trace(tracefile,cs_lc,"cs");
		sc_trace(tracefile,RRAM_din_lc,"DI");
		sc_trace(tracefile,RRAM_dout_lc,"DO");	
		sc_trace(tracefile,done_lc,"done");

/**************************************************************************
		Attaching sub-module ports to channels
**************************************************************************/
		rram.cs_p(cs_lc);
		rram.clk_p(clk);
		rram.data_in_p(RRAM_din_lc);
		rram.data_out_p(RRAM_dout_lc);

		controller.clk_in_p(clk);
		controller.cs_p(cs_lc);
		controller.bit_out_p(RRAM_din_lc);
		controller.bit_in_p(RRAM_dout_lc);

		controller.ren_p(ren_lc);
		controller.wen_p(wen_lc);
		controller.index_p(index_lc);	
		controller.data_read_p(data_lc);
		controller.data_valid_p(valid_lc);
		controller.done_p(done_lc);

/**************************************************************************
			Initializing channels
**************************************************************************/	
		ren_lc.write(sc_bit('1'));
		wen_lc.write(sc_bit('1'));
		index_lc.write(high_impedance);	
		data_lc.write(high_impedance);

/**************************************************************************
	Registering top module processes with simulation kernel
**************************************************************************/	
		SC_THREAD(transciever);
		SC_THREAD(top_RRAM_read);
			sensitive << begin_RRAM_read;
			dont_initialize();
		cout << "Initialized top module" << endl;
	}

	void transciever(void);
	void top_RRAM_read(void);
};
#endif
