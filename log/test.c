#include "./infolog.h"

int main()
{
	do_log("what the fuck", 13, LEVEL_DEBUG);
	do_log("what the fuck", 13, LEVEL_INFO);
	do_log("what the fuck", 13, LEVEL_ERR);
	return 0;
}
