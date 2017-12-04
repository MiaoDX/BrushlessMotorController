int parseInt2(byte bt){
	return int(bt);
}
int parseSInt2(byte bt){
	if(bt & (1<<7)) {
		bt ^= (1<<7);
		return -int(bt);
	}
	return int(bt);
}
