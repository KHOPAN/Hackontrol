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

public interface ButtonContext {
	ButtonInteractionEvent getEvent();
	boolean hasParameters();
	Object[] getParameters();
	void delete();
	void acknowledge();
	void acknowledge(Consumer<? super InteractionHook> callback);
	void thinking();
	void thinking(Consumer<? super InteractionHook> callback);
	void typing();
	ReplyCallbackAction reply(MessageCreateData message);
	ReplyCallbackAction reply(String content);
	ReplyCallbackAction replyEmbeds(Collection<? extends MessageEmbed> embeds);
	ReplyCallbackAction replyEmbeds(MessageEmbed embed, MessageEmbed... embeds);
	ReplyCallbackAction replyComponents(Collection<? extends LayoutComponent> components);
	ReplyCallbackAction replyComponents(LayoutComponent component, LayoutComponent... other);
	ReplyCallbackAction replyFormat(String format, Object... arguments);
	ReplyCallbackAction replyFiles(Collection<? extends FileUpload> files);
	ReplyCallbackAction replyFiles(FileUpload... files);
	ModalCallbackAction replyModal(Modal modal);
	MessageCreateAction sendMessage(CharSequence text);
	MessageCreateAction sendMessage(MessageCreateData message);
	MessageCreateAction sendMessageFormat(String format, Object... arguments);
	MessageCreateAction sendMessageEmbeds(MessageEmbed embed, MessageEmbed... other);
	MessageCreateAction sendMessageEmbeds(Collection<? extends MessageEmbed> embeds);
	MessageCreateAction sendMessageComponents(LayoutComponent component, LayoutComponent... other);
	MessageCreateAction sendMessageComponents(Collection<? extends LayoutComponent> components);
	MessageCreateAction sendFiles(Collection<? extends FileUpload> files);
	MessageCreateAction sendFiles(FileUpload... files);
	JDA getBot();
	Guild getGuild();
	MessageChannelUnion getChannel();
	InteractionHook getInteractionHook();
	Consumer<MessageCreateData> reply();
	Consumer<MessageCreateData> message();
}
