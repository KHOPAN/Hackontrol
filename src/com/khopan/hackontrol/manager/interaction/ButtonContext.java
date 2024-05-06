package com.khopan.hackontrol.manager.interaction;

import com.khopan.hackontrol.utils.sendable.ISendable;
import com.khopan.hackontrol.utils.sendable.ISendableMessage;
import com.khopan.hackontrol.utils.sendable.ISendableReply;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;
import com.khopan.hackontrol.utils.sendable.sender.ReplyCallbackSendable;

import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;

public class ButtonContext extends ButtonInteractionEvent implements IParameterized, ISendableMessage, ISendableReply {
	private final Object[] parameters;

	public ButtonContext(ButtonInteractionEvent Event, Object[] parameters) {
		super(Event.getJDA(), Event.getResponseNumber(), Event.getInteraction());
		this.parameters = parameters;
	}

	@Override
	public ISendable reply() {
		return ReplyCallbackSendable.of(this);
	}

	@Override
	public ISendable message() {
		return MessageChannelSendable.of(this.getChannel());
	}

	@Override
	public boolean hasParameter() {
		return this.parameters == null || this.parameters.length == 0;
	}

	@Override
	public Object[] getParameters() {
		return this.parameters;
	}
}
