package com.khopan.hackontrol.utils.interaction;

import java.util.List;

import com.khopan.hackontrol.service.interaction.context.StringSelectContext;

import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;

public class HackontrolStringSelectMenu {
	private HackontrolStringSelectMenu() {}

	public static void saveDefault(StringSelectContext context) {
		List<String> values = context.getValues();
		StringSelectMenu menu = context.getSelectMenu().createCopy().setDefaultValues(values).build();
		context.editSelectMenu(menu).queue();
	}
}
