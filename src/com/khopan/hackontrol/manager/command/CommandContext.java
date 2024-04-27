package com.khopan.hackontrol.manager.command;

import com.khopan.hackontrol.manager.common.ICommonGetter;
import com.khopan.hackontrol.manager.common.IEventGetter;
import com.khopan.hackontrol.manager.common.IThinkable;
import com.khopan.hackontrol.manager.common.sender.IMessageable;
import com.khopan.hackontrol.manager.common.sender.IRepliable;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendableMessage;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendableReply;

import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.interactions.commands.Command;

public interface CommandContext extends IEventGetter<SlashCommandInteractionEvent>, IThinkable, IRepliable, IMessageable, ICommonGetter, ISendableReply, ISendableMessage {
	Command getCommand();
}
