#include <dpp/dpp.h>
#include "definition.h"
#include "bottoken.h"

int main(int argc, char** argv) {
	Execute(NULL, NULL, NULL, 0);
	return 0;
}

EXPORT Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	dpp::cluster bot(BOT_TOKEN);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_slashcommand([](auto event) {
		if(event.command.get_command_name() == "screenshot") {
			event.reply("!! NOT IMPLEMENTED !!");
		}
	});

	bot.on_ready([&bot](auto event) {
		if(dpp::run_once<struct register_bot_commands>()) {
			bot.global_command_create(dpp::slashcommand("screenshot", "Take screenshot", bot.me.id));
		}
	});

	bot.start(dpp::st_wait);
}
