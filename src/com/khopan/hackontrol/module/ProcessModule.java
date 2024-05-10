package com.khopan.hackontrol.module;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ProcessModule extends Module {
	private static final String MODULE_NAME = "process";

	private static final Button BUTTON_SNAPSHOT = ButtonManager.staticButton(ButtonType.SUCCESS, "Snapshot", "processSnapshot");

	@Override
	public String getName() {
		return ProcessModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, ProcessModule.BUTTON_SNAPSHOT, this :: buttonSnapshot);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ProcessModule.BUTTON_SNAPSHOT)).queue();
	}

	private void buttonSnapshot(ButtonContext context) {

	}
}
