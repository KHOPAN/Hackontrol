package com.khopan.hackontrol.manager.interaction;

import com.khopan.hackontrol.utils.sendable.ISendable;
import com.khopan.hackontrol.utils.sendable.ISendableMessage;
import com.khopan.hackontrol.utils.sendable.ISendableReply;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;
import com.khopan.hackontrol.utils.sendable.sender.ReplyCallbackSendable;

import net.dv8tion.jda.api.events.interaction.component.StringSelectInteractionEvent;

public class StringSelectContext extends StringSelectInteractionEvent implements ISendableMessage, ISendableReply {
	public StringSelectContext(StringSelectInteractionEvent Event) {
		super(Event.getJDA(), Event.getResponseNumber(), Event.getInteraction());
	}

	@Override
	public ISendable reply() {
		return ReplyCallbackSendable.of(this);
	}

	@Override
	public ISendable message() {
		return MessageChannelSendable.of(this.getChannel());
	}
}
