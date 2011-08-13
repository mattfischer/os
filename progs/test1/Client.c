#include <Name.h>
#include <IO.h>

int main(int argc, char *argv[])
{
	int obj = LookupName("test");
	while(1) {
		Write(obj, "A\r\n", 3);
	}
}