package com.khopan.hackontrol.registry;

import java.util.function.Consumer;

import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.service.interaction.context.ModalContext;
import com.khopan.hackontrol.service.interaction.context.StringSelectContext;

import net.dv8tion.jda.api.events.message.MessageReceivedEvent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class Registration {
	private Registration() {}

	public static final Registrable<Consumer<MessageReceivedEvent>>          MESSAGE_RECEIVED_EVENT = Registrable.create();

	public static final BiRegistrable<Button, Consumer<ButtonContext>>       BUTTON                 = BiRegistrable.create();
	public static final BiRegistrable<String, Consumer<ModalContext>>        MODAL                  = BiRegistrable.create();
	public static final BiRegistrable<String, Consumer<StringSelectContext>> STRING_SELECT_MENU     = BiRegistrable.create();
}
