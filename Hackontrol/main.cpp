#include "stdafx.h"
#include "machine.h"
#include "bot_token.h"

int main(int argc, char** argv) {
	/*dpp::cluster bot(BOT_TOKEN);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_ready([&bot](auto event) {
		dpp::message message;
		message.set_content("Hello, world!, bot has started");
		message.set_guild_id(dpp::snowflake(1173967259304198154L));
		message.set_channel_id(dpp::snowflake(1173967259862048891L));
		bot.message_create(message);
	});

	bot.start(dpp::st_wait);*/
	char* guid = machine_getGUID();
	printf("GUID: %s\n", guid);
	free(guid);
	return 0;
}
