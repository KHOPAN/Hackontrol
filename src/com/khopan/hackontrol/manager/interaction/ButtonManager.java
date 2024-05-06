package com.khopan.hackontrol.manager.interaction;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public final class ButtonManager {
	private ButtonManager() {}

	public static Button dynamicButton(ButtonType type, String label, Consumer<ButtonContext> action, Object... parameters) {
		if(type == null) {
			throw new NullPointerException("Button type cannot be null");
		}

		if(label == null) {
			label = "Button";
		}

		long identifier = InteractionSession.newSession();
		Button button = ButtonManager.staticButton(type, label, InteractionSession.prefix(identifier));
		InteractionSession session = new InteractionSession();
		session.sessionIdentifier = identifier;
		session.paramters = parameters;
		session.action = action;
		session.type = InteractionType.BUTTON;
		InteractionSession.SESSION_LIST.add(session);
		return button;
	}

	public static Button staticButton(ButtonType style, String label, String identifier) {
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
			throw new IllegalArgumentException("Invalid button type");
		}
	}

	@SuppressWarnings("unchecked")
	static void buttonEvent(ButtonInteractionEvent Event, List<RegistrationTypeEntry<Button, Consumer<ButtonContext>>> buttonList) {
		String identifier = Event.getButton().getId();
		InteractionSession session = InteractionSession.decodeSession(identifier);
		Hackontrol.LOGGER.info("Processing button: {}", identifier);
		Consumer<ButtonContext> action = null;

		if(session == null) {
			HackontrolChannel hackontrolChannel = Hackontrol.getInstance().getChannel((TextChannel) Event.getChannel());

			for(int i = 0; i < buttonList.size(); i++) {
				RegistrationTypeEntry<Button, Consumer<ButtonContext>> entry = buttonList.get(i);

				if(hackontrolChannel.equals(entry.channel) && identifier.equals(entry.identifier.getId())) {
					action = entry.value;
					break;
				}
			}
		} else {
			if(!InteractionType.BUTTON.equals(session.type)) {
				Hackontrol.LOGGER.warn("Mismatch button interaction type: {}", identifier);
			}

			action = (Consumer<ButtonContext>) session.action;
		}

		if(action != null) {
			action.accept(new ButtonContext(Event, session == null ? null : session.paramters));
		}
	}

	static void assignIdentifier(Message message) {
		long messageIdentifier = message.getIdLong();
		List<ActionRow> rowList = message.getActionRows();

		for(int x = 0; x < rowList.size(); x++) {
			List<Button> buttonList = rowList.get(x).getButtons();

			for(int y = 0; y < buttonList.size(); y++) {
				InteractionSession session = InteractionSession.decodeSession(buttonList.get(y).getId());

				if(session != null) {
					session.messageIdentifier = messageIdentifier;
				}
			}
		}
	}

	public static enum ButtonType {
		PRIMARY(ButtonStyle.PRIMARY),
		SECONDARY(ButtonStyle.SECONDARY),
		SUCCESS(ButtonStyle.SUCCESS),
		DANGER(ButtonStyle.DANGER),
		LINK(ButtonStyle.LINK);

		private final ButtonStyle style;

		ButtonType(ButtonStyle style) {
			this.style = style;
		}

		public ButtonStyle getStyle() {
			return this.style;
		}
	}
}
