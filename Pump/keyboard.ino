
// --------------------------------------------------------
// KEYBOARD
// --------------------------------------------------------
//
void onKeyboard()
{
	expander.checkForInterrupt();
}

void onKeyStop()
{
	boolean press;
	
	wrkMode=2;
}

void onKeyManl()
{
	wrkMode=0;
}
