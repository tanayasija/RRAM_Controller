#include "top.h"

void top::transciever(void)
{
	cout << "Waiting for positive edge" << endl;
	wait(clk.posedge_event());
	ren_lc.write(sc_bit('0'));
	index = "00001000";
	cout << index << endl;
	for(int i=0;i<8;i++)
	{
		sc_logic ibit = (sc_logic) index[7-i];
		cout << "Sending bit " << i+1 << " as " << ibit << " at time " << sc_time_stamp() << endl;
		index_lc.write(index);
	}
	wait(clk.posedge_event());
	ren_lc.write(sc_bit('1'));
	index_lc.write(high_impedance);
	wait(valid_lc.posedge_event());
	begin_RRAM_read.notify();
	wait(done_RRAM_read);
	cout << sc_time_stamp() << endl;
	wait(clk.posedge_event());
	wait(5,SC_NS);
	cout << sc_time_stamp() << endl;
	ren_lc.write(sc_bit('1'));
	wen_lc.write(sc_bit('0'));
	wait(1,SC_MS);
	wait(clk.posedge_event());
	wait(5,SC_NS);
	wen_lc.write(sc_bit('1'));
}

void top::top_RRAM_read(void)
{
	for(;;)
	{
		data = data_lc.read();
		cout << "Byte number " << 1 << " read as " << data.to_string(SC_BIN) << endl;
		for (int i=0;i<RRAM_bytes_read-1;i++)
		{
			wait(clk.negedge_event() & valid_lc.posedge_event());
			data = data_lc.read();
			cout << "Byte number " << i+2 << " read as " << data.to_string(SC_BIN) << endl;
		}
		done_RRAM_read.notify();
		wait();
	}
}	
