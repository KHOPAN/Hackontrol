package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;

public class NicknameCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("nickname").requires(source -> source.isSelected()).executes(context -> {
			CommandSource source = context.getSource();
			String nickname = source.getNickname();
			source.sendMessage('`' + source.getMachineId() + "`: " + (nickname == null ? "NONE" : '`' + nickname + '`'));
			return 1;
		}).then(RequiredArgumentBuilder.<CommandSource, String>argument("nickname", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String nickname = StringArgumentType.getString(context, "nickname");
			source.setNickname(nickname);
			source.sendMessage('`' + source.getMachineId() + "`: Nickname changed to `" + nickname + '`');
			return 1;
		})));
	}
}
