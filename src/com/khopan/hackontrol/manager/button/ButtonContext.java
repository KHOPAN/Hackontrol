package com.khopan.hackontrol.manager.button;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.common.IEventGetter;
import com.khopan.hackontrol.manager.common.IMessageHandler;
import com.khopan.hackontrol.manager.common.IReplyHandler;
import com.khopan.hackontrol.manager.common.InteractionContext;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.requests.restaction.interactions.ModalCallbackAction;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public interface ButtonContext extends IEventGetter<ButtonInteractionEvent>, InteractionContext, IReplyHandler, IMessageHandler {
	boolean hasParameters();
	Object[] getParameters();
	void typing();
	ModalCallbackAction replyModal(Modal modal);
	JDA getBot();
	Guild getGuild();
	MessageChannelUnion getChannel();
	InteractionHook getInteractionHook();
	Consumer<MessageCreateData> reply();
	Consumer<MessageCreateData> message();
}
