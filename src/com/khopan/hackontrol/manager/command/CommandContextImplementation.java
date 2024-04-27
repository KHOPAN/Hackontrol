package com.khopan.hackontrol.manager.command;

import java.util.Collection;
import java.util.function.Consumer;

import com.khopan.hackontrol.manager.common.sender.sendable.ISendable;
import com.khopan.hackontrol.manager.common.sender.sendable.MessageSendable;
import com.khopan.hackontrol.manager.common.sender.sendable.ReplySendable;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.MessageEmbed;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.commands.Command;
import net.dv8tion.jda.api.interactions.components.LayoutComponent;
import net.dv8tion.jda.api.requests.restaction.MessageCreateAction;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class CommandContextImplementation implements CommandContext {
	private final SlashCommandInteractionEvent Event;
	private final JDA bot;
	private final Guild guild;
	private final MessageChannelUnion channel;
	private final InteractionHook hook;
	private final Command command;

	public CommandContextImplementation(SlashCommandInteractionEvent Event, Command command) {
		this.Event = Event;
		this.bot = this.Event.getJDA();
		this.guild = this.Event.getGuild();
		this.channel = this.Event.getChannel();
		this.hook = this.Event.getHook();
		this.command = command;
	}

	@Override
	public SlashCommandInteractionEvent getEvent() {
		return this.Event;
	}

	@Override
	public void thinking() {
		this.Event.deferReply().queue();
	}

	@Override
	public void thinking(Consumer<? super InteractionHook> callback) {
		this.Event.deferReply().queue(callback);
	}

	@Override
	public ReplyCallbackAction reply(MessageCreateData message) {
		return this.Event.reply(message);
	}

	@Override
	public ReplyCallbackAction reply(String content) {
		return this.Event.reply(content);
	}

	@Override
	public ReplyCallbackAction replyEmbeds(Collection<? extends MessageEmbed> embeds) {
		return this.Event.replyEmbeds(embeds);
	}

	@Override
	public ReplyCallbackAction replyEmbeds(MessageEmbed embed, MessageEmbed... embeds) {
		return this.Event.replyEmbeds(embed, embeds);
	}

	@Override
	public ReplyCallbackAction replyComponents(Collection<? extends LayoutComponent> components) {
		return this.Event.replyComponents(components);
	}

	@Override
	public ReplyCallbackAction replyComponents(LayoutComponent component, LayoutComponent... components) {
		return this.Event.replyComponents(component, components);
	}

	@Override
	public ReplyCallbackAction replyFormat(String format, Object... arguments) {
		return this.Event.replyFormat(format, arguments);
	}

	@Override
	public ReplyCallbackAction replyFiles(Collection<? extends FileUpload> files) {
		return this.Event.replyFiles(files);
	}

	@Override
	public ReplyCallbackAction replyFiles(FileUpload... files) {
		return this.Event.replyFiles(files);
	}

	@Override
	public MessageCreateAction sendMessage(CharSequence text) {
		return this.channel.sendMessage(text);
	}

	@Override
	public MessageCreateAction sendMessage(MessageCreateData message) {
		return this.channel.sendMessage(message);
	}

	@Override
	public MessageCreateAction sendMessageFormat(String format, Object... arguments) {
		return this.channel.sendMessageFormat(format, arguments);
	}

	@Override
	public MessageCreateAction sendMessageEmbeds(MessageEmbed embed, MessageEmbed... embeds) {
		return this.channel.sendMessageEmbeds(embed, embeds);
	}

	@Override
	public MessageCreateAction sendMessageEmbeds(Collection<? extends MessageEmbed> embeds) {
		return this.channel.sendMessageEmbeds(embeds);
	}

	@Override
	public MessageCreateAction sendMessageComponents(LayoutComponent component, LayoutComponent... components) {
		return this.channel.sendMessageComponents(component, components);
	}

	@Override
	public MessageCreateAction sendMessageComponents(Collection<? extends LayoutComponent> components) {
		return this.channel.sendMessageComponents(components);
	}

	@Override
	public MessageCreateAction sendFiles(Collection<? extends FileUpload> files) {
		return this.channel.sendFiles(files);
	}

	@Override
	public MessageCreateAction sendFiles(FileUpload... files) {
		return this.channel.sendFiles(files);
	}

	@Override
	public JDA getBot() {
		return this.bot;
	}

	@Override
	public Guild getGuild() {
		return this.guild;
	}

	@Override
	public MessageChannelUnion getChannel() {
		return this.channel;
	}

	@Override
	public InteractionHook getInteractionHook() {
		return this.hook;
	}

	@Override
	public ISendable reply() {
		return ReplySendable.of(this);
	}

	@Override
	public ISendable message() {
		return MessageSendable.of(this);
	}

	@Override
	public Command getCommand() {
		return this.command;
	}
}
