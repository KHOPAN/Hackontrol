package com.khopan.hackontrol.manager.button;

import java.util.Collection;
import java.util.function.Consumer;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.MessageEmbed;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.components.LayoutComponent;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.requests.restaction.MessageCreateAction;
import net.dv8tion.jda.api.requests.restaction.interactions.ModalCallbackAction;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ButtonContextImplementation implements ButtonContext {
	private final ButtonInteractionEvent Event;
	private final Object[] parameters;

	public ButtonContextImplementation(ButtonInteractionEvent Event, Object[] parameters) {
		this.Event = Event;
		this.parameters = parameters;
	}

	@Override
	public ButtonInteractionEvent getEvent() {
		return this.Event;
	}

	@Override
	public boolean hasParameters() {
		return this.parameters != null && this.parameters.length > 0;
	}

	@Override
	public Object[] getParameters() {
		return this.parameters;
	}

	@Override
	public void delete() {
		this.Event.deferEdit().queue(hook -> hook.deleteOriginal().queue());
	}

	@Override
	public void acknowledge() {
		this.Event.deferEdit().queue();
	}

	@Override
	public void acknowledge(Consumer<? super InteractionHook> callback) {
		this.Event.deferEdit().queue(callback);
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
	public void typing() {
		this.Event.getChannel().sendTyping().queue();
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
	public ReplyCallbackAction replyComponents(LayoutComponent component, LayoutComponent... other) {
		return this.Event.replyComponents(component, other);
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
	public ModalCallbackAction replyModal(Modal modal) {
		return this.Event.replyModal(modal);
	}

	@Override
	public MessageCreateAction sendMessage(CharSequence text) {
		return this.Event.getChannel().sendMessage(text);
	}

	@Override
	public MessageCreateAction sendMessage(MessageCreateData message) {
		return this.Event.getChannel().sendMessage(message);
	}

	@Override
	public MessageCreateAction sendMessageFormat(String format, Object... arguments) {
		return this.Event.getChannel().sendMessageFormat(format, arguments);
	}

	@Override
	public MessageCreateAction sendMessageEmbeds(MessageEmbed embed, MessageEmbed... other) {
		return this.Event.getChannel().sendMessageEmbeds(embed, other);
	}

	@Override
	public MessageCreateAction sendMessageEmbeds(Collection<? extends MessageEmbed> embeds) {
		return this.Event.getChannel().sendMessageEmbeds(embeds);
	}

	@Override
	public MessageCreateAction sendMessageComponents(LayoutComponent component, LayoutComponent... other) {
		return this.Event.getChannel().sendMessageComponents(component, other);
	}

	@Override
	public MessageCreateAction sendMessageComponents(Collection<? extends LayoutComponent> components) {
		return this.Event.getChannel().sendMessageComponents(components);
	}

	@Override
	public MessageCreateAction sendFiles(Collection<? extends FileUpload> files) {
		return this.Event.getChannel().sendFiles(files);
	}

	@Override
	public MessageCreateAction sendFiles(FileUpload... files) {
		return this.Event.getChannel().sendFiles(files);
	}

	@Override
	public JDA getBot() {
		return this.Event.getJDA();
	}

	@Override
	public Guild getGuild() {
		return this.Event.getGuild();
	}

	@Override
	public MessageChannelUnion getChannel() {
		return this.Event.getChannel();
	}

	@Override
	public InteractionHook getInteractionHook() {
		return this.Event.getHook();
	}

	@Override
	public Consumer<MessageCreateData> reply() {
		return data -> this.Event.reply(data).queue();
	}

	@Override
	public Consumer<MessageCreateData> message() {
		return data -> this.Event.getChannel().sendMessage(data).queue();
	}
}
