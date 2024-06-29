package com.khopan.hackontrol.utils.interaction;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.Base64;
import java.util.List;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.security.Permissive;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.utils.HackontrolPermission;

import net.dv8tion.jda.api.entities.Member;
import net.dv8tion.jda.api.entities.channel.middleman.GuildMessageChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class HackontrolButton {
	private HackontrolButton() {}

	public static Button delete(long... attachedMessages) {
		StringBuilder builder = new StringBuilder();
		builder.append("imsd");

		if(attachedMessages != null && attachedMessages.length > 0) {
			ByteArrayOutputStream stream = new ByteArrayOutputStream();

			for(int i = 0; i < attachedMessages.length; i++) {
				long identifier = attachedMessages[i];
				stream.write((byte) ((identifier >> 56) & 0xFF));
				stream.write((byte) ((identifier >> 48) & 0xFF));
				stream.write((byte) ((identifier >> 40) & 0xFF));
				stream.write((byte) ((identifier >> 32) & 0xFF));
				stream.write((byte) ((identifier >> 24) & 0xFF));
				stream.write((byte) ((identifier >> 16) & 0xFF));
				stream.write((byte) ((identifier >> 8) & 0xFF));
				stream.write((byte) (identifier & 0xFF));
			}

			String base64 = Base64.getEncoder().encodeToString(stream.toByteArray());

			if(builder.length() + base64.length() > 100) {
				Object[] array = new Object[attachedMessages.length];

				for(int i = 0; i < array.length; i++) {
					array[i] = attachedMessages[i];
				}

				return ButtonManager.dynamicButton(ButtonType.DANGER, "Delete", HackontrolButton :: dynamicDeleteCallback, array);
			}

			builder.append(base64);
		}

		return ButtonManager.staticButton(ButtonType.DANGER, "Delete", builder.toString());
	}

	private static void dynamicDeleteCallback(ButtonContext context) {
		if(!HackontrolPermission.checkPermission(Permissive.DELETE_BUTTON, context.getMember(), context.reply())) {
			return;
		}

		context.deferEdit().queue(hook -> hook.deleteOriginal().queue());

		if(context.hasParameter()) {
			Object[] parameters = context.getParameters();
			MessageChannelUnion channel = context.getChannel();

			for(int i = 0; i < parameters.length; i++) {
				if(parameters[i] instanceof Number number) {
					channel.retrieveMessageById(number.longValue()).queue(message -> message.delete().queue(), Errors -> {});
				}
			}
		}
	}

	public static boolean deleteMessages(ButtonInteractionEvent Event) {
		long originalMessage = Event.getMessageIdLong();
		GuildMessageChannel channel = (GuildMessageChannel) Event.getChannel();
		Member member = Event.getMember();

		for(Button button : Event.getMessage().getButtons()) {
			String identifier = button.getId();

			if(HackontrolButton.deleteMessages(identifier, originalMessage, channel, member)) {
				return true;
			}
		}

		return false;
	}

	public static boolean deleteMessages(String identifier, long originalMessage, GuildMessageChannel channel, Member member) {
		if(identifier.startsWith("imsd") && HackontrolButton.processDeleteButton(channel, originalMessage, identifier.substring(4), member)) {
			return true;
		}

		return false;
	}

	private static boolean processDeleteButton(GuildMessageChannel channel, long originalMessage, String base64, Member member) {
		List<String> list = new ArrayList<>();

		if(!base64.isEmpty()) {
			try {
				byte[] data = Base64.getDecoder().decode(base64);

				for(int i = 0; i < data.length; i += 8) {
					long identifier = (((long) data[i] & 0xFF) << 56) |
							(((long) data[i + 1] & 0xFF) << 48) |
							(((long) data[i + 2] & 0xFF) << 40) |
							(((long) data[i + 3] & 0xFF) << 32) |
							(((long) data[i + 4] & 0xFF) << 24) |
							(((long) data[i + 5] & 0xFF) << 16) |
							(((long) data[i + 6] & 0xFF) << 8) |
							((long) data[i + 7] & 0xFF);

					list.add(Long.toUnsignedString(identifier));
				}
			} catch(Throwable Errors) {
				return false;
			}
		}

		if(list.isEmpty()) {
			channel.deleteMessageById(originalMessage).queue();
			return true;
		}

		list.add(Long.toUnsignedString(originalMessage));
		channel.deleteMessagesByIds(list).queue();
		return true;
	}
}
