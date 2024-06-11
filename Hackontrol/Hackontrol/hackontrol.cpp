#include <dpp/dpp.h>
#include "hackontrol.h"

void startHackontrol(const char* botToken, unsigned long long userIdentifier) {
	std::string token(botToken);
	dpp::cluster bot(token);
	bot.on_ready([&bot, userIdentifier](auto event) {
		bot.direct_message_create_sync(dpp::snowflake(userIdentifier), dpp::message("**Hackontrol is running!**"));
	});

	bot.start(dpp::st_return);
}
