package com.khopan.hackontrol.manager.command;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.eventlistener.InteractionEventListener;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;
import com.khopan.hackontrol.registry.RegistryType;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.interactions.commands.Command;
import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;

public class CommandManager implements Manager {
	public static final RegistryType<SlashCommandData, Consumer<CommandContext>> COMMAND_REGISTRY = RegistryType.create();

	private Map<Long, CommandEntry> commandMap;

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(SlashCommandInteractionEvent.class, this :: commandEvent));
	}

	@Override
	public void initialize(RegistrationHandler handler) {
		List<RegistrationTypeEntry<SlashCommandData, Consumer<CommandContext>>> commandList = handler.filterType(CommandManager.COMMAND_REGISTRY);
		Hackontrol hackontrol = Hackontrol.getInstance();
		Guild guild = hackontrol.getGuild();
		this.commandMap = new LinkedHashMap<>();
		guild.updateCommands().queue();

		for(int i = 0; i < commandList.size(); i++) {
			RegistrationTypeEntry<SlashCommandData, Consumer<CommandContext>> entry = commandList.get(i);
			Command command = guild.upsertCommand(entry.identifier).complete();
			CommandEntry commandEntry = new CommandEntry();
			commandEntry.action = entry.value;
			commandEntry.command = command;
			commandEntry.channel = entry.channel;
			this.commandMap.put(command.getIdLong(), commandEntry);
		}
	}

	private void commandEvent(SlashCommandInteractionEvent Event) {
		long identifier = Event.getCommandIdLong();
		CommandEntry entry = this.commandMap.get(identifier);

		if(entry == null) {
			Hackontrol.LOGGER.warn("Unknown command identifier {}", identifier);
			return;
		}

		if(!Hackontrol.getInstance().getChannel((TextChannel) Event.getChannel()).equals(entry.channel)) {
			return;
		}

		if(entry.action == null) {
			Hackontrol.LOGGER.warn("Command identifier {} has null action", identifier);
			return;
		}

		entry.action.accept(new CommandContextImplementation(Event, entry.command));
	}

	private class CommandEntry {
		private Consumer<CommandContext> action;
		private Command command;
		private HackontrolChannel channel;
	}
}
