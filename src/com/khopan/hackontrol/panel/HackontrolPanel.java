package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.library.Information;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.StringSelectMenuManager;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.OnlineStatus;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class HackontrolPanel extends Panel {
	public static final String PANEL_NAME = "hackontrol";

	private static final Button BUTTON_PING          = ButtonManager.staticButton(ButtonType.SUCCESS, "Ping",       "ping");
	private static final Button BUTTON_STATUS        = ButtonManager.staticButton(ButtonType.SUCCESS, "Status",     "setStatus");
	private static final Button BUTTON_HACKONTROL    = ButtonManager.staticButton(ButtonType.SUCCESS, "Hackontrol", "settingsHackontrol");

	private static final String STRING_SELECT_STATUS = "selectBotStatus";

	@Override
	public String panelName() {
		return HackontrolPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON,             HackontrolPanel.BUTTON_PING,          context -> HackontrolMessage.deletable(context.reply(), this.getOnlineText(System.currentTimeMillis())));
		this.register(Registration.BUTTON,             HackontrolPanel.BUTTON_STATUS,        context -> context.replyComponents(ActionRow.of(StringSelectMenuManager.staticMenu(HackontrolPanel.STRING_SELECT_STATUS).addOption("Online", DiscordStatus.ONLINE.name(), "Hackontrol will appear online").addOption("Idle", DiscordStatus.IDLE.name(), "Hackontrol will appear as idle").addOption("Do Not Disturb", DiscordStatus.DO_NOT_DISTURB.name(), "Hackontrol will appear as do not disturb").addOption("Invisible", DiscordStatus.INVISIBLE.name(), "Hackontrol will not appear online").setMaxValues(1).setDefaultValues(DiscordStatus.fromOnlineStatus(this.channel.getJDA().getPresence().getStatus()).name()).build())).queue());
		this.register(Registration.BUTTON,             HackontrolPanel.BUTTON_HACKONTROL,    context -> {});
		this.register(Registration.STRING_SELECT_MENU, HackontrolPanel.STRING_SELECT_STATUS, context -> {
			this.channel.getJDA().getPresence().setStatus(DiscordStatus.fromName(context.getValues().get(0)).status);
			context.deferEdit().queue(hook -> hook.deleteOriginal().queue());
		});
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(HackontrolPanel.BUTTON_PING, HackontrolPanel.BUTTON_STATUS, HackontrolPanel.BUTTON_HACKONTROL)
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
		ONLINE(OnlineStatus.ONLINE),
		IDLE(OnlineStatus.IDLE),
		DO_NOT_DISTURB(OnlineStatus.DO_NOT_DISTURB),
		INVISIBLE(OnlineStatus.INVISIBLE);

		OnlineStatus status;

		DiscordStatus(OnlineStatus status) {
			this.status = status;
		}

		private static DiscordStatus fromOnlineStatus(OnlineStatus status) {
			for(DiscordStatus value : DiscordStatus.values()) {
				if(value.status.equals(status)) {
					return value;
				}
			}

			return DiscordStatus.INVISIBLE;
		}

		private static DiscordStatus fromName(String name) {
			for(DiscordStatus value : DiscordStatus.values()) {
				if(value.name().equals(name)) {
					return value;
				}
			}

			return DiscordStatus.ONLINE;
		}
	}
}
