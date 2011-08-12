extern int main(int argc, char *argv[]);

void _start()
{
  int argc = 0;
  char *argv[0];
  int ret = main(argc, argv);
}