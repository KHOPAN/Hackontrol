package com.khopan.hackontrol;

import java.util.List;

import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public class DiscordUtils {
	private DiscordUtils() {}

	public static Category getOrCreateCategory(Guild guild, String name) {
		List<Category> list = guild.getCategoriesByName(name, true);

		if(list.isEmpty()) {
			return guild.createCategory(name).complete();
		}

		return list.get(0);
	}

	public static TextChannel getOrCreateTextChannelInCategory(Category category, String name) {
		List<TextChannel> list = category.getTextChannels();
		boolean hasChannel = false;
		TextChannel targetChannel = null;

		for(int i = 0; i < list.size(); i++) {
			TextChannel channel = list.get(i);

			if(channel.getName().equalsIgnoreCase(name)) {
				hasChannel = true;
				targetChannel = channel;
				break;
			}
		}

		if(hasChannel) {
			return targetChannel;
		}

		return category.createTextChannel(name).complete();
	}
}
