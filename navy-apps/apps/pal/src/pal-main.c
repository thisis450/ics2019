#include <common.h>

void main_loop();
void hal_init();

int
main(int argc, char *argv[]) {
	Log("game start!");
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
  hal_init();
	main_loop();

	return 0;
}
