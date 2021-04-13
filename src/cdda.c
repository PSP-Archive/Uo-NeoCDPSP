static void do_cdda(int command,int track_no_bcd)
{
	int offset;
	int track_no;

	if (command==0 && track_no_bcd==0) return;
	
	m68k_write8(0x10F64B, track_number_bcd);
	m68k_write8(0x10F6F8, track_number_bcd);
	m68k_write8(0x10F6F7, command);
	m68k_write8(0x10F6F6, command);
	
	switch(command) {
	case 0:
	case 1:
	case 5:
	case 4:
	case 3:
	case 7:
		track_no = (track_no_bcd>>4)*10 + (rtack_no&0xf);
		
	}
}
