package com.khopan.hackontrol.panel;

import java.util.List;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.nativelibrary.Information;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.StringSelectMenuManager;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;

public class HackontrolPanel extends Panel {
	public static final String PANEL_NAME = "hackontrol";

	private static final Button BUTTON_PING          = ButtonManager.staticButton(ButtonType.SUCCESS, "Ping", "ping");

	private static final String STRING_SELECT_STATUS = "selectBotStatus";

	@Override
	public String panelName() {
		return HackontrolPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON,             HackontrolPanel.BUTTON_PING,          context -> HackontrolMessage.deletable(context.reply(), this.getOnlineText(System.currentTimeMillis())));
		this.register(Registration.STRING_SELECT_MENU, HackontrolPanel.STRING_SELECT_STATUS, context -> {
			List<String> values = context.getValues();
			StringSelectMenu menu = context.getSelectMenu().createCopy().setDefaultValues(values).build();
			context.editSelectMenu(menu).queue();
		});
	}

	@Override
	public ControlWidget[] controlWidget() {
		StringSelectMenu menu = StringSelectMenuManager.staticMenu(HackontrolPanel.STRING_SELECT_STATUS)
				.addOption("Online",         DiscordStatus.ONLINE.name(),         "Hackontrol will appear online")
				.addOption("Idle",           DiscordStatus.IDLE.name(),           "Hackontrol will appear as idle")
				.addOption("Do Not Disturb", DiscordStatus.DO_NOT_DISTURB.name(), "Hackontrol will appear as do not disturb")
				.addOption("Invisible",      DiscordStatus.INVISIBLE.name(),      "Hackontrol will not appear online")
				.setMaxValues(1)
				.build();

		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(HackontrolPanel.BUTTON_PING)
				.build(),
				ControlWidget.newBuilder()
				.text("**Bot Status:**")
				.actionRow(menu)
				.build()
		};
	}

	@Override
	public void initialize() {
		this.channel.sendMessage(this.getOnlineText(Hackontrol.STARTUP_TIME)).queue();
	}

	private String getOnlineText(long time) {
		time /= 1000L;
		return "**Online: <t:" + time + ":f> (<t:" + time + ":R>) (" + Information.getUserName() + ") (" + (Information.isEnabledUIAccess() ? "" : "No ") + "UI Access)**";
	}

	private static enum DiscordStatus {
		ONLINE,
		IDLE,
		DO_NOT_DISTURB,
		INVISIBLE;
	}
}
