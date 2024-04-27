package com.khopan.hackontrol.manager.modal;

import java.util.List;

import com.khopan.hackontrol.manager.common.ICommonGetter;
import com.khopan.hackontrol.manager.common.IEventGetter;
import com.khopan.hackontrol.manager.common.IInteractionContext;
import com.khopan.hackontrol.manager.common.IThinkable;
import com.khopan.hackontrol.manager.common.sender.IMessageable;
import com.khopan.hackontrol.manager.common.sender.IRepliable;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendableMessage;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendableReply;

import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;

public interface ModalContext extends IEventGetter<ModalInteractionEvent>, IInteractionContext, IThinkable, IRepliable, IMessageable, ICommonGetter, ISendableReply, ISendableMessage {
	ModalMapping value(String identifier);
	List<ModalMapping> values();
}
