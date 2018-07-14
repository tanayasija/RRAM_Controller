#include "RRAM_Controller.h"

void RRAM_Controller::enable_read(void)
{
	ren = ren_p->read();
	if (ren_low == false && ren == SC_LOGIC_0)
	{
		cout << "Read enable set low at time " << sc_time_stamp()  <<  endl;
		ren_low = true;
		begin_read_index.notify();
	}
	else if (ren_low == true && ren == SC_LOGIC_1)
	{
		cout << "Read enable set high at time " << sc_time_stamp()  << endl;
		ren_low = false;
	}

	next_trigger(ren_p->default_event() & clk_in_p->posedge_event());
}

void RRAM_Controller::enable_write(void)
{
	wen = wen_p->read();
	if (wen_low == false && wen == SC_LOGIC_0)
	{
		cout << "Write enable set low at time " << sc_time_stamp() << endl;
		begin_RRAM_write.notify();
		wen_low = true;
	}
	else if (wen_low == true && wen == SC_LOGIC_1)
	{
		cout << "Write enable set high at time " << sc_time_stamp() << endl;
		wen_low = false;
	}
	
	next_trigger(wen_p->default_event() & clk_in_p->posedge_event());
}

void RRAM_Controller::read_index(void)
{
	for(;;)
	{
		int i=0;
		wait(SC_ZERO_TIME);
		cout << "Reading index byte "<< endl;
		index = index_p->read();
		cout << "Read index byte successfully as " << index << " at time " << sc_time_stamp() << endl;  
		begin_RRAM_read.notify();
		wait();
	}
}

void RRAM_Controller::RRAM_read(void)
{
	for(;;)
	{
		wait(begin_RRAM_read);
		cout << "Starting RRAM read operation" << endl;	
		wait(clk_in_p.posedge_event());
		wait(time_RRAM_cs_hold);	
		cs_p->write(sc_bit('0'));
		sc_bv<8> read_ins =RRAM_instruction_read;
		for (int i=0;i<8;i++)
		{
			wait(clk_in_p.posedge_event());
			sc_logic r = (sc_logic)read_ins[i];
			bit_out_p->write(r);
		}
		sc_uint<8> index_int =  index;
		sc_uint<8> para_add_int = para_add;
		
		sc_uint<16> add_int = index_int*para_add_int;
		sc_lv<16> address = add_int;
	
		for (int i=0;i<8;i++)
		{
			wait(clk_in_p->posedge_event());
			bit_out_p->write(sc_logic('0'));	
		}
		for (int i=0;i<16;i++)
		{
			wait(clk_in_p->posedge_event());
			sc_logic abit = (sc_logic) address[i];
			bit_out_p->write(abit);	
		}

		sc_lv<8> high_impedance = "ZZZZZZZZ";
		int num_read_bits = 8*num_read_bytes;
		for (int i=0;i<num_read_bits;i++)
		{
			sc_logic d = sc_logic('0');
			wait(clk_in_p->negedge_event());
			if (i==0) bit_out_p->write(sc_logic('Z'));
			data_valid_p->write(sc_bit('0'));
			wait(SC_ZERO_TIME);   
			d= bit_in_p->read();
			cout  << "Read bit " << i%8+1 << " of byte " << i/8+1 << " at time " << sc_time_stamp() << " as " << d << endl;
			data[i%8] = d;
			if (i%8 == 7)
			{
				cout << "Data valid changing to 1" << endl;
				data_valid_p->write(sc_bit('1'));
				data_read_p->write(data);
			}
			else
			{
				data_read_p->write(high_impedance);
				
			}
		}
		wait(clk_in_p->posedge_event());
		done_p->write(true);
		data_read_p->write(high_impedance);
		data_valid_p->write(sc_bit('0'));
		wait(time_RRAM_cs_hold);
		cout << "Chip select changing value at time " << sc_time_stamp() << endl;
		cs_p->write(sc_bit('1'));
		cout << "Exiting read operation" << endl;
		wait(clk_in_p->negedge_event());
		done_p->write(false);	
	}
}

void RRAM_Controller::RRAM_write(void)
{
	for(;;)
	{
		wait(begin_RRAM_write);
		cout << "Write enable set low, starting RRAM write operation by opening file" << endl;
		fileReader.open("weights.txt");
		int num_lines = 0;
		int weight = 0;
		sc_bv<32> data_sc_bv = 0;
		fileReader >> num_lines;
		
		wait(clk_in_p->posedge_event());
		wait(5,SC_NS);
		cout << "Driving CS low" << endl;
		cout << "Write 0: " << sc_time_stamp() << endl;
		cs_p->write(sc_bit('0'));
		cout << "CS written low by controller" << endl;
		sc_bv<8> write_enable_ins = "01100000";
		for (int i=0;i<8;i++){
			wait();
			sc_logic r = (sc_logic)write_enable_ins[i];
			bit_out_p->write(r);
		}
		cout << "Write enable instruction written" << endl;
		wait(5,SC_NS);
		cout << "Write 1: " << sc_time_stamp() << endl;
		cs_p->write(sc_bit('1'));
		wait(clk_in_p->posedge_event());
		wait(5,SC_NS);
		cout << "Write 2: " << sc_time_stamp() << endl;
		cs_p->write(sc_bit('0'));
		sc_bv<8> write_ins = "01000000";
		for (int i=0;i<8;i++){
			wait(clk_in_p->posedge_event());
			sc_logic r = (sc_logic)write_ins[i];
			bit_out_p->write(r);
		}
		for (int i=0;i<24;i++){
			wait(clk_in_p->posedge_event());
			bit_out_p->write(sc_logic('0'));
		}
		cout << "Address written to RRAM" << endl;
		for (int i=0; i<num_lines ; i++)
		{
			fileReader >> weight;
			data_sc_bv = weight;
			cout << "Weight " << i+1 << " "  << data_sc_bv << endl;
			for (int j=0;j<32;j++)
			{
				wait(clk_in_p->posedge_event());
				bit_out_p->write(sc_logic(data_sc_bv[j]));
			}
			cout << "Weight " << i+1 << " written to RRAM" << endl; 
		}
		wait(5,SC_NS);
		cout << "Write 3: " << sc_time_stamp() << endl;
		cs_p->write(sc_bit('1'));
		fileReader.close();
	}
}
