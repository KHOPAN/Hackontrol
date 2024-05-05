package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.modal.ModalContext;
import com.khopan.hackontrol.manager.modal.ModalManager;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.interactions.callbacks.IModalCallback;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;

public class DialogChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "dialog";

	private static Button BUTTON_NEW_DIALOG = ButtonManager.staticButton(ButtonStyle.SUCCESS, "New Dialog", "newDialog");

	private static String MODAL_NEW_DIALOG = "modalNewDialog";
	private static String MODAL_EDIT_DIALOG = "modalEditDialog";

	@Override
	public String getName() {
		return DialogChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, DialogChannel.BUTTON_NEW_DIALOG, this :: buttonNewDialog);
		registry.register(ModalManager.MODAL_REGISTRY, DialogChannel.MODAL_NEW_DIALOG, this :: modalNewDialog);
		registry.register(ModalManager.MODAL_REGISTRY, DialogChannel.MODAL_EDIT_DIALOG, this :: modalEditDialog);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(DialogChannel.BUTTON_NEW_DIALOG)).queue();
	}

	private void buttonNewDialog(ButtonContext context) {
		this.newEditDialogModal(context.getEvent(), false, null, null);
	}

	private void modalNewDialog(ModalContext context) {
		StringSelectMenu menu = StringSelectMenu.create("testSelect")
				.addOption("Icon Information", "iconInformation")
				.addOption("Icon Question", "iconQuestion")
				.addOption("Icon Warning", "iconWarning")
				.addOption("Icon Error", "iconError")
				.setDefaultValues("iconWarning")
				.build();

		context.replyComponents(ActionRow.of(menu)).queue();
	}

	private void modalEditDialog(ModalContext context) {
		context.acknowledge();
	}

	private void newEditDialogModal(IModalCallback callback, boolean edit, String title, String messageContent) {
		TextInput titleInput = TextInput.create("title", "Title", TextInputStyle.SHORT)
				.setMinLength(-1)
				.setMaxLength(-1)
				.setRequired(false)
				.setPlaceholder("Error")
				.setValue(title)
				.build();

		TextInput messageInput = TextInput.create("messageContent", "Message", TextInputStyle.PARAGRAPH)
				.setMinLength(-1)
				.setMaxLength(-1)
				.setRequired(false)
				.setPlaceholder("Type a message...")
				.setValue(messageContent)
				.build();

		Modal modal = Modal.create(edit ? DialogChannel.MODAL_EDIT_DIALOG : DialogChannel.MODAL_NEW_DIALOG, edit ? "Edit Dialog" : "New Dialog")
				.addActionRow(titleInput)
				.addActionRow(messageInput)
				.build();

		callback.replyModal(modal).queue();
	}
}
