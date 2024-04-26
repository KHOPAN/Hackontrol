package com.khopan.hackontrol.manager.modal;

import java.util.Collection;
import java.util.List;
import java.util.function.Consumer;

import net.dv8tion.jda.api.entities.MessageEmbed;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.components.LayoutComponent;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ModalContextImplementation implements ModalContext {
	private final ModalInteractionEvent Event;

	public ModalContextImplementation(ModalInteractionEvent Event) {
		this.Event = Event;
	}

	@Override
	public ModalInteractionEvent getEvent() {
		return this.Event;
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
	public ModalMapping value(String identifier) {
		return this.Event.getValue(identifier);
	}

	@Override
	public List<ModalMapping> values() {
		return this.Event.getValues();
	}
}
