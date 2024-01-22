package com.khopan.hackontrol.command;

import com.khopan.hackontrol.permission.Permission;

import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;

public interface Command {
	SlashCommandData getCommand();
	Permission getPermissionLevel();
}
