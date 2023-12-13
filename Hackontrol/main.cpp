#include <dpp/dpp.h>
#include "bottoken.h"

int main(int argc, char** argv) {
	dpp::cluster bot(BOT_TOKEN);
	bot.on_log(dpp::utility::cout_logger());
	bot.start(dpp::st_wait);
	return 0;
}
