package com.khopan.hackontrol.manager.button;

import com.khopan.hackontrol.manager.common.ICommonGetter;
import com.khopan.hackontrol.manager.common.IEventGetter;
import com.khopan.hackontrol.manager.common.IInteractionContext;
import com.khopan.hackontrol.manager.common.IThinkable;
import com.khopan.hackontrol.manager.common.sender.IMessageable;
import com.khopan.hackontrol.manager.common.sender.IRepliable;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendableMessage;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendableReply;

import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.requests.restaction.interactions.ModalCallbackAction;

public interface ButtonContext extends IEventGetter<ButtonInteractionEvent>, IInteractionContext, IThinkable, IRepliable, IMessageable, ICommonGetter, ISendableReply, ISendableMessage {
	boolean hasParameters();
	Object[] getParameters();
	void typing();
	ModalCallbackAction replyModal(Modal modal);
}
