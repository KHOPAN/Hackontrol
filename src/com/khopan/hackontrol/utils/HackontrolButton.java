package com.khopan.hackontrol.utils;

import com.khopan.hackontrol.manager.button.ButtonManager;

import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class HackontrolButton {
	private HackontrolButton() {}

	public static Button delete(Object... parameters) {
		return ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete", parameters);
	}
}
