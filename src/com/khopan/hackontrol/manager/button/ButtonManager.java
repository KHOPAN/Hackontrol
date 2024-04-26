package com.khopan.hackontrol.manager.button;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.utils.DiscordUtils;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.events.message.MessageDeleteEvent;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class ButtonManager implements Manager {
	public static final RegistryType<String, Consumer<ButtonContext>> STATIC_BUTTON_REGISTRY = RegistryType.create();

	private List<RegistrationTypeEntry<String, Consumer<ButtonContext>>> staticCallbackList;

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(ButtonInteractionEvent.class, this :: buttonEvent));
		builder.addEventListeners(FilteredEventListener.create(MessageDeleteEvent.class, this :: deleteEvent));
	}

	@Override
	public void initialize(RegistrationHandler handler) {
		this.staticCallbackList = handler.filterType(ButtonManager.STATIC_BUTTON_REGISTRY);
	}

	private void buttonEvent(ButtonInteractionEvent Event) {
		MessageChannelUnion channel = Event.getChannel();

		if(!DiscordUtils.checkCategory(channel)) {
			return;
		}

		Button button = Event.getButton();
		String identifier = button.getId();
		DynamicButtonSession session = DynamicButtonSession.decodeSession(identifier);
		Hackontrol.LOGGER.info("Processing button: {}", identifier);
		Consumer<ButtonContext> action;

		if(session == null) {
			HackontrolChannel hackontrolChannel = Hackontrol.getInstance().getChannel((TextChannel) channel);
			action = RegistrationTypeEntry.filter(this.staticCallbackList, hackontrolChannel, identifier);
		} else {			
			action = session.action;
		}

		if(action == null) {
			Hackontrol.LOGGER.warn("Button {} has null action", identifier);
			return;
		}

		action.accept(new ButtonContextImplementation(Event, session == null ? null : session.paramters));
	}

	private void deleteEvent(MessageDeleteEvent Event) {
		if(!DiscordUtils.checkCategory(Event.getChannel())) {
			return;
		}

		long messageIdentifier = Event.getMessageIdLong();
		List<DynamicButtonSession> deleteList = new ArrayList<>();

		for(int i = 0; i < DynamicButtonSession.SESSION_LIST.size(); i++) {
			DynamicButtonSession session = DynamicButtonSession.SESSION_LIST.get(i);

			if(session.messageIdentifier == messageIdentifier) {
				deleteList.add(session);
			}
		}

		DynamicButtonSession.SESSION_LIST.removeAll(deleteList);
	}

	public static Button dynamicButton(ButtonStyle style, String label, Consumer<ButtonContext> action, Object... paramters) {
		if(style == null) {
			throw new NullPointerException("Button style cannot be null");
		}

		if(label == null) {
			label = "Button";
		}

		long sessionIdentifier = DynamicButtonSession.randomSessionIdentifier();
		String buttonIdentifier = "bm-sid" + Long.toString(sessionIdentifier);
		Button button = ButtonManager.staticButton(style, label, buttonIdentifier);
		DynamicButtonSession session = new DynamicButtonSession();
		session.sessionIdentifier = sessionIdentifier;
		session.action = action;
		session.paramters = paramters;
		DynamicButtonSession.SESSION_LIST.add(session);
		return button;
	}

	public static Button staticButton(ButtonStyle style, String label, String identifier) {
		switch(style) {
		case PRIMARY:
			return Button.primary(identifier, label);
		case SECONDARY:
			return Button.secondary(identifier, label);
		case SUCCESS:
			return Button.success(identifier, label);
		case DANGER:
			return Button.danger(identifier, label);
		case LINK:
			return Button.link(identifier, label);
		default:
			throw new IllegalArgumentException("Invalid button style");
		}
	}

	public static Button selfDelete(ButtonStyle style, String label, Object... paramters) {
		return ButtonManager.dynamicButton(style, label, ButtonManager :: selfDeleteCallback, paramters);
	}

	private static void selfDeleteCallback(ButtonContext context) {
		if(context.hasParameters()) {
			Object[] parameters = context.getParameters();
			MessageChannelUnion channel = context.getChannel();

			for(int i = 0; i < parameters.length; i++) {
				if(parameters[i] instanceof Number number) {
					long messageIdentifier = number.longValue();
					channel.retrieveMessageById(messageIdentifier).queue(message -> message.delete().queue(), Errors -> {});
				}
			}
		}

		context.delete();
	}

	public static void dynamicButtonCallback(Message message) {
		long messageIdentifier = message.getIdLong();
		List<ActionRow> actionRows = message.getActionRows();

		for(int x = 0; x < actionRows.size(); x++) {
			ActionRow actionRow = actionRows.get(x);
			List<Button> buttons = actionRow.getButtons();

			for(int y = 0; y < buttons.size(); y++) {
				Button button = buttons.get(y);
				String identifier = button.getId();
				DynamicButtonSession session = DynamicButtonSession.decodeSession(identifier);

				if(session == null) {
					continue;
				}

				session.messageIdentifier = messageIdentifier;
			}
		}
	}

	public static void dynamicButtonCallback(InteractionHook hook) {
		hook.retrieveOriginal().queue(ButtonManager :: dynamicButtonCallback);
	}
}
