#include "stdafx.h"
#include "machine.h"
#include "bot_token.h"

char* g_machineGuid;

void onReady(dpp::cluster&);

int main(int argc, char** argv) {
	g_machineGuid = machine_getGUID();
	dpp::cluster bot(BOT_TOKEN);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_ready([&bot](auto event) {
		onReady(bot);
	});

	bot.start(dpp::st_wait);
	return 0;
}

void onReady(dpp::cluster& bot) {
	dpp::channel_map channels = bot.channels_get_sync(dpp::snowflake(1173967259304198154L));
	
	for(std::pair<const dpp::snowflake, dpp::channel> entry : channels) {
		const dpp::snowflake snowflake = entry.first;
		dpp::channel channel = entry.second;
		std::cout << channel.name << '\n';
	}
}
