package com.khopan.hackontrol.manager.modal;

import java.util.List;

import com.khopan.hackontrol.manager.common.IEventGetter;
import com.khopan.hackontrol.manager.common.IReplyHandler;
import com.khopan.hackontrol.manager.common.InteractionContext;

import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;

public interface ModalContext extends IEventGetter<ModalInteractionEvent>, InteractionContext, IReplyHandler {
	ModalMapping value(String identifier);
	List<ModalMapping> values();
}
