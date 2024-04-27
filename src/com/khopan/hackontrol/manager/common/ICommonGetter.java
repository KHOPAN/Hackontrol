package com.khopan.hackontrol.manager.common;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.interactions.InteractionHook;

public interface ICommonGetter {
	JDA getBot();
	Guild getGuild();
	MessageChannelUnion getChannel();
	InteractionHook getInteractionHook();
}
