package com.khopan.hackontrol.channel.command;

import java.util.List;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public class ClearCommand {
	private ClearCommand() {}

	public static boolean test(String command) {
		return (command.length() == 3 && command.startsWith("cls")) ||
				(command.length() == 5 && command.startsWith("clear")) ||
				command.startsWith("cls ") ||
				command.startsWith("clear ");
	}

	public static void execute(MessageChannel channel) {
		MessageHistory.getHistoryFromBeginning(channel).queue(history -> {
			List<Message> list = history.getRetrievedHistory();

			for(int i = 0; i < list.size(); i++) {
				list.get(i).delete().queue();
			}
		});
	}
}
