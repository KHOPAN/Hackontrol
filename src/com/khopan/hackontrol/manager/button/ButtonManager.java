package com.khopan.hackontrol.manager.button;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;
import com.khopan.hackontrol.registry.RegistryType;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.channel.Channel;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ButtonManager implements Manager {
	public static final RegistryType<String, Consumer<ButtonInteraction>> BUTTON_CALLBACK_REGISTRY = RegistryType.create();

	public static final String BUTTON_DELETE_SELF = "buttonManagerSpecialDeleteSelf";

	private List<RegistrationTypeEntry<String, Consumer<ButtonInteraction>>> list;
	private Consumer<Boolean> callback;

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(ButtonInteractionEvent.class, this :: buttonEvent));
	}

	@Override
	public void initialize(RegistrationHandler handler) {
		this.list = handler.filterType(ButtonManager.BUTTON_CALLBACK_REGISTRY);
	}

	private void buttonEvent(ButtonInteractionEvent Event) {
		Channel channel = Event.getChannel();

		if(!(channel instanceof TextChannel)) {
			return;
		}

		TextChannel textChannel = (TextChannel) channel;
		Category category = textChannel.getParentCategory();
		Hackontrol hackontrol = Hackontrol.getInstance();

		if(!hackontrol.getCategory().equals(category)) {
			return;
		}

		HackontrolChannel hackontrolChannel = hackontrol.getChannel(textChannel);

		if(hackontrolChannel == null) {
			return;
		}

		Button button = Event.getButton();
		String identifier = button.getId();
		ButtonInteraction interaction = new ButtonInteractionImplementation(Event);

		if(this.callback != null && ("ok".equals(identifier) || "cancel".equals(identifier))) {
			interaction.consume();

			if("ok".equals(identifier)) {
				this.callback.accept(true);
			} else {
				this.callback.accept(false);
			}

			this.callback = null;
			return;
		}

		if(ButtonManager.BUTTON_DELETE_SELF.equals(identifier)) {
			interaction.consume();
			return;
		}

		Consumer<ButtonInteraction> value = RegistrationTypeEntry.filter(this.list, hackontrolChannel, identifier);

		if(value == null) {
			return;
		}

		value.accept(interaction);
	}

	private class ButtonInteractionImplementation implements ButtonInteraction {
		private final ButtonInteractionEvent Event;

		private ButtonInteractionImplementation(ButtonInteractionEvent Event) {
			this.Event = Event;
		}

		@Override
		public ButtonInteractionEvent getEvent() {
			return this.Event;
		}

		@Override
		public void consume() {
			this.Event.deferEdit().queue(hook -> hook.deleteOriginal().queue());
		}

		@Override
		public void okCancelQuestion(String question, Consumer<Boolean> callback) {
			this.question(question, callback, "Ok", "Cancel");
		}

		@Override
		public void yesNoQuestion(String question, Consumer<Boolean> callback) {
			this.question(question, callback, "Yes", "No");
		}

		private void question(String question, Consumer<Boolean> callback, String ok, String cancel) {
			ButtonManager.this.callback = callback;
			this.Event.reply(question).addActionRow(Button.success("ok", ok), Button.danger("cancel", cancel)).queue();
		}
	}
}
